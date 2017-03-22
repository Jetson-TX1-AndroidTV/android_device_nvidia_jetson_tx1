/*
 * Copyright (c) 2016 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software and related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#include "nv_bootloader_payload_updater.h"
#include <boot_control.h>
#include <base/logging.h>
#include <string>
#include <iostream>

extern "C" {
#include <fdt_util.h>
#include <bct_update.h>
}

boot_control_module_t *g_bootctrl_module = NULL;

BLStatus NvPayloadUpdate::UpdateDriver() {
    BLStatus status;

    status = OTAUpdater(BLOB_PATH);
    if (status != kSuccess) {
        LOG(ERROR) << "OTA Blob update failed. Status: "
            << static_cast<int>(status);
       return status;
    }

    status = BMPUpdater(BMP_PATH);
    if (status != kSuccess) {
        LOG(ERROR) << "BMP Blob update failed. Status: "
            << static_cast<int>(status);
       return status;
    }

    return status;
}

NvPayloadUpdate::NvPayloadUpdate() {
    const hw_module_t* hw_module;

    if (hw_get_module("bootctrl", &hw_module) != 0)  {
        LOG(ERROR) << "Error getting bootctrl module";
    }

    g_bootctrl_module  = reinterpret_cast<boot_control_module_t*>(
                    const_cast<hw_module_t*>(hw_module));

    g_bootctrl_module->init(g_bootctrl_module);
}

BLStatus NvPayloadUpdate::BMPUpdater(const char* bmp_path) {
    FILE* blob_file;
    char* buffer;
    char* unused_path = NULL;
    int bytes;
    int err;
    Header* header = new Header;
    FILE* slot_stream;
    int current_slot;
    BLStatus status = kSuccess;

    if (!g_bootctrl_module) {
        LOG(ERROR) << "Error getting bootctrl module";
        return kBootctrlGetFailed;
    }

    blob_file = fopen(bmp_path, "r");
    if (!blob_file) {
         return  kBlobOpenFailed;
    }

    // Parse Header
    buffer = new char[HEADER_LEN];
    bytes = fread(buffer, 1, HEADER_LEN, blob_file);
    ParseHeaderInfo((unsigned char*) buffer, header);
    delete[] buffer;

    PrintHeader(header);

    err = fseek(blob_file, 0, SEEK_SET);
    buffer = new char[header->size];
    bytes = fread(buffer, 1, header->size, blob_file);

    current_slot =
        g_bootctrl_module->getCurrentSlot(g_bootctrl_module);
    if (current_slot < 0) {
        LOG(ERROR) << "Error getting current SLOT";
        status = kBootctrlGetFailed;
        goto exit;
    }

    unused_path = GetUnusedPartition(BMP_NAME, !current_slot);
    if (!unused_path) {
        status = kBootctrlGetFailed;
        goto exit;
    }

    LOG(INFO) << "Writing to " << unused_path << " for "
              << BMP_NAME;

    slot_stream = fopen(unused_path, "rb+");
    if (!slot_stream) {
        LOG(ERROR) << "Slot could not be opened "<< BMP_NAME;
        status = kSlotOpenFailed;
        goto exit;
    }

    bytes = fwrite(buffer, 1, header->size, slot_stream);
    LOG(INFO) << "Bytes written to "<< BMP_NAME
                << ": "<< bytes;

    fclose(slot_stream);

exit:
    delete[] buffer;
    delete header;
    fclose(blob_file);

    return status;
}

BLStatus NvPayloadUpdate::OTAUpdater(const char* ota_path) {
    FILE* blob_file;
    Header* header = new Header;
    char* buffer = new char[HEADER_LEN];
    int bytes = 0;
    int err;
    BLStatus status;

    blob_file = fopen(ota_path, "r");
    if (!blob_file) {
        return  kBlobOpenFailed;
    }

    // Parse the header
    bytes = fread(buffer, 1, HEADER_LEN, blob_file);
    ParseHeaderInfo((unsigned char*) buffer, header);
    delete[] buffer;

    PrintHeader(header);

    // Parse the entry table
    Entry* entry_table = new Entry[header->number_of_elements];
    int entry_table_size = header->number_of_elements * ENTRY_LEN;
    buffer = new char[entry_table_size];
    err = fseek(blob_file, HEADER_LEN, SEEK_SET);
    bytes = fread(buffer, 1, entry_table_size, blob_file);
    ParseEntryTable(buffer, entry_table, header);
    delete[] buffer;

    PrintEntryTable(entry_table, header);

    // Write each partition
    err = fseek(blob_file, 0, SEEK_SET);
    status = WriteToPartition(entry_table, blob_file,
                header->number_of_elements);
    if (status) {
        LOG(ERROR) << "Writing to partitions failed.";
    }

    delete header;
    delete[] entry_table;
    fclose(blob_file);

    return status;
}

static int OffsetOfBootPartition(const char *part, int slot) {
    int offset = 0;

    if (!strcmp(part, "BCT")) {
        if (slot)
            return (BCT_MAX_COPIES - 1) * BR_BLOCK_SIZE;
        else
            return 0;
    }

    offset = PART_BCT_SIZE;

    for (unsigned i = 1; i < sizeof(boot_partiton)/sizeof(Partition); i++) {
        if (!strcmp(part, boot_partiton[i].name)) {
            offset += slot * boot_partiton[i].part_size;
            break;
        }
        offset += 2 * boot_partiton[i].part_size;
    }

    return offset;
}

BLStatus NvPayloadUpdate::EnableBootPartitionWrite(int enable) {
    FILE* fd;
    int bytes = 0;

    fd = fopen(BP_ENABLE_PATH, "rb+");
    if (!fd) {
        LOG(ERROR) << "BP_ENABLE_PATH could not be opened ";
        return kFsOpenFailed;
    }

    if (enable)
        bytes = fwrite("0", 1, 1, fd);
    else
        bytes = fwrite("1", 1, 1, fd);

    fclose(fd);
    return kSuccess;
}

char* NvPayloadUpdate::GetUnusedPartition(const char* partition_name,
                                          int slot) {
    char* unused_path;
    const char* free_slot_name;
    int path_len;

    free_slot_name =
        (slot)? BOOTCTRL_SUFFIX_B: BOOTCTRL_SUFFIX_A;

    path_len = strlen(PARTITION_PATH) + strlen(partition_name)
                            + strlen(free_slot_name) + 1;

    unused_path = new char[path_len];
    snprintf(unused_path, path_len,  "%s%s%s", PARTITION_PATH,
        partition_name, free_slot_name);

    return unused_path;
}

bool NvPayloadUpdate::IsDependPartition(const char *partition) {
    unsigned int i;

    for (i = 0; i < sizeof(part_dependence)/sizeof(*part_dependence); i++) {
        if (!strcmp(partition, part_dependence[i].name))
            return true;
    }

    return false;
}

bool NvPayloadUpdate::IsBootPartition(const char *partition) {
    unsigned int i;

    for (i = 0; i < sizeof(boot_partiton)/sizeof(Partition); i++) {
        if (!strcmp(partition, boot_partiton[i].name))
            return true;
    }

    return false;
}

BLStatus NvPayloadUpdate::WriteToBctPartition(Entry *entry_table,
                                                FILE *blob_file,
                                                FILE *bootp,
                                                int slot) {
    int bytes = 0;
    unsigned pages_in_bct = 0;
    unsigned slot_size = 0;
    unsigned bct_count = 0;
    int offset = 0;
    int bin_size = entry_table->len;
    BLStatus status;
    int err;

    /*
     * Read update binary from blob
     */
    unsigned char* new_bct = new unsigned char[bin_size];
    fseek(blob_file, entry_table->pos, SEEK_SET);
    bytes = fread(new_bct, 1, bin_size, blob_file);

    /*
     * Read current BCT from boot partition
     */
    unsigned char *cur_bct = new unsigned char[bin_size];
    fseek(bootp, 0, SEEK_SET);
    bytes = fread(cur_bct, 1, bin_size, bootp);

    /*
     * Update signature of current BCT
     */
    err = update_bct_signedsection(cur_bct, new_bct, bin_size);
    if (err) {
        LOG(ERROR) << "Update BCT signature failed, err = " << err;
        status = kInternalError;
        goto exit;
    }

    pages_in_bct = DIV_CEIL(bin_size, BR_PAGE_SIZE);

    /*
    The term "slot" refers to a potential location
    of a BCT in a block. A slot is the smallest integral
    number of pages that can hold a BCT.Thus, every
    BCT begins at the start of a page and may span
    multiple pages. A block is a space in memory that
    can hold multiple slots of BCTs.
    */
    slot_size = pages_in_bct * BR_PAGE_SIZE;

    /*
    The BCT search sequence followed by BootROM is:
    Block 0, Slot 0
    Block 0, Slot 1
    Block 1, Slot 0
    Block 1, Slot 1
    Block 1, Slot 2
    .....
    Block 1, Slot N
    Block 2, Slot 0
    .....
    Block 2, Slot N
    .....
    Block 63, Slot N
    Based on the search sequence, we write the
    block 0, slot 1 BCT first, followed by one BCT
    in slot 0 of subsequent blocks and lastly one BCT
    in block0, slot 0.
    */

    if (slot)
        goto block_1;

    /*
     * Write one BCT to slot 1, block 0
     * if blocksize is bigger than twice of slotsize
     */
    if (BR_BLOCK_SIZE > slot_size * 2) {
        fseek(bootp, slot_size, SEEK_SET);
        bytes = fwrite(new_bct, 1, bin_size, bootp);

        LOG(INFO) << entry_table->partition << " write: offset = " << slot_size
            << " bytes = " << bytes;
    }

    /* Finally write to block 0, slot 0 */
    offset = 0;
    fseek(bootp, offset, SEEK_SET);
    bytes = fwrite(new_bct, 1, bin_size, bootp);

    LOG(INFO) << entry_table->partition << " write: offset = " << offset
           << " bytes = " << bytes;

    goto exit;

