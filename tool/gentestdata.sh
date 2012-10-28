#!/usr/bin/env sh

n=10
#echo $*
if [ ! -z $1 ]
then
    n=$1
fi
echo "n=$n"

dfile=doc.dat
ffile=followee.dat
suffix=test

head -$n $dfile > $dfile.$suffix
head -$n $ffile > $ffile.$suffix
