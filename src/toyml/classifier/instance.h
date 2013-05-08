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
 * @date		2013-5-8
 */

#ifndef INSTANCE_H_
#define INSTANCE_H_

#include <stdint.h>
#include <string>
#include <vector>
#include <deque>

namespace toyml {

/**
 * @brief 
 */
class Instance {
public:
  Instance(std::size_t sz = 0, uint32_t lb = 0): data_(sz, 0), label_(lb) {}
  virtual ~Instance() {}

  double& operator[](std::size_t i) { return data_[i]; }
  const double& operator[](std::size_t i) const { return data_[i]; }
  std::size_t size() const { return data_.size(); }
  void resize(std::size_t sz) { data_.resize(sz); }
  uint32_t label() const { return label_; }
  void set_label(uint32_t lb) { label_ = lb; }
protected:
  std::vector<double> data_;
  uint32_t label_;
};

class SparseInstance: public Instance {};
class BinaryInstance: public Instance {};

class Instances {
public:
  Instances(): nclasses_(0), nfeatures(0) {}
  virtual ~Instances() {}

  // load libsvm-format data file
  bool Load(const std::string& path);
  void Add(const Instance& inst) { insts_.push_back(inst); }
  Instance& Add(std::size_t sz = 0, uint32_t label = 0) {
    insts_.push_back(Instance(sz, label));
    return insts_.back();
  }
  const Instance& operator[](std::size_t i) const { return insts_[i]; }
  std::size_t size() const { return insts_.size(); }
  std::size_t NumClasses() const { return nclasses_; }
protected:
  std::deque<Instance> insts_;
  std::size_t nclasses_;
  std::size_t nfeatures;
};

} /* namespace toyml */
#endif /* INSTANCE_H_ */
