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

#ifndef TOYML_DATA_DATASET_H_
#define TOYML_DATA_DATASET_H_

#include <stdint.h>
#include <fstream>
#include <sstream>
#include <string>
#include <deque>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

namespace toyml {

namespace ublas = boost::numeric::ublas;

typedef ublas::vector<double> RealVector;

template<typename T>
class Data: public ublas::vector<T> {
public:
  virtual ~Data() {}

  virtual bool Read(const std::string& path) { return false; }
  virtual bool Write(const std::string& path) { return false; }
};

template<typename T>
class UnlabeledData : public Data<T> {

};

template<typename InputT, typename LabelT>
class LabeledData {
public:
  typedef InputT Input;
  typedef LabelT Label;
  typedef UnlabeledData<InputT> Inputs;
  typedef Data<LabelT> Labels;

  virtual ~LabeledData() {}

  const Labels& labels() const { return labels_; }
  Labels& labels() { return labels_; }
  const Inputs& inputs() const { return inputs_; }
  Inputs& inputs() { return inputs_; }
  Label& label(std::size_t i) { return labels_[i]; }
  Input& input(std::size_t i) { return inputs_[i]; }
  std::size_t Size() const { return labels_.size(); }

  virtual bool Read(const std::string& path) {
    std::ifstream inf(path.c_str());
    if (!inf) return false;
    std::string line;
    while (std::getline(inf, line)) {

    }
    return true;
  }
  virtual bool Write(const std::string& path) { return false; }
private:
  Inputs inputs_;
  Labels labels_;
};

typedef LabeledData<RealVector, uint32_t> ClassificationData;
typedef LabeledData<RealVector, RealVector> RegressionData;

template<typename T>
bool operator==(const Data<T>& a, const Data<T>& b) {
  if (a.size() != b.size()) return false;
  for (std::size_t i = 0; i < a.size(); ++i) {
    if (a(i) != b(i)) return false;
  }
  return true;
}

} /* namespace toyml */
#endif /* TOYML_DATA_DATASET_H_ */
