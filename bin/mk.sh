#!/bin/sh

cd ../
rootdir=`pwd`

builddir=${rootdir}/build
bindir=${rootdir}/bin

cd ${builddir}
make
if [ $? -ne 0 ]
then
    echo "[make failed]"
    exit 1
fi
echo "[make succeeded]"

