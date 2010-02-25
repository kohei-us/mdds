#!/bin/bash

VERSION=0.1
DIR=mdds_$VERSION
svn checkout http://multidimalgorithm.googlecode.com/svn/trunk/ $DIR
find ./$DIR -name '.svn' -type d | xargs rm -r || exit 1
tar jcvf $DIR.tar.bz2 $DIR