block_1:
    /* Fill Slot 0 for all other blocks */
    offset = BR_BLOCK_SIZE;
    while (offset < PART_BCT_SIZE) {
        fseek(bootp, offset, SEEK_SET);
        bytes = fwrite(new_bct, 1, bin_size, bootp);

        LOG(INFO) << entry_table->partition << " write: offset = " << offset
              << " bytes = " << bytes;

        offset += BR_BLOCK_SIZE;
        bct_count++;
        if (bct_count == BCT_MAX_COPIES - 1)
            break;
    }

exit:
    delete[] cur_bct;
    delete[] new_bct;

    return kSuccess;
}

BLStatus NvPayloadUpdate::WriteToBootPartition(Entry *entry_table,
                                               FILE* blob_file,
                                               int slot) {
    int bin_size = entry_table->len;
    FILE* bootp;
    int bytes = 0;
    BLStatus status = kSuccess;
    int offset = 0;

    status = EnableBootPartitionWrite(1);
    if (status) {
        return kFsOpenFailed;
    }

    bootp = fopen(BOOT_PART_PATH, "rb+");
    if (!bootp) {
        LOG(ERROR) << "Boot Partition could not be opened "
            << entry_table->partition;
        status = EnableBootPartitionWrite(0);

        return kFsOpenFailed;
    }

    if (!strcmp(entry_table->partition, "BCT")) {
        status = WriteToBctPartition(entry_table, blob_file, bootp, slot);
    } else {
        /*
         * Read update binary from blob
         */
        char* buffer = new char[bin_size];
        fseek(blob_file, entry_table->pos, SEEK_SET);
        bytes = fread(buffer, 1, bin_size, blob_file);

        offset = OffsetOfBootPartition(entry_table->partition, slot);

        fseek(bootp, offset , SEEK_SET);
        bytes = fwrite(buffer, 1, bin_size, bootp);
        delete[] buffer;

        LOG(INFO) << entry_table->partition
            << " write: offset = " << offset << " bytes = " << bytes;
    }

    fclose(bootp);
    EnableBootPartitionWrite(0);

    return status;
}

