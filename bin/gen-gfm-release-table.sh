#!/usr/bin/env bash

PKG_PREFIX=mdds

# Pick up version number string from configure.ac.
VER=$(cat ./configure.ac | grep AC_INIT | sed -e "s/.*\([0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\).*/\1/g")

PKGS=$(ls $PKG_PREFIX-$VER.tar.*)

echo "## Release Notes"
echo ""
echo "* add item"
echo ""
echo "## Checksums for Packages"
echo ""

echo "| name | sha256sum | size |"
echo "|-----|-----------|------|"

for _PKG in $PKGS; do
    _HASH=$(sha256sum $_PKG | sed -e "s/^\(.*\)$PKG_PREFIX.*/\1/g" | tr -d "[:space:]")
    _SIZE=$(stat -c "%s" $_PKG)
    echo "| $_PKG | $_HASH | $_SIZE |"
done

