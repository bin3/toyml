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
 * @date		2013-5-17
 */

#ifndef MODEL_H_
#define MODEL_H_

#include <string>

#include <toyml/common/inameable.h>
#include <toyml/data/dataset.h>

namespace toyml {

/**
 * @brief 
 */
template<typename InputT, typename OutputT>
class Model: public INameable {
public:
  typedef InputT Input;
  typedef OutputT Output;
  typedef Data<InputT> Inputs;
  typedef Data<OutputT> Outputs;

  virtual ~Model() {}

  virtual std::string name() const { return "Model"; }

  virtual void Predict(const Input& input, Output* output) const = 0;
  virtual void Predict(const Inputs& inputs, Outputs* outputs) const {
    outputs->resize(inputs.size());
    Output output;
    for (std::size_t i = 0; i < inputs.size(); ++i) {
      Predict(inputs[i], &(*outputs)[i]);
    }
  }
  Output operator()(const Input& input) const {
    Output output;
    Predict(input, &output);
    return output;
  }
  Outputs operator()(const Inputs& inputs) const {
    Outputs outputs;
    Predict(inputs, &outputs);
    return outputs;
  }

  virtual bool Read(const std::string& path) { return false; }
  virtual bool Write(const std::string& path) { return false; }
};

} /* namespace toyml */
#endif /* MODEL_H_ */