void NvPayloadUpdate::GetEntryTable(const char *part, Entry *entry_t,
                                    Entry entry_table[], int len) {
    for (int i = 0; i < len; i++) {
        if (!strcmp(entry_table[i].partition, part)) {
            *entry_t = entry_table[i];
            break;
        }
    }
}

int NvPayloadUpdate::VerifiedPartiton(Entry *entry_table,
                                           FILE *blob_file, int slot) {
    FILE *fd;
    int offset;
    int bin_size = entry_table->len;
    char* source = new char[bin_size];
    char* target = new char[bin_size];
    int result;

    fd = fopen(BOOT_PART_PATH, "r");

    offset = OffsetOfBootPartition(entry_table->partition, slot);

    fseek(fd, offset, SEEK_SET);
    fread(source, 1, bin_size, fd);
    fclose(fd);

    fseek(blob_file, entry_table->pos, SEEK_SET);
    fread(target, 1, bin_size, blob_file);

    result = memcmp(target, source, bin_size);
    if (result == 0) {
        LOG(INFO) << entry_table->partition
            << " slot: " << slot
            << " is updated. Skip... ";
    }

    delete[] target;
    delete[] source;

    return result;
}

BLStatus NvPayloadUpdate::WriteToDependPartition(Entry entry_table[],
                                                 FILE* blob_file, int len) {
    Entry entry_t;
    BLStatus status = kSuccess;
    int slot, i;
    int num_part = sizeof(part_dependence)/sizeof(*part_dependence);

    for (i = 0; i < num_part; i++) {
        slot = part_dependence[i].slot;
        GetEntryTable(part_dependence[i].name, &entry_t, entry_table, len);
        if (VerifiedPartiton(&entry_t, blob_file, slot)) {
            status = (entry_t.write)(&entry_t, blob_file, slot);
            if (status) {
                LOG(ERROR) << entry_t.partition <<" update failed ";
                return kInternalError;
            }
        }
    }

    return status;
}

