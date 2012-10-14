/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-10-14
 */

#ifndef DATASET_H_
#define DATASET_H_

#include <stdint.h>
#include <sstream>
#include <string>
#include <vector>
#include <map>

namespace toyml {

class Document {
public:
  void Add(uint32_t word, uint32_t freq) {
    wfl_.push_back(WordFreq(word, freq));
  }
  uint32_t Word(uint32_t idx) const {
    return wfl_[idx].first;
  }
  uint32_t Freq(uint32_t idx) const {
    return wfl_[idx].second;
  }
  uint32_t Size() const {
    return wfl_.size();
  }
private:
  typedef std::pair<uint32_t, uint32_t> WordFreq;
  typedef std::vector<WordFreq> WordFreqList;
  WordFreqList wfl_;
};

/**
 * @brief 
 */
class Dataset {
public:
  Dataset();
  virtual ~Dataset();
  bool Load(const std::string& fname);
  const Document& Doc(uint32_t idx) const {
    return docs_[idx];
  }
  uint32_t Size() const {
    return docs_.size();
  }
  uint32_t DicSize() const {
    return word2idx_.size();
  }
  std::string StatString() const {
    std::stringstream ss;
    ss << "Size=" << Size() << ", DicSize=" << DicSize();
    return ss.str();
  }
private:
  typedef std::map<std::string, uint32_t> Word2Idx;
  Word2Idx word2idx_;
  std::vector<Document> docs_;

  uint32_t Index(const std::string& word) {
    Word2Idx::const_iterator it = word2idx_.find(word);
    uint32_t idx = 0;
    if (it == word2idx_.end()) {
      idx = word2idx_.size();
      word2idx_[word] = idx;
    } else {
      idx = it->second;
    }
    return idx;
  }
};

} /* namespace toyml */
#endif /* DATASET_H_ */
