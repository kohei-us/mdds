#!/bin/bash

PROGDIR=`dirname $0`
VERSION=`cat $PROGDIR/../VERSION`
DIR=mdds_$VERSION
git clone https://code.google.com/p/multidimalgorithm/ $DIR
pushd .
cd $DIR
autoconf
rm -rf autom4te.cache .git bin slickedit vsprojects
popd
tar jcvf $DIR.tar.bz2 $DIR
