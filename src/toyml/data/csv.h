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
 * @date		2013-5-19
 */

#ifndef CSV_H_
#define CSV_H_

#include <iostream>
#include <string>
#include <glog/logging.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "util.h"
#include "dataset.h"

namespace toyml {

namespace algo = boost::algorithm;

enum LabelPosition {
  FIRST_COLUMN = 0, LAST_COLUMN
};

const char* kCsvDefaultSerparator = "\t, ";
const char* kCsvDefaultComment = "#";

template<typename C>
bool ReadCsv(std::istream& is, C* data, const std::string &separator =
    kCsvDefaultSerparator, const std::string &comment = kCsvDefaultComment) {
  typedef typename C::value_type Sample;
  typedef typename Sample::value_type Value;
  if (!is) return false;

  std::string line;
  std::vector<std::string> toks;
  while (std::getline(is, line)) {
    if (line.empty() || (!comment.empty() && line.find(comment) == 0)) continue;
    algo::split(toks, line, algo::is_any_of(separator));
    Sample sample(toks.size(), 0);
    Value v;
    for (std::size_t i = 0; i < toks.size(); ++i) {
      try {
        v = boost::lexical_cast<Value>(toks[i]);
        sample(i) = v;
      } catch (const boost::bad_lexical_cast& e) {
      }
    }
    data->push_back(sample);
  }
  return true;
}

template<typename Labels, typename Inputs>
bool ReadCsv(std::istream& is, Inputs* inputs, Labels* labels,
    LabelPosition label_pos = LAST_COLUMN, const std::string& separator =
        kCsvDefaultSerparator,
    const std::string &comment = kCsvDefaultComment) {
  typedef typename Labels::value_type Label;
  typedef typename Inputs::value_type Input;
  typedef typename Input::value_type Value;

  if (!is) return false;

  bool first_column = (label_pos == FIRST_COLUMN);
  std::string line;
  std::vector<std::string> toks;
  while (std::getline(is, line)) {
    if (line.empty() || (!comment.empty() && line.find(comment) == 0)) continue;
    algo::trim(line);
    algo::split(toks, line, algo::is_any_of(separator));
    if (toks.size() < 2) {
      LOG(WARNING)<< "toks.size() is too small. toks.size=" << toks.size()
      << ", line=" << line;
      continue;
    }
    Label label;
    try {
      label = boost::lexical_cast<Value>(first_column ?
          toks.front() : toks.back());
    } catch (const boost::bad_lexical_cast& e) {
    }
    Input input(toks.size() - 1, 0);
    std::size_t begin = first_column;
    std::size_t end = toks.size() - (!first_column);
    for (std::size_t i = begin; i < end; ++i) {
      Value v;
      try {
        v = boost::lexical_cast<Value>(toks[i]);
      } catch (const boost::bad_lexical_cast& e) {
      }
      input(i - begin) = v;
    }
    labels->push_back(label);
    inputs->push_back(input);
  }
  return true;
}

template<typename LabeledData>
bool ReadCsv(const std::string& path, LabeledData* data,
    LabelPosition label_pos = LAST_COLUMN, const std::string& separator =
        kCsvDefaultSerparator,
    const std::string& comment = kCsvDefaultComment) {
  typedef typename LabeledData::Input Input;
  typedef typename LabeledData::Label Label;
  std::vector<Input> inputs;
  std::vector<Label> labels;
  std::ifstream inf(path.c_str());
  bool ret = ReadCsv(inf, &inputs, &labels, label_pos, separator, comment);
  if (ret) {
    ret = data->Init(inputs, labels);
  }
  return ret;
}

} /* namespace toyml */
#endif /* CSV_H_ */
