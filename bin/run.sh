#!/usr/bin/env sh

./mk.sh
if [ ! $? -eq 0 ]; then
  exit 1
fi

for test in `ls *_test`; do
  ./$test
done

./perceptron_demo