BLStatus NvPayloadUpdate::WriteToUserPartition(Entry *entry_table,
                                               FILE* blob_file,
                                               int slot) {
    char* unused_path;
    FILE* slot_stream;
    int bytes = 0;
    int part_size = entry_table->len;

    unused_path = GetUnusedPartition(entry_table->partition, slot);
    if (!unused_path) {
        return kBootctrlGetFailed;
    }

    slot_stream = fopen(unused_path, "rb+");
    if (!slot_stream) {
        LOG(ERROR) << "Slot could not be opened "<< entry_table->partition;
        return  kSlotOpenFailed;
    }

    char* buffer = new char[part_size];
    fseek(blob_file, entry_table->pos, SEEK_SET);
    bytes = fread(buffer, 1, part_size, blob_file);

    if (!strcmp(entry_table->partition, KERNEL_DTB_NAME) &&
            is_dtb_valid(reinterpret_cast<void*>(buffer), KERNEL_DTB, slot)) {
        goto exit;
    } else if (!strcmp(entry_table->partition, BL_DTB_NAME) &&
            is_dtb_valid(reinterpret_cast<void*>(buffer), BL_DTB, slot)) {
        goto exit;
    }

    LOG(INFO) << "Writing to " << unused_path << " for "
        << entry_table->partition;

    bytes = fwrite(buffer, 1, part_size, slot_stream);
    LOG(INFO) << entry_table->partition
        << " write: bytes = " << bytes;

exit:
    delete[] buffer;
    fclose(slot_stream);

    return kSuccess;
}

