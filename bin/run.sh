#!/usr/bin/env sh

cd ../build
make
if [ $? -ne 0 ]
then
    echo "---make failed---"
    exit 1
fi

cd ../bin

#./dataset_main --v=1 --docpath=../data/topic/author_docs.dat.clean

#./plsa_main --v=1
#./plsa_main --v=1 --docpath=../data/topic/tweet_docs.dat.1k
#./plsa_main --v=1 --docpath=../data/topic/author_docs.dat.clean

#./explsa_main --v=1 --save_interval=1 --iters=1
#./explsa_main --v=4 --docpath=../data/explsa/doc.dat --followeepath=../data/explsa/followee.dat
#./explsa_main --v=1 --lambda=1
#./explsa_main --v=1 --save_interval=1 --threads=1 --lambda=1
./explsa_main --v=2 --save_interval=40 --lambda=0.8