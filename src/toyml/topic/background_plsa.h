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

  BackgroundPLSAOptions(): PLSAOptions::PLSAOptions(), lambda(0) {}
  std::string ToString() const {
    std::stringstream ss;
    ss << NAME_VAL(lambda) << ", ";
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
protected:
  BackgroundPLSAOptions boptions_;
  double lambda_;
  ublas::vector<double> p_w_b_;           // p(p(w|B)

  double LogLikelihood();
  void InitProb();
  void EMStep();
};

} /* namespace toyml */
#endif /* BACKGROUND_PLSA_H_ */
