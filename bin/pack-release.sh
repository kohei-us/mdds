#!/usr/bin/env bash

VERSION=$1
DIR=mdds_$VERSION
REPO_URL=https://gitlab.com/mdds/mdds.git
# do a shallow clone.
git clone --depth 1 --branch $VERSION $REPO_URL $DIR || exit 1
pushd .
cd $DIR
autoconf
rm -rf autom4te.cache .git bin slickedit vsprojects obsolete
popd
tar jcvf $DIR.tar.bz2 $DIR
