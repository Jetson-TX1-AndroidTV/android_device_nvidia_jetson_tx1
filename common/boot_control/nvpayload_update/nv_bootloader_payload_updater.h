/*
 * Copyright (c) 2016 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software and related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#ifndef T210_NV_BOOTLOADER_PAYLOAD_UPDATER_H_
#define T210_NV_BOOTLOADER_PAYLOAD_UPDATER_H_

#include <bootctrl_nvidia.h>
#include <boot_control.h>

#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <bootloader_payload_updater.h>

#define UPDATE_TYPE 0
#define BMP_TYPE 1

#define UPDATE_MAGIC "NVIDIA__BLOB__V2"
#define UPDATE_MAGIC_SIZE 17
#define HEADER_LEN 36
#define ENTRY_LEN 52

#define PARTITION_PATH "/dev/block/platform/sdhci-tegra.3/by-name/"
#define BOOT_PART_PATH "/dev/block/platform/sdhci-tegra.3/mmcblk0boot0"
#define BP_ENABLE_PATH "/sys/block/mmcblk0boot0/force_ro"
#define PARTITION_LEN 40

#define BMP_PATH "/data/misc/update_engine/bmp.blob"
#define BLOB_PATH "/data/misc/update_engine/blob"

#define BMP_NAME "BMP"
#define KERNEL_DTB_NAME "DTB"
#define BL_DTB_NAME "RP1"

/* Compute ceil(n/d) */
#define DIV_CEIL(n, d) (((n) + (d) - 1) / (d))

/* Round-up n to next multiple of w */
#define ROUND_UP(n, w) (DIV_CEIL(n, w) * (w))


/*
 * TODO : read those value from block device
 *
 */
#define BLOCK_SIZE 512
#define BCT_MAX_COPIES 64
#define BR_BLOCK_SIZE (16 * 1024)
#define BR_PAGE_SIZE 512

/*
 * The define of each partion in emmc boot partition.
 * Please refer to flash.xml for partition info.
 *
 * TODO: find a way to parse size of each partition.
 *
 */
// Size of parttions
#define PART_BCT_SIZE 3145728
#define PART_NVC_SIZE 262144

struct Partition {
    char name[PARTITION_LEN];
    int part_size;
};

Partition boot_partiton[] = {
    { "BCT", PART_BCT_SIZE },
    { "NVC", PART_NVC_SIZE },
};

struct DependPartition {
    const char *name;
    int slot;
};

DependPartition part_dependence[] = {
    { "NVC", 0 },
    { "BCT", 0 },
    { "BCT", 1 },
    { "NVC", 1 },
};

enum PartitionType {
    kBootPartition = 0,
    kUserPartition,
    kDependPartition
};

class NvPayloadUpdate : public BLPayloadUpdate {
 public:
    NvPayloadUpdate();
    ~NvPayloadUpdate() = default;

    /* UpdateDriver - main function that parses the Bootloader
     * Payload (blob) and writes to partitions in unused slots.
     * @params - payload_path, path to bootloader paylaod
     * @return - 0 on success, non-zero otherwise.
     */
    BLStatus UpdateDriver();

 private:
    struct Header{
        char magic[UPDATE_MAGIC_SIZE];
        int hex;
        int size;
        int header_size;
        int number_of_elements;
        int type;
    };

    struct Entry{
         char partition[PARTITION_LEN];
         int pos;
         int len;
         int version;
         PartitionType type;
         BLStatus (*write)(Entry*, FILE*, int);
    };

    // Updates the partitions in ota.blob
    static BLStatus OTAUpdater(const char* ota_path);

    // Updates BMP-A/BMP-B with bmp.blob
    static BLStatus BMPUpdater(const char* bmp_path);

    // Gets the name of partition in unused slot
    static char* GetUnusedPartition(const char* partition_name, int slot);

    // Parses header in the payload
    static void ParseHeaderInfo(unsigned char* buffer, Header* header);
    static void ParseEntryTable(char* buffer, Entry* entry_table,
                                Header* header);

    static bool IsDependPartition(const char *partition);
    static bool IsBootPartition(const char *partition);
    static void GetEntryTable(const char *part, Entry *entry_t,
                              Entry entry_table[],
                              int len);

    // Writes to unused slot partitions from the payload
    static BLStatus WriteToPartition(Entry entry_table[], FILE* blobfile,
                                     int len);

    static BLStatus WriteToUserPartition(Entry *entry_table,
                                         FILE* blobfile,
                                         int slot);
    static BLStatus WriteToDependPartition(Entry entry_table[],
                                           FILE* blobfile, int len);

    static BLStatus WriteToBootPartition(Entry *entry_table,
                                         FILE* blobfile, int slot);

    static BLStatus EnableBootPartitionWrite(int enable);

    static BLStatus WriteToBctPartition(Entry *entry_table,
                                        FILE *blob_file,
                                        FILE *bootp,
                                        int slot);


    static int VerifiedPartiton(Entry *entry_table, FILE *blob_file, int slot);

    // Log parsing of payload
    static void PrintHeader(Header* header);
    static void PrintEntryTable(Entry* entry_table, Header* header);
};

BLPayloadUpdate* make_updater() { return new NvPayloadUpdate(); }

#endif  // T210_NV_BOOTLOADER_PAYLOAD_UPDATER_H_
