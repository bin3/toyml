/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-10-31
 */

#include "background_plsa.h"

namespace toyml {

BackgroundPLSA::BackgroundPLSA(): lambda_(0) {
}

BackgroundPLSA::~BackgroundPLSA() {
}

bool BackgroundPLSA::Init(const BackgroundPLSAOptions& options, const Dataset& dataset) {
  boptions_ = options;
  lambda_ = options.lambda;
  return PLSA::Init(options, dataset);
}

double BackgroundPLSA::LogLikelihood() {
  VLOG(2) << "LogLikelihood";
  double lik = 0;
  for (uint32_t d = 0; d < nd_; ++d) {
    const Document& doc = dataset_->Doc(d);
    for (uint32_t p = 0; p < doc.Size(); ++p) {
      uint32_t w = doc.Word(p);
      uint32_t n = doc.Freq(p);
      double p_dw = 0;
      for (uint32_t z = 0; z < nz_; ++z) {
        p_dw += p_z_(z) * p_d_z_(d, z) * p_w_z_(w, z);
      }
      VLOG(5) << "d=" << d << ", w=" << w << ", p_dw=" << p_dw;
      if (p_dw > 0) {
//        lik += n * log(p_dw);
        lik += n * log((1 - lambda_) * p_dw + lambda_ * p_w_b_(w));
      }
    }
  }
  return lik;
}

void BackgroundPLSA::InitProb() {
  PLSA::InitProb();
  dataset_->CalcWordProb(p_w_b_);
}

void BackgroundPLSA::EMStep() {
  VLOG(2) << "EMStep";

  p_z_new_.clear();
  p_d_z_new_.clear();
  p_w_z_new_.clear();

  p_z_dw_.clear();
  dnorm_.clear();
  wnorm_.clear();

  double znorm = 0;
  for (uint32_t d = 0; d < nd_; ++d) {
    const Document& doc = dataset_->Doc(d);
    for (uint32_t p = 0; p < doc.Size(); ++p) {
      uint32_t w = doc.Word(p);
      uint32_t n = doc.Freq(p);
      // Estep
      double norm = 0;
      for (uint32_t z = 0; z < nz_; ++z) {
        double p_zdw = p_z_(z) * p_d_z_(d, z) * p_w_z_(w, z);
        p_z_dw_(z) = p_zdw;
        norm += p_zdw;
      }
      for (uint32_t z = 0; z < nz_; ++z) {
        p_z_dw_(z) /= norm;
      }
      double tmp = lambda_ * p_w_b_(w);
      double p_dwb = tmp / (tmp + (1 - lambda_) * norm);
      // Mstep
      for (uint32_t z = 0; z < nz_; ++z) {
//        double np = n * p_z_dw_(z);
        double np = n * (1 - p_dwb) * p_z_dw_(z);
        p_d_z_new_(d, z) += np;
        p_w_z_new_(w, z) += np;
        p_z_new_(z) += np;
        dnorm_(z) += np;
        wnorm_(z) += np;
        znorm += np;
      }
    }
  }
  // normalize
  for (uint32_t z = 0; z < nz_; ++z) {
    for (uint32_t d = 0; d < nd_; ++d) {
      if (dnorm_(z) > 0) {
        p_d_z_(d, z) = p_d_z_new_(d, z) / dnorm_(z);
      } else {
        p_d_z_(d, z) = 0;
      }
    }
    for (uint32_t w = 0; w < nw_; ++w) {
      if (wnorm_(z) > 0) {
        p_w_z_(w, z) = p_w_z_new_(w, z) / wnorm_(z);
      } else {
        p_w_z_(w, z) = 0;
      }
    }
    if (znorm > 0) {
      p_z_(z) = p_z_new_(z) / znorm;
    } else {
      p_z_(z) = 0;
    }
  }
}

} /* namespace toyml */
