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

#ifndef CLASSIFIER_H_
#define CLASSIFIER_H_

#include <vector>

#include "instance.h"

namespace toyml {

/**
 * @brief 
 */
class Classifier {
public:
  Classifier();
  virtual ~Classifier();

  virtual bool Train(const Instances& insts) = 0;
  virtual bool Classify(const Instance& inst, std::vector<double>* outs) const = 0;
  virtual int Classify(const Instance& inst) const = 0;
  std::size_t NumClasses() const;
};

class OnlineClassifier: public Classifier {
public:
  virtual bool OnlineTrain(const Instance& inst) = 0;
};

} /* namespace toyml */
#endif /* CLASSIFIER_H_ */
