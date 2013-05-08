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

#include "instance.h"
#include <fstream>
#include <glog/logging.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace toyml {

namespace algo = boost::algorithm;

bool Instances::Load(const std::string& path) {
  std::ifstream ifs(path.c_str());
  if (!ifs) {
    return false;
  }
  std::string line;
  std::vector<std::string> toks;
  while (std::getline(ifs, line)) {
    algo::split(toks, line, algo::is_any_of(" \t"));
    if (toks.size() < 2) {
      LOG(WARNING) << "Invalid line: " << line;
      continue;
    }
    uint32_t label = boost::lexical_cast<uint32_t>(toks[0]);
    Instance& inst = Add(toks.size() - 1, label);
    for (std::size_t i = 1; i < toks.size(); ++i) {
      std::size_t colon = toks[i].find(':');
      std::size_t idx = boost::lexical_cast<std::size_t>(toks[i].substr(0, colon));
      double x = boost::lexical_cast<double>(toks[i].substr(colon + 1));
      inst[idx] = x;
    }
  }
  return true;
}

} /* namespace toyml */

