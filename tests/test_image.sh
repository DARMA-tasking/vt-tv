#!/bin/bash

set -e

CURRENT_DIR="$(dirname -- "$(realpath -- "$0")")" # Current directory

ACTUAL=${ACTUAL:-/home/thomas/repositories/vt-tv/output/tests/ccm_example0.png}
EXPECTED=${EXPECTED:-$CURRENT_DIR/expected/ccm_example0.png}
TOLERANCE=${TOLERANCE:-2.0}

if [ ! -f "$ACTUAL" ]; then
    echo "Image not found at "$ACTUAL
    exit 1
fi

pip install imgcompare --quiet 2>/dev/null

DIFF=$(printf "%.2f" $(python -c 'import imgcompare; print(imgcompare.image_diff_percent("'$ACTUAL'", "'$EXPECTED'"));'))
if { echo $TOLERANCE ; echo $DIFF ; } | sort -n -c 2>/dev/null; then
    echo "Image diff = $DIFF%. Tolerance is $TOLERANCE"
    exit 2
fi

exit 0