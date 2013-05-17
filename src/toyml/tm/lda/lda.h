/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-11-22
 */

#ifndef LDA_H_
#define LDA_H_

#include <string>

#include <toyml/tm/utils.h>
#include <toyml/tm/dataset.h>

namespace toyml {

struct LDAOptions {
  double alpha;
  double beta;
  std::size_t topics;  // number of topics
  std::size_t iters;
  double eps;
  int nlog;
  int nsave;
  std::size_t topn;
  std::string datadir;
  std::string finalsuffix;
  std::string seperator;
  std::string zpath;
  std::string zwpath;
  std::string dzpath;
  bool random;
  LDAOptions() :
      alpha(50.0), beta(0.1), topics(30), iters(100), eps(1e-3), nlog(
          10), nsave(10), topn(10), datadir("./"), finalsuffix("final"), seperator(
          "\t"), zpath("topics.dat"), zwpath("topic-word-prob.dat"), dzpath("doc-topic-prob.dat"), random(false) {
  }
  std::string ToString() const {
    std::stringstream ss;
    ss << NVC_(alpha) << NVC_(beta);
    ss << NVC_(iters);
    ss << NVC_(topics);
    ss << NVC_(eps);
    ss << NVC_(nlog);
    ss << NVC_(nsave);
    ss << NVC_(topn);
    ss << NVC_(random);
    ss << NV_(datadir);
    return ss.str();
  }
};

/**
 * @brief 
 */
class LDA {
public:
  LDA();
  virtual ~LDA();

  bool Init(const LDAOptions& options, const Dataset& dataset);
  bool Train();
};

} /* namespace toyml */
#endif /* LDA_H_ */
