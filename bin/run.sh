#!/usr/bin/env sh

./mk.sh
if [ ! $? -eq 0 ]; then
  exit 1
fi

./csv_test
./perception_demo
