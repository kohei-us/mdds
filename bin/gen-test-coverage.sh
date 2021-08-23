#!/usr/bin/env bash

COVFILE="coverage.info"
PROGDIR=$(dirname $0)
TESTDIR=$(realpath "$PROGDIR/../test")

# Get the output directory from the user.
OUTDIR=$1
shift

if [ -z "$OUTDIR" ]; then
    echo "Usage: $0 [output directory]"
    exit 1
fi

mkdir -p "$OUTDIR"
cd "$OUTDIR"

lcov --capture --directory "$TESTDIR" -o $COVFILE || exit 1
genhtml $COVFILE -o html || exit 1

# View the reports in web browser.
xdg-open html/index.html

