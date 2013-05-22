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
 * @date		2013-5-21
 */

#ifndef PERCEPTRON_H_
#define PERCEPTRON_H_

#include "classifier.h"

namespace toyml {

/**
 * @brief 
 */
class Perceptron: public Classifier {
public:
  struct Options {
    Options(): niters(100), learning_rate(0.01) {}
    std::size_t niters;
    double learning_rate;
  };

  Perceptron();
  virtual ~Perceptron();

  bool Init(const Options& options) {
    opts_ = options;
    return true;
  }
  virtual std::string name() const { return "Perceptron"; }

  virtual void Eval(const Input& input, Output* output) const;
  using Classifier::Eval;
  virtual bool Train(const ClassificationData& data);
private:
  Options opts_;
  RealVector w_;
  double b_;

  static int Sign(double x) {
    return x >= 0 ? 1 : -1;
  }
};
} /* namespace toyml */
#endif /* PERCEPTRON_H_ */
