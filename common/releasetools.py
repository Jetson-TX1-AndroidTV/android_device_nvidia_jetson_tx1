#!/usr/bin/python
#
# Copyright (c) 2011 NVIDIA Corporation.  All rights reserved.
#
# NVIDIA Corporation and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA Corporation is strictly prohibited.
#

import common

def FullOTA_InstallEnd(info):
    partitions = ["/staging", "/bmps"]
    filenames = ["blob", "bmp.blob"]
    for i,filepath in enumerate(['RADIO/blob', 'RADIO/bmp.blob']):
        try:
            info.input_zip.getinfo(filepath)
        except KeyError:
            continue;
        else:
            # copy the data into the package.
            data = info.input_zip.read(filepath)
            common.ZipWriteStr(info.output_zip, filenames[i], data)
            # emit the script code to install this data on the device
            info.script.AppendExtra(
                'nv_copy_blob_file("%s", "%s");' % (filenames[i], partitions[i]))




def IncrementalOTA_InstallEnd(info):
    partitions = ["/staging", "/bmps"]
    filenames = ["blob", "bmp.blob"]
    for i,filepath in enumerate(['RADIO/blob', 'RADIO/bmp.blob']):
        try:
            info.target_zip.getinfo(filepath)
        except KeyError:
            continue;
        else:
            target_data = info.target_zip.read(filepath)
            try:
                info.source_zip.getinfo(filepath)
                # copy the data into the package.
                source_data = info.source_zip.read(filepath)
                if source_data == target_data:
                    # data is unchanged from previous build; no
                    # need to reprogram it
                    continue;
                else:
                    # include the new dat in the OTA package
                    common.ZipWriteStr(info.output_zip, filenames[i], target_data)
                    # emit the script code to install this data on the device
                    info.script.AppendExtra(
                        'nv_copy_blob_file("%s", "%s");' % (filenames[i], partitions[i]))
            except KeyError:
                # include the new data in the OTA package
                common.ZipWriteStr(info.output_zip, filenames[i], target_data)
                # emit the script code to install this data on the device
                info.script.AppendExtra(
                    'nv_copy_blob_file("%s", "%s");' % (filenames[i], partitions[i]))

def AbOTA_InstallEnd(info):
    filenames = ["blob", "bmp.blob"]
    for i,filepath in enumerate(['RADIO/blob', 'RADIO/bmp.blob']):
        try:
            info.input_zip.getinfo(filepath)
        except KeyError:
            continue;
        else:
            # copy the data into the package.
            data = info.input_zip.read(filepath)
            common.ZipWriteStr(info.output_zip, filenames[i], data)
