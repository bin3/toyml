/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-10-14
 */

#include "dataset.h"

#include <fstream>
#include <boost/algorithm/string.hpp>
#include <glog/logging.h>

namespace toyml {

static const std::string kSeperator = " \t\r\n";

DocumentSet::DocumentSet(): woccurs_(0), idx2freq_done_(false) {
}

DocumentSet::~DocumentSet() {
}

bool DocumentSet::Load(const std::string& fname) {
  typedef std::map<uint32_t, uint32_t> Word2Freq;

  std::ifstream inf(fname);
  if (!inf) {
    LOG(ERROR) << "Failed to open data file " << fname;
    return false;
  }

  Clear();
  std::string line;
  std::vector<std::string> tokens;
  while (std::getline(inf, line)) {
      boost::trim(line);
    boost::split(tokens, line, boost::is_any_of(kSeperator), boost::token_compress_on);
    Word2Freq word2freq;
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i].size() == 0) continue;
      uint32_t word = Index(tokens[i]);
      Word2Freq::iterator it = word2freq.find(word);
      if (it == word2freq.end()) {
        word2freq[word] = 1;
      } else {
        ++it->second;
      }
    }
    Document doc;
    for (Word2Freq::const_iterator it = word2freq.begin(); it != word2freq.end(); ++it) {
      doc.Add(it->first, it->second);
    }
    docs_.push_back(doc);
  }

  // build posting lists
  posts_.resize(DictSize());
  for (std::size_t d = 0; d < docs_.size(); ++d) {
    const Document& doc = docs_[d];
    for (std::size_t p = 0; p < doc.Size(); ++p) {
      posts_[doc.Word(p)].Add(d, doc.Freq(p));
    }
  }

  return true;
}


bool DocumentSet::CalcWordFreq() const {
  if (idx2freq_done_) return true;
  idx2freq_done_ = true;
  idx2freq_.clear();
  std::size_t total_freq = 0;
  for (std::size_t i = 0; i < DocSize(); ++i) {
    const Document& doc = Doc(i);
    for (std::size_t j = 0; j < doc.Size(); ++j) {
      std::size_t word = doc.Word(j);
      std::size_t freq = doc.Freq(j);
      idx2freq_[word] += freq;
      total_freq += freq;
    }
  }
  return true;
}

bool DocumentSet::CalcWordProb(ublas::vector<double>& probs) const {
  if (!CalcWordFreq()) {
    return false;
  }
  probs.resize(idx2freq_.size());
  for (Idx2Freq::const_iterator it = idx2freq_.begin(); it != idx2freq_.end(); ++it) {
    probs(it->first) = static_cast<double>(it->second) / TotalWordOccurs();
  }
  return true;
}

bool DocumentSet::SaveDetailedDict(const std::string& path) const {
  std::ofstream outf(path.c_str());
  if (!outf) {
    return false;
  }
  if (!CalcWordFreq()) {
    return false;
  }

  outf << DictSize() << "\n";
  for (Word2Idx::const_iterator it = word2idx_.begin(); it != word2idx_.end(); ++it) {
    const std::string& word = it->first;
    std::size_t idx = it->second;
    std::size_t freq = idx2freq_[idx];
    double prob = static_cast<double>(freq) / TotalWordOccurs();
    outf << word << "\t" << idx << "\t" << freq << "\t" << prob << "\n";
  }
  outf.close();
  return true;
}

bool DocumentSet::SaveTopFreqWord(const std::string& path, std::size_t topn) const {
  std::ofstream outf(path.c_str());
  if (!outf) {
    return false;
  }
  if (!CalcWordFreq()) {
    return true;
  }
  typedef std::pair<std::size_t, std::size_t> FreqIdx;
  std::vector<FreqIdx> freq_idx_vec;
  for (Idx2Freq::const_iterator it = idx2freq_.begin(); it != idx2freq_.end(); ++it) {
    freq_idx_vec.push_back(FreqIdx(it->second, it->first));
  }
  std::partial_sort(freq_idx_vec.begin(), freq_idx_vec.begin() + topn, freq_idx_vec.end(), std::greater<FreqIdx>());
  outf << topn << "\n";
  for (std::size_t i = 0; i < freq_idx_vec.size() && i < topn; ++i) {
    std::size_t freq = freq_idx_vec[i].first;
    std::size_t idx = freq_idx_vec[i].second;
    const std::string& word = Word(idx);
    double prob = static_cast<double>(freq) / TotalWordOccurs();
    outf << word << "\t" << idx << "\t" << freq << "\t" << prob << "\n";
  }
  outf.close();
  return true;
}


} /* namespace toyml */
