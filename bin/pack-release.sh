#!/bin/bash

PROGDIR=`dirname $0`
VERSION=`cat $PROGDIR/../VERSION`
DIR=mdds_$VERSION
hg clone https://multidimalgorithm.googlecode.com/hg/ $DIR
pushd .
cd $DIR
autoconf
rm -rf autom4te.cache .hg .hgtags bin
popd
find $DIR -name '*.vp?' -type f | xargs rm -f
tar jcvf $DIR.tar.bz2 $DIR
