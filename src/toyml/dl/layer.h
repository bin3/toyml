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

#ifndef TOYML_DL_LAYER_H_
#define TOYML_DL_LAYER_H_

#include <vector>

#include <toyml/common/ito_string.h>
#include <toyml/util/util.h>
#include <toyml/data/dataset.h>
#include <toyml/model/model.h>


namespace toyml {
namespace dl {

typedef NNetData::Input Input;
typedef NNetData::Output Output;

/**
 * @brief Layer of neurons in a neural network
 */
class Layer : public IToString , public Model<RealVector, RealVector> {
 public:
  typedef Model<RealVector, RealVector> Model;
  struct Options : public IToString {
    Options(): in(4), out(2), max_iterations(10), learning_rate(0.01) {}
    virtual std::string ToString() const;
    std::size_t in;         ///< input dimension
    std::size_t out;        ///< output dimension
    std::size_t max_iterations;        ///< maximum iterations
    double learning_rate;   ///< learning rate
  };

  Layer();
  virtual ~Layer();

  virtual bool Init(const Options& options);
  virtual bool Train(const NNetData& data);
  virtual void Train(const Input& x, const Output& y) = 0;
  using Model::Predict;
  virtual void Predict(const Input& x, ClassificationData::Output* y) const;
  virtual void Predict(const Inputs& inputs, ClassificationData::Outputs* outputs) const {
    outputs->resize(inputs.size());
    for (std::size_t i = 0; i < inputs.size(); ++i) {
      Predict(inputs[i], &(*outputs)[i]);
    }
  }
  virtual std::string ToString() const;
 protected:
  Options options_;

  RealMatrix w_;    ///< w_[i][j] is the weight of the edge between output i and input j
  RealVector b_;   ///< biases for each output unit
};

} /* namespace dl */
} /* namespace toyml */
#endif /* TOYML_DL_LAYER_H_ */
