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

#include "csv.h"
#include <gtest/gtest.h>
//#include <glog/logging.h>

namespace toyml {

TEST(Csv, ReadCsv) {
  ClassificationData data;
  EXPECT_EQ(0U, data.Size());
  EXPECT_FALSE(ReadCsv("null/null", &data));
  ASSERT_TRUE(ReadCsv("testdata/data/cls.10.csv", &data, 2));
  const std::size_t kN = 10;
  EXPECT_EQ(kN, data.Size());
  uint32_t expected_labels_arr[kN] = {0, 1, 0, 1, 0, 1, 0, 1, 0, 1};
  Data<uint32_t> expected_labels;
  ToData(expected_labels_arr, kN, &expected_labels);
  EXPECT_EQ(expected_labels, data.labels());
//  LOG(ERROR) << data.labels();
//  LOG(ERROR) << data.inputs();
  ASSERT_TRUE(ReadCsv("testdata/data/cls.csv", &data));
  EXPECT_EQ(1000U, data.Size());
}

} /* namespace toyml */
