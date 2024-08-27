#!/bin/bash

set -e

CURRENT_DIR="$(dirname -- "$(realpath -- "$0")")" # Current directory
PARENT_DIR="$(dirname "$CURRENT_DIR")"

ACTUAL=${ACTUAL:-$PARENT_DIR/output/tests/ccm_example0.png}
EXPECTED=${EXPECTED:-$CURRENT_DIR/expected/ccm_example0.png}
TOLERANCE=${TOLERANCE:-0.1}

if [ ! -f "$ACTUAL" ]; then
    echo "Image not found at "$ACTUAL
    exit 1
fi

pip install imgcompare --quiet 2>/dev/null

DIFF=$(printf "%.2f" $(python -c 'import imgcompare; print(imgcompare.image_diff_percent("'$ACTUAL'", "'$EXPECTED'"));'))
if { echo $TOLERANCE ; echo $DIFF ; } | sort -n -c 2>/dev/null; then
    echo "Image diff = $DIFF%. Tolerance = $TOLERANCE. FAILED"
    exit 2
else
    echo "Image diff = $DIFF%. Tolerance = $TOLERANCE. OK"
fi

exit 0
