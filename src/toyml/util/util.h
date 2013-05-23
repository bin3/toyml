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

#ifndef TOYML_UTIL_UTIL_H_
#define TOYML_UTIL_UTIL_H_

#include <algorithm>

#include <toyml/common/common.h>

namespace toyml {

#define NAME_VAL(v) #v << "=" << v
#define NAME_VAL_COMMA(v) #v << "=" << v << ", "
#define NV_ NAME_VAL
#define NVC_ NAME_VAL_COMMA

/**
 * @brief 
 */
class Util {
 public:
  static void Softmax(RealVector& x) {
    double max = *std::max_element(x.begin(), x.end());
    double sum = 0;
    for (std::size_t i = 0; i < x.size(); ++i) {
      x(i) = exp(x(i) - max);
      sum += x(i);
    }
    for (std::size_t i = 0; i < x.size(); ++i) {
      x(i) /= sum;
    }
  }
};

} /* namespace toyml */
#endif /* TOYML_UTIL_UTIL_H_ */
