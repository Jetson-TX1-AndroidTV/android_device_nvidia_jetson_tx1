#!/bin/bash
#
# Usage: copy_simtools.sh [--help]
#
# Description: Copy the simtools scripts and testlists so they can be packaged
#              with other binaries and images.
#-------------------------------------------------------------------------------

if [ "$1" == "--help" ]; then
    this_script=`basename $0`

    cat <<EOF

    Copy the simtools scripts and testlists so they can be packaged
    with other binaries and images.

    Usage:

          $this_script [--help]

EOF
    exit 0
fi

# Sanity checks & default option processing
if [ -z "$TOP" ]; then
    echo "ERROR: You must set environment variable TOP to the top of your repo tree"
    exit 2
fi

if [ -z "$OUT" ]; then
    echo "ERROR: You must set environment variable OUT to the output directory"
    exit 2
fi

if [ "$TOP" == "." ]; then
    top=`pwd`
else
    top=$TOP
fi

SIMTOOLS=$top/vendor/nvidia/tegra/core-private/tools/simtools

if [ ! -d "$OUT" ]; then
    echo "ERROR: Product output directory $OUT does not exist."
    exit 2
fi

if [ ! -d $SIMTOOLS ]; then
    echo "ERROR: simtools directory $SIMTOOLS does not exits."
    exit 2
fi

echo "Copying the simtools directory..."
rm -rf $OUT/system/simtools
cp -R $SIMTOOLS $OUT/system
status=$?

if [ $status == 0 ]; then
    echo "Done"
fi
exit $status
