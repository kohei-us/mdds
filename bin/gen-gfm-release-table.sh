#!/usr/bin/env bash

PKG_PREFIX=mdds

# Pick up version number string from configure.ac.
VER=$(cat ./configure.ac | grep AC_INIT | sed -e "s/.*\([0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\).*/\1/g")

PKGS=$(ls $PKG_PREFIX-$VER.tar.*)

echo "| URL | sha256sum | size |"
echo "|-----|-----------|------|"

for _PKG in $PKGS; do
    _URL="[$_PKG](https://kohei.us/files/orcus/src/$_PKG)"
    _HASH=$(sha256sum $_PKG | sed -e "s/^\(.*\)$PKG_PREFIX.*/\1/g" | tr -d "[:space:]")
    _SIZE=$(stat -c "%s" $_PKG)
    echo "| $_URL | $_HASH | $_SIZE |"
done

