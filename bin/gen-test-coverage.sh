#!/usr/bin/env bash

COVFILE="coverage.info"
ROOTDIR="$PWD"

# Get the output directory from the user.
OUTDIR=$1
shift

if [ -z "$OUTDIR" ]; then
    echo "Usage: $0 [output directory] file1 file2 file3 ..."
    exit 1
fi

# Turn the file paths into absolute path.
_FILES=
for _FILE in "$@"; do
    _FILES="$_FILES $ROOTDIR/$_FILE"
done

mkdir -p $OUTDIR
cd $OUTDIR
gcov $_FILES || exit 1

# For now, we assume that all test source files are located in the same directory.
SRCDIR=$(echo $_FILES | cut -d' ' -f1)
SRCDIR=$(dirname $SRCDIR)

# Generate HTML reports.
lcov --capture --directory $SRCDIR -o $COVFILE || exit 1
genhtml $COVFILE -o html || exit 1

# View the reports in web browser.
xdg-open html/index.html

