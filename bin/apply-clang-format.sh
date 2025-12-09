#!/usr/bin/env sh

# SPDX-FileCopyrightText: 2025 Kohei Yoshida
#
# SPDX-License-Identifier: MIT

PROGDIR=$(dirname $0)

cd "$PROGDIR/.."
git ls-tree --full-tree --name-only -r HEAD -- include tools test \
    | grep -E "\.(cpp|hpp|inl)$" \
    | xargs clang-format -i
