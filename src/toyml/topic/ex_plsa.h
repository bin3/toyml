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
  double lambda;       // for backgroud
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
  ExPLSAOptions() :
      niters(100), ntopics(100), lambda(0.2), eps(1e-3), log_interval(10), save_interval(10),
      em_log_interval(1000), threads(4), topn(10),
      datadir("./"), topic_path("topics.dat"), wtpath("word-topic-prob.dat"),
      tcpath("topic-cel-prob.dat"), cupath("cel-user-prob.dat"),
      finalsuffix("final"), seperator("\t") {
  }
  std::string ToString() const {
    std::stringstream ss;
    ss << NAME_VAL_COMMA(niters);
    ss << NAME_VAL_COMMA(ntopics);
    ss << NAME_VAL_COMMA(lambda);
    ss << NAME_VAL_COMMA(eps);
    ss << NAME_VAL_COMMA(log_interval);
    ss << NAME_VAL_COMMA(save_interval);
    ss << NAME_VAL_COMMA(threads);
    ss << NAME_VAL_COMMA(topn);
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

  double lambda_;   // for background
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
