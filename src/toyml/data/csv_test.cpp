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

namespace toyml {

TEST(Csv, ReadCsv) {
  LabeledData<RealVector, uint32_t> data;
  EXPECT_EQ(0U, data.size());
  EXPECT_FALSE(ReadCsv("null/null", &data));
  ASSERT_TRUE(ReadCsv("testdata/data/cls.10.csv", &data, LAST_COLUMN));
  const std::size_t kN = 10;
  EXPECT_EQ(kN, data.size());
  uint32_t expected_labels_arr[kN] = {0, 1, 0, 1, 0, 1, 0, 1, 0, 1};
  Data<uint32_t> expected_labels;
  ToData(expected_labels_arr, kN, &expected_labels);
  EXPECT_EQ(expected_labels, data.labels());
  EXPECT_EQ(2U, data.input(0).size());
  EXPECT_DOUBLE_EQ(2.4114, data.input(0)(0));
  EXPECT_DOUBLE_EQ(-3.8901, data.input(0)(1));

  ASSERT_TRUE(ReadCsv("testdata/data/cls.csv", &data));
  EXPECT_EQ(1000U, data.size());

  LabeledData<RealVector, double> data2;
  ASSERT_TRUE(ReadCsv("testdata/data/cls.10.csv", &data2, FIRST_COLUMN));
  EXPECT_EQ(10U, data2.size());
  EXPECT_DOUBLE_EQ(2.4114, data2.label(0));
  EXPECT_EQ(2U, data2.input(0).size());
  EXPECT_DOUBLE_EQ(-3.8901, data2.input(0)(0));
  EXPECT_DOUBLE_EQ(0, data2.input(0)(1));
}

} /* namespace toyml */
