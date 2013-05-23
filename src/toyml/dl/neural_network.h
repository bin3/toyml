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

#ifndef TOYML_DL_NEURAL_NETWORK_H_
#define TOYML_DL_NEURAL_NETWORK_H_

#include <vector>

#include "toyml/dl/layer.h"

namespace toyml {
namespace dl {

/**
 * @brief 
 */
class NeuralNetwork {
 public:
  typedef std::vector<Layer*> Layers;

  NeuralNetwork();
  virtual ~NeuralNetwork();

  Layers& layers() { return layers_; }
  Layer* layer(std::size_t i) { return layers_[i]; }
  Layer* input_layer() { return layers_.front(); }
  Layer* output_layer() { return layers_.back(); }

  std::size_t NumLayers() const { return layers_.size(); }
 private:
  Layers layers_;
};

} /* namespace dl */
} /* namespace toyml */
#endif /* TOYML_DL_NEURAL_NETWORK_H_ */
