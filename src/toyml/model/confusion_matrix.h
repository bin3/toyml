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
 * @date		2013-5-20
 */

#ifndef CONFUSION_MATRIX_H_
#define CONFUSION_MATRIX_H_

#include <sstream>
#include <algorithm>
#include <boost/numeric/ublas/matrix.hpp>

#include <toyml/data/dataset.h>

namespace toyml {

/**
 * @brief 
 */
class ConfusionMatrix {
public:
  typedef ublas::matrix<uint64_t> Matrix;

  virtual ~ConfusionMatrix() {}

  template<typename T>
  bool Init(const Data<T>& golds, const Data<T>& preds) {
    if (golds.size() != preds.size()) return false;
    num_instances_ = golds.size();
    num_classes_ = 1 + std::max(*std::max_element(golds.begin(), golds.end()),
        *std::max_element(preds.begin(), preds.end()));

    Matrix(num_classes_, num_classes_, 0).swap(matrix_);
    matrix_.resize(num_classes_, num_classes_, false);
    for (std::size_t i = 0; i < preds.size(); ++i) {
      matrix_(preds(i), golds(i)) += 1;
    }

    accuracy_ = Accuracy(matrix_);
    precision_ = Precision(matrix_);
    recall_ = Recall(matrix_);
    f1_ = F1(precision_, recall_);
    return true;
  }

  std::size_t num_instances() const { return num_instances_; }
  std::size_t num_classes() const { return num_classes_; }
  const Matrix& matrix() const { return matrix_; }
  double accuracy() const { return accuracy_; }
  double precision() const { return precision_; }
  double recall() const { return recall_; }
  double f1() const { return f1_; }
  std::string ToString() const {
    std::stringstream ss;
    ss << "instances=" << num_instances() << ", classes=" << num_classes()
        << ", accuracy=" << accuracy() << ", precision=" << precision() << ", "
        << "recall=" << recall() << ", f1=" << f1();
    return ss.str();
  }
private:
  std::size_t num_instances_;
  std::size_t num_classes_;
  Matrix matrix_;
  double accuracy_;
  double precision_;
  double recall_;
  double f1_;

  static double Div(uint64_t numerator, uint64_t denominator) {
    if (denominator == 0) return 1.0;
    return static_cast<double>(numerator) / denominator;
  }
  static double Accuracy(const Matrix& m) {
    uint64_t total = 0;
    uint64_t correct = 0;
    for (std::size_t i = 0; i < m.size1(); ++i) {
      for (std::size_t j = 0; j < m.size2(); ++j) total += m(i, j);
      correct += m(i, i);
    }
    return Div(correct, total);
  }
  static double Precision(const Matrix& m) {
    double sum = 0;
    for (std::size_t i = 0; i < m.size1(); ++i) {
      uint64_t predsum = 0;
      for (std::size_t j = 0; j < m.size2(); ++j) predsum += m(i, j);
      sum += Div(m(i, i), predsum);
    }
    return sum / m.size1();
  }
  static double Recall(const Matrix& m) {
    double sum = 0;
    for (std::size_t i = 0; i < m.size1(); ++i) {
      uint64_t goldsum = 0;
      for (std::size_t j = 0; j < m.size1(); ++j) goldsum += m(j, i);
      sum += Div(m(i, i), goldsum);
    }
    return sum / m.size1();
  }
  static double F1(double precision, double recall) {
    return 2 * precision * recall / (precision + recall);
  }
};

} /* namespace toyml */
#endif /* CONFUSION_MATRIX_H_ */
