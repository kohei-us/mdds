#!/bin/bash

VERSION=0.3.1
DIR=mdds_$VERSION
hg clone https://multidimalgorithm.googlecode.com/hg/ $DIR
pushd .
cd $DIR
autoconf
rm -rf autom4te.cache .hg .hgtags bin misc autogen.sh configure.ac
popd
find $DIR -name '*.vp?' -type f | xargs rm -f
tar jcvf $DIR.tar.bz2 $DIR
