/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-10-31
 */

#include "background_plsa.h"

namespace toyml {

BackgroundPLSA::BackgroundPLSA(): lambda_(0), delta_(1e-3) {
}

BackgroundPLSA::~BackgroundPLSA() {
}

bool BackgroundPLSA::Init(const BackgroundPLSAOptions& options, const Dataset& dataset) {
  if (options.lambda < 0 || options.lambda > 1.0) {
    LOG(ERROR) << "lambda=" << options.lambda << " which should be [0, 1]";
    return false;
  }
  boptions_ = options;
  lambda_ = options.lambda;

  bool ret = PLSA::Init(options, dataset);
  if (!ret) return false;

  delta_ = options.delta / (nd_ * nz_ * nw_);

  return true;
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
        p_dw += p_z_d_(z, d) * p_w_z_(w, z);
      }
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

  p_d_new_.clear();
  p_z_new_.clear();
  p_z_d_new_.clear();
  p_w_z_new_.clear();

  for (uint32_t d = 0; d < nd_; ++d) {
    const Document& doc = dataset_->Doc(d);
    for (uint32_t p = 0; p < doc.Size(); ++p) {
      uint32_t w = doc.Word(p);
      uint32_t n = doc.Freq(p);

      // Estep
      double norm = 0;
      for (uint32_t z = 0; z < nz_; ++z) {
        double p_zdw = p_z_d_(z, d) * p_w_z_(w, z);
        p_z_dw_(z) = p_zdw;
        norm += p_zdw;
      }
//      CHECK(norm > 0) << "Iter#" << iter_ << " norm=" << norm << ", d=" << d << ", w=" << w << ", SaveModel=" << SaveModel("debug");
      for (uint32_t z = 0; z < nz_; ++z) {
        p_z_dw_(z) /= norm;
      }
      double p_w_b = lambda_ * p_w_b_(w);
      double p_b_dw = p_w_b / (p_w_b + (1 - lambda_) * norm);
//      VLOG_EVERY_N(0, 1000) << "#" << google::COUNTER << " p_w_b=" << p_w_b << ", norm=" << norm << ", p_dwb=" << p_b_dw;

      // Mstep
      for (uint32_t z = 0; z < nz_; ++z) {
//        double np = n * p_z_dw_(z);
        double np = n * (1 - p_b_dw) * p_z_dw_(z) + delta_;
        p_w_z_new_(w, z) += np;
        p_z_d_new_(z, d) += np;
        p_z_new_(z) += np;
        p_d_new_(d) += np;
      }
    }
  }

  Normalize();
}

} /* namespace toyml */
