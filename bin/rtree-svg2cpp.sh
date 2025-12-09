#!/usr/bin/env bash

# SPDX-FileCopyrightText: 2025 Kohei Yoshida
#
# SPDX-License-Identifier: MIT

# Input file must be an svg exported from LibreOffice Draw.
# Each line consists of x, y, width and height in this order.

if [ -z "$1" ]; then
    echo "no input file."
    exit 1
fi

cat "$1" | grep BoundingBox \
    | sed \
    -e 's/.*x\=\"/{\ /g' \
    -e 's/\"\ y\=\"/, /g' \
    -e 's/\"\ width\=\"/, /g' \
    -e 's/\"\ height\=\"/, /g' \
    -e 's/\"\/>/\ },/g'
