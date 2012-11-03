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
  std::string zdpath;
  std::string wzpath;
  std::string finalsuffix;
  std::string seperator;
  bool random;
  PLSAOptions() :
      niters(100), ntopics(30), eps(1e-3), log_interval(10), save_interval(10), topn(10),
      datadir("./"), topic_path("topics.dat"),
      zdpath("topic-doc-prob.dat"), wzpath("word-topic-prob.dat"),
      finalsuffix("final"), seperator("\t"), random(false) {
  }
  std::string ToString() const {
    std::stringstream ss;
    ss << NVC_(niters);
    ss << NVC_(ntopics);
    ss << NVC_(eps);
    ss << NVC_(log_interval);
    ss << NVC_(save_interval);
    ss << NVC_(topn);
    ss << NVC_(random);
    ss << NV_(datadir);
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
  std::string ToString() const {
    std::stringstream ss;
    ss << NVC_(nd_) << NVC_(nz_) << NV_(nw_);
    return ss.str();
  }
protected:
  PLSAOptions options_;
  const Dataset* dataset_;

  std::size_t nd_;  // number of documents
  std::size_t nw_;  // size of vocabulary
  std::size_t nz_;  // number of topics

  ublas::matrix<double> p_z_d_;        // p(z|d)
  ublas::matrix<double> p_w_z_;        // p(w|z)
  ublas::vector<double> p_z_dw_;       // p(z|d,w)

  ublas::vector<double> p_d_new_;
  ublas::vector<double> p_z_new_;
  ublas::matrix<double> p_z_d_new_;
  ublas::matrix<double> p_w_z_new_;

  std::size_t iter_;    // current iteration

  void RandomizeMatrix(ublas::matrix<double>& mat);
  bool SaveMatrix(const ublas::matrix<double>& mat, const std::string& path) const;

  virtual double LogLikelihood();
  virtual void InitProb();
  virtual void EMStep();
  virtual void Normalize();

  std::string Path(const std::string& fname, const std::string& suffix) const;
};

} /* namespace toyml */
#endif /* PLSA_H_ */
