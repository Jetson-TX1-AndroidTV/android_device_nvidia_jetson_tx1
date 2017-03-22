# Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
#
# NVIDIA Corporation and its licensors retain all intellectual property and
# proprietary rights in and to this software and related documentation.  Any
# use, reproduction, disclosure or distribution of this software and related
# documentation without an express license agreement from NVIDIA Corporation
# is strictly prohibited.

#!/bin/bash
# A blob is composed of 2 parts: header field and data field
# this script compresses the data field and keeps header

compressor=$1
blob_file=$2
blob_header_len=$3
blob_size_pos=$4

if [[ $# -lt 4 ]]; then
	echo 1>&2 "usage: $0 <path to lz4c> <blob to be compressed> <blob header length> <blob size postion in header>"
	exit 1
fi

if ! command -v ${compressor} >/dev/null 2>&1; then
	echo 1>&2 "can't find '${compressor}', stop compressing blob."
	exit 1
fi

blob_size=`cat ${blob_file} | wc -c`
new_blob=${blob_file}.tmp

# copy blob header directly
head -c ${blob_header_len} ${blob_file} > ${new_blob}

# compress blob data feild
blob_size=$((${blob_size} - ${blob_header_len}))
cmd="${compressor} -l -c1 -f stdin >> ${new_blob}"
tail -c ${blob_size} ${blob_file} | eval $cmd

# update blob size(4Bytes) in header
comp_blob_size=`cat ${new_blob} | wc -c`

pos_cur=${blob_size_pos}
pos_end=$((${pos_cur} + 4))
while [[ ${pos_cur} -lt ${pos_end} ]]
do
byte=$((${comp_blob_size} % 256))
byte=$(printf %x ${byte})
printf "\x${byte}" | dd conv=notrunc of=${new_blob} bs=1 seek=${pos_cur}

comp_blob_size=$((${comp_blob_size} / 256))
pos_cur=$((${pos_cur} + 1))
done

# replace output blob with compressed one
rm ${blob_file}
mv ${new_blob} ${blob_file}

exit 0

