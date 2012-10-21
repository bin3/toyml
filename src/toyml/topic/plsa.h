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

template<typename T>
class ZDWMatrix {
public:
  void Resize(std::size_t ndocs, std::size_t nwords, std::size_t ntopics) {
    nd_ = ndocs;
    nw_ = nwords;
    nz_ = ntopics;
    dw_idx_.resize(nd_, nw_, false);
    zvec_.clear();
  }
  void Set(uint32_t d, uint32_t w, uint32_t z, T p) {
    uint32_t* pidx = dw_idx_.find_element(d, w);
    if (pidx) {
      zvec_[*pidx][z] = p;
    } else {
      uint32_t idx = zvec_.size();
      zvec_.push_back(std::vector<T>(nz_));
    }
  }
  T Get(uint32_t d, uint32_t w, uint32_t z) {
    uint32_t* pidx = dw_idx_.find_element(d, w);
    if (pidx) {
      return zvec_[*pidx][z];
    } else {
      LOG(WARNING) << "No value for d=" << d << ", w=" << w << ", z=" << z;
      return T();
    }
  }
  T& operator()(uint32_t d, uint32_t w, uint32_t z) {
    uint32_t* pidx = dw_idx_.find_element(d, w);
    uint32_t idx = 0;
    if (pidx) {
      idx = *pidx;
    } else {
      idx = zvec_.size();
      dw_idx_(d, w) = idx;
      zvec_.push_back(std::vector<T>(nz_));
    }
    return zvec_[idx][z];
  }
  const T& operator()(uint32_t d, uint32_t w, uint32_t z) const {
    uint32_t* pidx = dw_idx_.find_element(d, w);
    CHECK(pidx) << "pidx is NULL. d=" << d << ", w=" << w << ", z=" << z;
    return zvec_[*pidx][z];
  }
private:
  std::size_t nd_; // number of documents
  std::size_t nw_; // size of vocabulary
  std::size_t nz_; // number of topics

  mapped_matrix<uint32_t> dw_idx_;
  std::vector<std::vector<T> > zvec_;
};

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
  PLSAOptions() :
      niters(100), ntopics(100), eps(1e-3), log_interval(10), save_interval(10), topn(20),
      datadir("./"), topic_path("topic-words.dat"), tpath("topic-prob.dat"),
      dzpath("doc-topic-prob.dat"), wzpath("word-topic-prob.dat"),
      finalsuffix("final"), seperator("\t") {
  }
  std::string ToString() const {
    std::stringstream ss;
    ss << NAME_VAL(niters) << ", ";
    ss << NAME_VAL(ntopics) << ", ";
    ss << NAME_VAL(eps) << ", ";
    ss << NAME_VAL(log_interval) << ", ";
    ss << NAME_VAL(save_interval) << ", ";
    ss << NAME_VAL(topn) << ", ";
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
private:
  PLSAOptions opts_;
  const Dataset* dataset_;

  std::size_t nd_;  // number of documents
  std::size_t nw_;  // size of vocabulary
  std::size_t nz_;  // number of topics
  std::size_t nr_;  // number of total word occurs

  std::vector<double> p_z_;     // p(z)
  matrix<double> p_d_z_;        // p(d|z)
  matrix<double> p_w_z_;        // p(w|z)
  ZDWMatrix<double> p_z_dw_;    // p(z|d,w)

  double LogLikelihood();
  void InitProb();
  void Mstep();
  void Estep();

  std::string Path(const std::string& fname, const std::string& suffix) const;
};

} /* namespace toyml */
#endif /* PLSA_H_ */
