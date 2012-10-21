/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-10-21
 */

#ifndef EX_DATASET_H_
#define EX_DATASET_H_

#include "dataset.h"

namespace toyml {

typedef std::vector<int> Followees;

/**
 * @brief 
 */
class ExDataset: public Dataset {
public:
  ExDataset();
  virtual ~ExDataset();
  bool Load(const std::string& doc_fname, const std::string& followee_fname);
  bool SaveCelebrities(const std::string path) const {
    return followee_data_.SaveDict(path);
  }
  std::size_t NumUsers() const {
    return followee_data_.DocSize();
  }
  std::size_t NumCelebrities() const {
    return followee_data_.DictSize();
  }
  std::size_t TotalCelebrities() const {
    return followee_data_.TotalWordOccurs();
  }
  const Followees& GetFollowees(uint32_t uid) const;
  void Clear();
  std::string StatString() const {
    std::stringstream ss;
    ss << Dataset::StatString() << "\nNumUsers=" << NumUsers() << ", NumCelebrities="
        << NumCelebrities() << ", TotalCelebrities=" << TotalCelebrities();
    return ss.str();
  }
protected:
  std::vector<Followees> followees_vec_;
  Dataset doc_data_;
  Dataset followee_data_;
};

} /* namespace toyml */
#endif /* EX_DATASET_H_ */
