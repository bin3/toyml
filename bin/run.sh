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

#./plsa_main --v=1 --random=0 --log_dir=../log
#./plsa_main --v=1 --docpath=../data/topic/tweet_docs.dat.1k
#./plsa_main --v=1 --docpath=../data/topic/author_docs.dat.clean

#./explsa_main --v=1 --save_interval=1 --iters=1
#./explsa_main --v=4 --docpath=../data/explsa/doc.dat --followeepath=../data/explsa/followee.dat
#./explsa_main --v=1 --lambda=1
#./explsa_main --v=1 --save_interval=1 --threads=1 --lambda=1
#./explsa_main --v=2 --save_interval=40 --lambda=0.8
#./explsa_main --v=2 --save_interval=40 --lambda=1

#./plsa_main --v=1 --random=0 --topics=20
#./background_plsa_main --v=1 --random=0 --log_dir=../log --lambda=0.0 --delta=0.0 --topics=20
#./background_plsa_main --v=1 --random=0 --log_dir=../log --lambda=0.8 --delta=0.1 --topics=10
#./explsa_main --v=1 --lambda=0.8 --super_celebrity=1 --log_interval=1 --save_interval=1
#./explsa_main --v=1 --lambda=0.0 --super_celebrity=0 --log_interval=10

./lda_main --v=2
#./lda_main --v=4 --iters=1
