#!/bin/bash

PROGDIR=`dirname $0`
VERSION=`cat $PROGDIR/../VERSION`
DIR=mdds_$VERSION
git clone https://code.google.com/p/multidimalgorithm/ $DIR
pushd .
cd $DIR
git checkout mdds-0.5
autoconf
rm -rf autom4te.cache .git bin
popd
find $DIR -name '*.vp?' -type f | xargs rm -f
tar jcvf $DIR.tar.bz2 $DIR