BLStatus NvPayloadUpdate::WriteToPartition(Entry entry_table[],
                                           FILE* blob_file, int len) {
    BLStatus status = kSuccess;
    int current_slot;

    if (!g_bootctrl_module) {
        LOG(ERROR) << "Error getting bootctrl module";
        return kBootctrlGetFailed;
    }

    current_slot = g_bootctrl_module->getCurrentSlot(g_bootctrl_module);
    if (current_slot < 0) {
        LOG(ERROR) << "Error getting current SLOT";
        return kBootctrlGetFailed;
    }

    for (int i = 0; i < len; i++) {
        if (entry_table[i].type != kDependPartition) {
            status = (entry_table[i].write)(&entry_table[i], blob_file,
                        !current_slot);
            if (status) {
                LOG(INFO) << entry_table[i].partition
                    << " fail to write ";
                return status;
            }
        }
    }

    status = WriteToDependPartition(entry_table, blob_file, len);
    if (status) {
        LOG(INFO) << "Fail to write Dependence partitions ";
    }

    return status;
}

void NvPayloadUpdate::ParseHeaderInfo(unsigned char* buffer,
                                      Header* header) {
    std::memcpy(header->magic, buffer, (sizeof(header->magic)-1));

    // blob header magic string does not have null terminator
    header->magic[sizeof(header->magic)-1] = '\0';
    buffer += sizeof(header->magic)-1;

    std::memcpy( &header->hex, buffer, sizeof(int));
    buffer += sizeof(int);

    std::memcpy( &header->size, buffer, sizeof(int));
    buffer += sizeof(int);

    std::memcpy( &header->header_size, buffer, sizeof(int));
    buffer += sizeof(int);

    std::memcpy( &header->number_of_elements, buffer, sizeof(int));
    buffer += sizeof(int);

    std::memcpy( &header->type, buffer, sizeof(int));
    buffer += sizeof(int);
}

void NvPayloadUpdate::ParseEntryTable(char* buffer, Entry* entry_table,
                                      Header* header) {
    int num_entries = header->number_of_elements;

    for (int i = 0; i < num_entries; i++) {
        std::memcpy(entry_table[i].partition, buffer,
                                              (PARTITION_LEN)*sizeof(char));
        buffer+= (PARTITION_LEN)*sizeof(char);

        std::memcpy(&entry_table[i].pos, buffer, sizeof(int));
        buffer+= sizeof(int);

        std::memcpy(&entry_table[i].len, buffer, sizeof(int));
        buffer+= sizeof(int);

        std::memcpy(&entry_table[i].version, buffer, sizeof(int));
        buffer+= sizeof(int);

        if (IsDependPartition(entry_table[i].partition)) {
            entry_table[i].type = kDependPartition;
            entry_table[i].write = WriteToBootPartition;
        } else if (IsBootPartition(entry_table[i].partition)) {
            entry_table[i].type = kBootPartition;
            entry_table[i].write = WriteToBootPartition;
        } else {
            entry_table[i].type = kUserPartition;
            entry_table[i].write = WriteToUserPartition;
        }
    }
}

void NvPayloadUpdate::PrintHeader(Header* header) {
    LOG(INFO) << "HEADER: MAGIC " << header->magic;
    LOG(INFO) << "HEX_VALUE " << header->hex;
    LOG(INFO) << "BLOB_SIZE " << header->size;
    LOG(INFO) << "HEADER_SIZE " << header->header_size;
    LOG(INFO) << "NUMBER_OF_ELEMENTS " << header->number_of_elements;
    LOG(INFO) << "HEADER_TYPE " << header->type;
}

void NvPayloadUpdate::PrintEntryTable(Entry* entry_table, Header* header) {
    LOG(INFO) << "ENTRY_TABLE:";
    LOG(INFO) << "PART  POS  LEN  VER TYPE";

    for (int i = 0; i < header->number_of_elements; i++) {
        LOG(INFO) << entry_table[i].partition << "  "
                << entry_table[i].pos << "  "
                << entry_table[i].len << "  "
                << entry_table[i].version << "  "
                << static_cast<int>(entry_table[i].type);
    }
}
