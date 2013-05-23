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
 * @date		2013-5-23
 */

#include "sigmoid_layer.h"
#include <glog/logging.h>
#include <toyml/common/common.h>
#include <toyml/util/util.h>

namespace toyml {
namespace dl {

SigmoidLayer::SigmoidLayer() {
}

SigmoidLayer::~SigmoidLayer() {
}

void SigmoidLayer::Train(const Input& x, const Output& y) {
  Output out;
  Predict(x, &out);

  for (std::size_t i = 0; i < options_.out; ++i) {
    double dy = y(i) - out(i);
    for (std::size_t j = 0; j < options_.in; ++j) {
      w_(i, j) += dy * x(j) * options_.learning_rate;
    }
    b_(i) += dy * options_.learning_rate;
  }
}

void SigmoidLayer::Predict(const Input& x, Output* y) const {
  y->resize(options_.out);
  for (std::size_t i = 0; i < options_.out; ++i) {
    ublas::matrix_row<const RealMatrix> wi(w_, i);
    (*y)(i) = ublas::inner_prod(wi, x) + b_(i);
  }
  Util::Softmax(*y);
}

} /* namespace dl */
} /* namespace toyml */
