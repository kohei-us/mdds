#!/bin/bash

PROGDIR=`dirname $0`
VERSION=`cat $PROGDIR/../VERSION`
DIR=mdds_$VERSION
# do a shallow clone.
git clone --depth 1 --branch $VERSION https://gitlab.com/mdds/mdds.git $DIR
pushd .
cd $DIR
autoconf
rm -rf autom4te.cache .git bin slickedit vsprojects include/obsolete
popd
tar jcvf $DIR.tar.bz2 $DIR
