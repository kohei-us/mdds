#!/usr/bin/env sh

PROGDIR=$(dirname $0)

cd "$PROGDIR/.."
git ls-tree --full-tree --name-only -r HEAD -- include tools test \
    | grep -E "\.(cpp|hpp|inl)$" \
    | xargs clang-format -i
