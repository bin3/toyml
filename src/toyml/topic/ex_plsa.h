/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-10-21
 */

#ifndef EX_PLSA_H_
#define EX_PLSA_H_

#include <cstddef>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <glog/logging.h>

#include "utils.h"
#include "ex_dataset.h"
#include "plsa.h"

namespace toyml {
using namespace boost::numeric::ublas;

template<typename T>
class UWCTMatrix {
public:
  void Resize(std::size_t nusers, std::size_t nwords, std::size_t ncels, std::size_t ntopics) {
    nu_ = nusers;
    nw_ = nwords;
    nc_ = ncels;
    nt_ = ntopics;
    uw_idx_.resize(nu_, nw_, false);
    ct_vec_.clear();
  }

  T& operator()(uint32_t u, uint32_t w, uint32_t c, uint32_t t) {
    uint32_t* pidx = uw_idx_.find_element(u, w);
    uint32_t idx = 0;
    if (pidx) {
      idx = *pidx;
    } else {
      idx = ct_vec_.size();
      uw_idx_(u, w) = idx;
      ct_vec_.push_back(matrix<T>(nc_, nt_));
    }
    return ct_vec_[idx](c, t);
  }
  const T& operator()(uint32_t u, uint32_t w, uint32_t c, uint32_t t) const {
    uint32_t* pidx = uw_idx_.find_element(u, w);
    CHECK(pidx) << "pidx is NULL. u=" << u << ", w=" << w << ", t=" << t;
    return ct_vec_[*pidx](c, t);
  }
private:
  std::size_t nu_;  // number of users
  std::size_t nc_;  // number of celebrities
  std::size_t nt_;  // number of topics
  std::size_t nw_;  // size of vocabulary

  mapped_matrix<uint32_t> uw_idx_;

  std::vector<matrix<T> > ct_vec_;
};

/**
 * @brief Extended pLSA model options
 */
struct ExPLSAOptions {
  std::size_t niters;
  std::size_t ntopics;  // number of topics
  double eps;
  int log_interval;
  int save_interval;
  std::size_t topn;
  std::string datadir;
  std::string topic_path;
  std::string wtpath;
  std::string tcpath;
  std::string cupath;
  std::string finalsuffix;
  std::string seperator;
  ExPLSAOptions() :
      niters(100), ntopics(100), eps(1e-3), log_interval(10), save_interval(10), topn(20),
      datadir("./"), topic_path("topic-words.dat"), wtpath("word-topic-prob.dat"),
      tcpath("topic-cel-prob.dat"), cupath("cel-user-prob.dat"),
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
 * @brief Extended pLSA model
 */
class ExPLSA {
public:
  virtual ~ExPLSA();
  bool Init(const ExPLSAOptions& options, const Dataset& document_data,
      const Dataset& followee_data);
  std::size_t Train();
  bool SaveModel(int no) const;
  bool SaveModel(const std::string& suffix = "") const;
  bool SaveTopics(const std::string& path) const;
  bool SaveWTModel(const std::string& path) const;
  bool SaveTCModel(const std::string& path) const;
  bool SaveCUModel(const std::string& path) const;
  std::string ToString() const {
    std::stringstream ss;
    ss << NAME_VAL_COMMA(nu_);
    ss << NAME_VAL_COMMA(nc_);
    ss << NAME_VAL_COMMA(nt_);
    ss << NAME_VAL(nw_);
    return ss.str();
  }
private:
  ExPLSAOptions opts_;
  const Dataset* ddata_;  // document dataset
  const Dataset* fdata_;  // followee dataset whose format is similar like document dataset

  std::size_t nu_;  // number of users
  std::size_t nc_;  // number of celebrities
  std::size_t nt_;  // number of topics
  std::size_t nw_;  // size of vocabulary

  matrix<double> p_c_u_;          // p(c|u)
  matrix<double> p_t_c_;          // p(t|c)
  matrix<double> p_w_t_;          // p(w|t)
  UWCTMatrix<double> p_ct_uw_;    // p(c,t|u,w)

  double LogLikelihood();
  void InitProb();
  void Estep();
  void Mstep();

  std::string Path(const std::string& fname, const std::string& suffix) const;
  bool SaveModel(const std::string& path, const matrix<double>& mat,
      std::size_t size1, std::size_t size2) const;
};

} /* namespace toyml */
#endif /* EX_PLSA_H_ */
