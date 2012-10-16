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

static const std::string kSeperator = " \r\n";

Dataset::Dataset() {
  // TODO Auto-generated constructor stub

}

Dataset::~Dataset() {
  // TODO Auto-generated destructor stub
}

bool Dataset::Load(const std::string& fname) {
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
    boost::split(tokens, line, boost::is_any_of(kSeperator), boost::token_compress_on);
    Word2Freq word2freq;
    for (std::size_t i = 0; i < tokens.size(); ++i) {
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

} /* namespace toyml */
