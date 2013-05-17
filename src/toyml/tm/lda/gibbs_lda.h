/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-11-22
 */

#ifndef GIBBS_LDA_H_
#define GIBBS_LDA_H_

#include <boost/numeric/ublas/matrix.hpp>

#include "lda.h"

namespace toyml {

namespace ublas = boost::numeric::ublas;

typedef std::size_t Size;

/**
 * @brief LDA using Gibbs Sampling
 */
class GibbsLDA {
public:
  virtual ~GibbsLDA();

  bool Init(const LDAOptions& options, const Dataset& dataset);
  std::size_t Train();
  std::string ToString() const;

  bool SaveModel(int no);
  bool SaveModel(const std::string& suffix = "");
  bool SaveTopics(const std::string& path) const;
private:
  LDAOptions options_;
  const Dataset* dataset_;

  std::size_t nd_;  // number of documents
  std::size_t nw_;  // size of vocabulary
  std::size_t nz_;  // number of topics

  double alpha_;
  double beta_;
  double kalpha_;
  double vbeta_;

  ublas::matrix<Size> c_dz_;  // c_dz_(d, z): count of words in document d assigned to topic z
  ublas::matrix<Size> c_zw_;  // c_zw_(z, w): count of word w assigned to topic z
  ublas::vector<Size> c_d_;   // c_d_(d): count of topics in document d
  ublas::vector<Size> c_z_;   // c_z_(d): count of words assigned to topic z

  std::vector<std::vector<Size> > z_;     // z_(d, w): the topic assigned to word w in document d
  ublas::vector<double> p_z_;

  ublas::matrix<double> theta_;   // document-topic distributions
  ublas::matrix<double> phi_;     // topic-word distributions

  std::size_t iter_;    // current iteration

  void Initialize();
  Size Sampling(Size d, Size w);
  void CalcThetaPhi();
  std::string Path(const std::string& fname, const std::string& suffix) const;
};

} /* namespace toyml */
#endif /* GIBBS_LDA_H_ */
