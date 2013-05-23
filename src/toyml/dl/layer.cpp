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

#include "layer.h"
#include <sstream>
#include <algorithm>
#include <glog/logging.h>

#include <toyml/util/util.h>

namespace toyml {
namespace dl {

Layer::Layer() {
}

Layer::~Layer() {
}

std::string Layer::Options::ToString() const {
  std::stringstream ss;
  ss << NVC_(in) << NVC_(out) << NVC_(max_iterations) << NV_(learning_rate);
  return ss.str();
}

bool Layer::Init(const Options& options) {
  options_ = options;
  RealMatrix(options_.out, options_.in, 0).swap(w_);
  RealVector(options_.out, 0).swap(b_);
  return true;
}

bool Layer::Train(const NNetData& data) {
  for (std::size_t iter = 0; iter < options_.max_iterations; ++iter) {
    VLOG(2) << "iteration#" << iter;
    for (std::size_t i = 0; i < data.size(); ++i) {
      VLOG(4) << "instance#" << i;
      Train(data.input(i), data.label(i));
    }
  }
  return true;
}

void Layer::Predict(const Input& x, ClassificationData::Output* y) const {
  Output out;
  Predict(x, &out);
  *y = std::distance(out.begin(), std::max_element(out.begin(), out.begin()));
}

std::string Layer::ToString() const {
  std::stringstream ss;
  ss << NVC_(options_) << NVC_(w_) << NV_(b_);
  return ss.str();
}

} /* namespace dl */
} /* namespace toyml */
