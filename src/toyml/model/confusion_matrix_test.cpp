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

#include "confusion_matrix.h"
#include <gtest/gtest.h>

namespace toyml {

TEST(ConfusionMatrix, Init) {
  ConfusionMatrix m;

  const std::size_t kN = 10;
  uint32_t golds_arr[kN] = {0, 0, 0, 0, 1, 1, 1, 1, 2, 3};
  uint32_t preds_arr[kN] = {0, 0, 1, 1, 0, 0, 1, 1, 2, 3};
  Data<uint64_t> golds;
  Data<uint64_t> preds;
  ToData(golds_arr, kN, &golds);

  EXPECT_FALSE(m.Init(golds, preds));

  EXPECT_TRUE(m.Init(golds, golds));
  EXPECT_EQ(kN, m.num_instances());
  EXPECT_EQ(4U, m.num_classes());
  EXPECT_DOUBLE_EQ(1.0, m.accuracy());
  EXPECT_DOUBLE_EQ(1.0, m.precision());
  EXPECT_DOUBLE_EQ(1.0, m.recall());
  EXPECT_DOUBLE_EQ(1.0, m.f1());

  ToData(preds_arr, kN, &preds);
  EXPECT_TRUE(m.Init(golds, preds));
  EXPECT_EQ(kN, m.num_instances());
  EXPECT_EQ(4U, m.num_classes());
  EXPECT_DOUBLE_EQ(0.6, m.accuracy());
  EXPECT_DOUBLE_EQ(0.75, m.precision());
  EXPECT_DOUBLE_EQ(0.75, m.recall());
  EXPECT_DOUBLE_EQ(0.75, m.f1());
}

} /* namespace toyml */
