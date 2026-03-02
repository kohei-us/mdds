#!/usr/bin/env bash

# SPDX-FileCopyrightText: 2025 Kohei Yoshida
#
# SPDX-License-Identifier: MIT

PKG_PREFIX=mdds

# Pick up version number string from configure.ac.
MAJOR_VER=$(cat ./configure.ac | grep "m4_define.*\[mdds_version_major\]" | sed -e "s/.*\[\([0-9]*\)\].*/\1/g")
MINOR_VER=$(cat ./configure.ac | grep "m4_define.*\[mdds_version_minor\]" | sed -e "s/.*\[\([0-9]*\)\].*/\1/g")
MICRO_VER=$(cat ./configure.ac | grep "m4_define.*\[mdds_version_micro\]" | sed -e "s/.*\[\([0-9]*\)\].*/\1/g")
VER=$MAJOR_VER.$MINOR_VER.$MICRO_VER

PKGS=$(ls $PKG_PREFIX-$VER.tar.*)

echo "## Checksums for Packages"
echo ""

echo "| name | sha256sum | size |"
echo "|-----|-----------|------|"

for _PKG in $PKGS; do
    _HASH=$(sha256sum $_PKG | sed -e "s/^\(.*\)$PKG_PREFIX.*/\1/g" | tr -d "[:space:]")
    _SIZE=$(stat -c "%s" $_PKG)
    echo "| $_PKG | $_HASH | $_SIZE |"
done

