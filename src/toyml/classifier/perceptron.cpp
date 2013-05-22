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

#include "perceptron.h"
#include <glog/logging.h>

#include <toyml/data/util.h>

namespace toyml {

Perceptron::Perceptron(): b_(0) {
}

Perceptron::~Perceptron() {
}

void Perceptron::Eval(const Input& input, Output* output) const {
  if (input.size() != w_.size()) {
    // TODO
    LOG(WARNING) << "input.size() != w_.size(). input.size=" << input.size()
        << ", w_.size=" << w_.size();
    *output = 0;
    return;
  }
  double z = b_;
  for (std::size_t i = 0; i < w_.size(); ++i) {
    z += w_(i) * input(i);
  }
  *output = (z >= 0);
}

bool Perceptron::Train(const ClassificationData& data) {
  VLOG(0) << "Train";
  RealVector(data.dimension(), 0).swap(w_);
  b_ = 0;
  for (std::size_t iter = 0; iter < opts_.niters; ++iter) {
    std::size_t err_cnt = 0;
    for (std::size_t i = 0; i < data.num_instances(); ++i) {
      Output y = (*this)(data.input(i));
      if (y != data.label(i)) {
        ++err_cnt;
        int sy = Sign(data.label(i));
        w_ += opts_.learning_rate * sy * data.input(i);
        b_ += opts_.learning_rate * sy;
//        VLOG_EVERY_N(0, 500) << NV_(w_);
//        VLOG_EVERY_N(0, 500) << NV_(data.input(i));
//        VLOG_EVERY_N(0, 500) << NVC_(y) << NV_(sy);
      }
    }
    VLOG(0) << "iter#" << iter << ": " << NVC_(err_cnt) << NVC_(w_) << NV_(b_);
    if (err_cnt == 0) break;
  }
  return true;
}

} /* namespace toyml */
