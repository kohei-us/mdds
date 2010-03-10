#!/bin/bash

VERSION=0.1.1
DIR=mdds_$VERSION
hg clone https://multidimalgorithm.googlecode.com/hg/ $DIR
rm -rf $DIR/.hg
rm -rf $DIR/bin
find $DIR -name '*.vp?' -type f | xargs rm -f
tar jcvf $DIR.tar.bz2 $DIR
