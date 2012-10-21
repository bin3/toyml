/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-10-21
 */

#include "ex_dataset.h"

#include <glog/logging.h>

namespace toyml {

ExDataset::ExDataset() {
  // TODO Auto-generated constructor stub

}

ExDataset::~ExDataset() {
  // TODO Auto-generated destructor stub
}

bool ExDataset::Load(const std::string& doc_fname,
    const std::string& followee_fname) {
  if (!Dataset::Load(doc_fname)) return false;

  if (!followee_data_.Load(followee_fname)) {
    LOG(ERROR) << "Failed to load followee file " << followee_fname;
    return false;
  }

  if (DocSize() != NumUsers()) {
    LOG(ERROR) << "The number of documents and users are not equal. docs="
        << DocSize() << ", users=" << NumUsers();
    return false;
  }

  followees_vec_.reserve(followee_data_.DocSize());
  for (std::size_t i = 0; i < followee_data_.DocSize(); ++i) {
    const Document& doc = followee_data_.Doc(i);
    followees_vec_.push_back(Followees());
    Followees& f = followees_vec_.back();
    for (std::size_t j = 0; j < doc.Size(); ++j) {
      f.push_back(doc.Word(j));
    }
  }

  return true;
}

const Followees& ExDataset::GetFollowees(uint32_t uid) const {
  return followees_vec_[uid];
}

void ExDataset::Clear() {
  Dataset::Clear();
  followee_data_.Clear();
  followees_vec_.clear();
}

} /* namespace toyml */
