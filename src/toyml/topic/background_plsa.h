/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-10-31
 */

#ifndef BACKGROUND_PLSA_H_
#define BACKGROUND_PLSA_H_

#include "plsa.h"

namespace toyml {

struct BackgroundPLSAOptions: public PLSAOptions {
  double lambda;    // weight of background model
  double delta;     // added to loglikelyhood

  BackgroundPLSAOptions(): PLSAOptions::PLSAOptions(), lambda(0.8), delta(0.1) {}
  std::string ToString() const {
    std::stringstream ss;
    ss << NVC_(lambda) << NVC_(delta);
    ss << PLSAOptions::ToString();
    return ss.str();
  }
};

/**
 * @brief PLSA with a background model
 */
class BackgroundPLSA: public PLSA {
public:
  BackgroundPLSA();
  virtual ~BackgroundPLSA();

  bool Init(const BackgroundPLSAOptions& options, const Dataset& dataset);
  std::string ToString() const {
    std::stringstream ss;
    ss << NVC_(nd_) << NVC_(nz_) << NVC_(nw_);
    ss << NVC_(lambda_) << NV_(delta_);
    return ss.str();
  }
protected:
  BackgroundPLSAOptions boptions_;
  double lambda_;
  double delta_;
  ublas::vector<double> p_w_b_;           // p(w|B)

  double LogLikelihood();
  void InitProb();
  void EMStep();
};

} /* namespace toyml */
#endif /* BACKGROUND_PLSA_H_ */
