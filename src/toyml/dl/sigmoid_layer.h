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

#ifndef TOYML_DL_SIGMOID_LAYER_H_
#define TOYML_DL_SIGMOID_LAYER_H_

#include "toyml/dl/layer.h"

namespace toyml {
namespace dl {

/**
 * @brief A layer using sigmoid as neuron function, also called Logistic Regression
 */
class SigmoidLayer : public Layer {
 public:
  SigmoidLayer();
  virtual ~SigmoidLayer();

  using Layer::Train;
  virtual void Train(const Input& x, const Output& y);
  using Layer::Predict;
  virtual void Predict(const Input& x, Output* y) const;
};

} /* namespace dl */
} /* namespace toyml */
#endif /* TOYML_DL_SIGMOID_LAYER_H_ */
