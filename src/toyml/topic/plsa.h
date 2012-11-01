/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-10-14
 */

#ifndef PLSA_H_
#define PLSA_H_

#include <cstddef>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <glog/logging.h>

#include "utils.h"
#include "dataset.h"

namespace toyml {

using namespace boost::numeric::ublas;
namespace ublas = boost::numeric::ublas;

/**
 * @brief pLSA model options
 */
struct PLSAOptions {
  std::size_t niters;
  std::size_t ntopics;  // number of topics
  double eps;
  int log_interval;
  int save_interval;
  std::size_t topn;
  std::string datadir;
  std::string topic_path;
  std::string tpath;
  std::string dzpath;
  std::string wzpath;
  std::string finalsuffix;
  std::string seperator;
  bool random;
  PLSAOptions() :
      niters(100), ntopics(100), eps(1e-3), log_interval(10), save_interval(10), topn(20),
      datadir("./"), topic_path("topics.dat"), tpath("topic-prob.dat"),
      dzpath("doc-topic-prob.dat"), wzpath("word-topic-prob.dat"),
      finalsuffix("final"), seperator("\t"), random(true) {
  }
  std::string ToString() const {
    std::stringstream ss;
    ss << NAME_VAL_COMMA(niters);
    ss << NAME_VAL_COMMA(ntopics);
    ss << NAME_VAL_COMMA(eps);
    ss << NAME_VAL_COMMA(log_interval);
    ss << NAME_VAL_COMMA(save_interval);
    ss << NAME_VAL_COMMA(topn);
    ss << NAME_VAL_COMMA(random);
    ss << NAME_VAL(datadir);
    return ss.str();
  }
};

/**
 * @brief pLSA model
 */
class PLSA {
public:
  virtual ~PLSA();
  bool Init(const PLSAOptions& options, const Dataset& dataset);
  std::size_t Train();
  bool SaveModel(int no) const;
  bool SaveModel(const std::string& suffix = "") const;
  bool SaveTopics(const std::string& path) const;
  bool SaveTModel(const std::string& path) const;
  bool SaveDZModel(const std::string& path) const;
  bool SaveWZModel(const std::string& path) const;
protected:
  PLSAOptions options_;
  const Dataset* dataset_;

  std::size_t nd_;  // number of documents
  std::size_t nw_;  // size of vocabulary
  std::size_t nz_;  // number of topics
  std::size_t nr_;  // number of total word occurs

  ublas::vector<double> p_z_;          // p(z)
  ublas::matrix<double> p_d_z_;        // p(d|z)
  ublas::matrix<double> p_w_z_;        // p(w|z)

  double znorm_;
  ublas::vector<double> p_z_new_;
  ublas::matrix<double> p_d_z_new_;
  ublas::matrix<double> p_w_z_new_;
  ublas::vector<double> p_z_dw_;

  virtual double LogLikelihood();
  virtual void InitProb();
  virtual void EMStep();
  virtual void Normalize();

  std::string Path(const std::string& fname, const std::string& suffix) const;
};

} /* namespace toyml */
#endif /* PLSA_H_ */
