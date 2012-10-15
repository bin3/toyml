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
private:
  std::size_t nd_; // number of documents
  std::size_t nw_; // size of vocabulary
  std::size_t nz_; // number of topics

  mapped_matrix<uint32_t> dw_idx_;
  std::vector<std::vector<T> > zvec_;
};

/**
 * @brief pLSA model
 */
class PLSA {
public:
  PLSA();
  virtual ~PLSA();
  bool Init(const Dataset& dataset, std::size_t ntopics, std::size_t niterators) {
    dataset_ = &dataset;
    niters_ = niterators;

    nd_ = dataset.DocSize();
    nw_ = dataset.DictSize();
    nz_ = ntopics;
    nr_ = TotalWordOccurs(dataset);

    p_z_.resize(nz_);
    p_d_z_.resize(nd_, nz_);
    p_w_z_.resize(nw_, nz_);
    p_z_dw_.Resize(nd_, nw_, nz_);

    return true;
  }
  bool Train() {
    InitProb();
    for (std::size_t t = 0; t < niters_; ++t) {
      Mstep();
      Estep();
    }
    return true;
  }
  bool SaveTopics(const std::string& path) {
    return true;
  }
  bool SaveModel(const std::string& path) {
    return true;
  }
private:
  const Dataset* dataset_;
  std::size_t niters_;

  std::size_t nd_;  // number of documents
  std::size_t nw_;  // size of vocabulary
  std::size_t nz_;  // number of topics
  std::size_t nr_;  // number of total word occurs

  std::vector<double> p_z_;     // p(z)
  matrix<double> p_d_z_;        // p(d|z)
  matrix<double> p_w_z_;        // p(w|z)
  ZDWMatrix<double> p_z_dw_;    // p(z|d,w)

  std::size_t TotalWordOccurs(const Dataset& dataset) {
    std::size_t cnt = 0;
    for (uint32_t d = 0; d < dataset.DocSize(); ++d) {
      const Document& doc = dataset.Doc(d);
      for (uint32_t p = 0; p < doc.Size(); ++p) {
        cnt += doc.Freq(p);
      }
    }
    return cnt;
  }
  void InitProb() {

  }
  void Mstep();
  void Estep();
  double LogLikelihood() {
    return 0.0;
  }
};

} /* namespace toyml */
#endif /* PLSA_H_ */
