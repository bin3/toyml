/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-10-21
 */

#ifndef EX_PLSA_H_
#define EX_PLSA_H_

#include <cstddef>
#include <atomic>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/thread.hpp>
#include <glog/logging.h>

#include "utils.h"
#include "ex_dataset.h"
#include "plsa.h"

namespace toyml {
namespace ublas = boost::numeric::ublas;

/**
 * @brief Extended pLSA model options
 */
struct ExPLSAOptions {
  std::size_t niters;
  std::size_t ntopics;  // number of topics
  double lambda;       // weight of backgroud
  double ow;            // factor for w to prevent overfitting
  double ot;            // factor for t to prevent overfitting
  double oc;            // factor for c to prevent overfitting
  double eps;
  int log_interval;
  int save_interval;
  int em_log_interval;
  std::size_t threads;
  std::size_t topn;
  std::string datadir;
  std::string topic_path;
  std::string wtpath;
  std::string tcpath;
  std::string cupath;
  std::string finalsuffix;
  std::string seperator;
  bool random;
  ExPLSAOptions() :
      niters(100), ntopics(100), lambda(0.8), ow(0.1), ot(50), oc(0.1),
      eps(0.1), log_interval(10), save_interval(10),
      em_log_interval(1000), threads(4), topn(10),
      datadir("./"), topic_path("topics.dat"), wtpath("word-topic-prob.dat"),
      tcpath("topic-cel-prob.dat"), cupath("cel-user-prob.dat"),
      finalsuffix("final"), seperator("\t"), random(false) {
  }
  std::string ToString() const {
    std::stringstream ss;
    ss << NVC_(niters);
    ss << NVC_(ntopics);
    ss << NVC_(lambda);
    ss << NVC_(eps);
    ss << NVC_(log_interval);
    ss << NVC_(save_interval);
    ss << NVC_(threads);
    ss << NVC_(topn);
    ss << NVC_(random) << NV_(datadir);
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
    ss << NVC_(nu_) << NVC_(nc_) << NVC_(nt_) << NVC_(nw_);
    ss << NVC_(lambda_) << NVC_(ow_) << NVC_(ot_) << NV_(oc_);
    return ss.str();
  }
private:
  ExPLSAOptions opts_;
  const Dataset* ddata_;  // document dataset
  const Dataset* fdata_;  // followee dataset whose format is similar like document dataset

  double lambda_;   // weight of background
  double ow_;            // factor for w to prevent overfitting
  double ot_;            // factor for t to prevent overfitting
  double oc_;            // factor for c to prevent overfitting

  std::size_t nu_;  // number of users
  std::size_t nc_;  // number of celebrities
  std::size_t nt_;  // number of topics
  std::size_t nw_;  // size of vocabulary

  ublas::matrix<double> p_c_u_;          // p(c|u)
  ublas::matrix<double> p_t_c_;          // p(t|c)
  ublas::matrix<double> p_w_t_;          // p(w|t)
  ublas::vector<double> p_w_b_;           // p(p(w|B)

  ublas::matrix<double> p_c_u_new_;          // p(c|u)
  ublas::matrix<double> p_t_c_new_;          // p(t|c)
  ublas::matrix<double> p_w_t_new_;          // p(w|t)
  ublas::vector<double> unorm_;
  ublas::vector<double> cnorm_;
  ublas::vector<double> tnorm_;

  std::atomic<std::size_t> uid_;
  std::vector<ublas::matrix<double> > p_c_u_new_vec_;
  std::vector<ublas::matrix<double> > p_t_c_new_vec_;
  std::vector<ublas::matrix<double> > p_w_t_new_vec_;
  std::vector<ublas::matrix<double> > p_ct_vec_;
  std::vector<ublas::vector<double> > unorm_vec_;
  std::vector<ublas::vector<double> > cnorm_vec_;
  std::vector<ublas::vector<double> > tnorm_vec_;

  double LogLikelihood();
  void InitProb();
  void EMStep();
  void DoEM(std::size_t tid);

  std::string Path(const std::string& fname, const std::string& suffix) const;
  bool SaveModel(const std::string& path, const ublas::matrix<double>& mat,
      std::size_t size1, std::size_t size2) const;
};

} /* namespace toyml */
#endif /* EX_PLSA_H_ */
