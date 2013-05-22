/*
 * Copyright (c) 2013 Binson Zhang.
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2013-5-19
 */

#include <iostream>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include <toyml/data/csv.h>
#include <toyml/model/confusion_matrix.h>
#include <toyml/classifier/perceptron.h>

DEFINE_string(trainpath, "data/classifier/train.csv", "the training data file");
DEFINE_string(testpath, "data/classifier/test.csv", "the test data file");

int main(int argc, char **argv) {
  FLAGS_stderrthreshold = 0;
  FLAGS_log_dir = "log/";
  google::SetVersionString("0.0.1");
  google::SetUsageMessage(argv[0] + std::string(" [OPTIONS]"));
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  VLOG(0) << "------" << argv[0] << "------";

  toyml::ClassificationData train;
  toyml::ClassificationData test;
  CHECK(toyml::ReadCsv(FLAGS_trainpath, &train));
  CHECK(toyml::ReadCsv(FLAGS_testpath, &test));
  VLOG(0) << "Training data: " << train.ToString();
  VLOG(0) << "Test data: " << test.ToString();

  toyml::Perceptron::Options options;
  options.niters = 100;
  toyml::Perceptron m;
  CHECK(m.Init(options));
  CHECK(m.Train(train)) << "Failed to train " << m.name() << " model.";
  toyml::Perceptron::Outputs outputs = m(test.inputs());
//  VLOG(0) << "inputs: " << test.inputs();
//  VLOG(0) << "labels: " << test.labels();
//  VLOG(0) << "outputs: " << outputs;

  toyml::ConfusionMatrix cm;
  CHECK(cm.Init(test.labels(), outputs));
  VLOG(0) << "Evaluation: " << cm.ToString();

  return 0;
}
