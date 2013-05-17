/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-10-14
 */

#include "plsa.h"

#include <ctime>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <functional>

namespace toyml {

PLSA::~PLSA() {
}

bool PLSA::Init(const PLSAOptions& options, const DocumentSet& dataset) {
  options_ = options;
  dataset_ = &dataset;

  nd_ = dataset.DocSize();
  nw_ = dataset.DictSize();
  nz_ = options_.ntopics;

  p_w_z_.resize(nw_, nz_);
  p_z_d_.resize(nz_, nd_);
  p_z_dw_.resize(nz_);

  p_z_new_.resize(nz_);
  p_d_new_.resize(nd_);
  p_w_z_new_.resize(nw_, nz_);
  p_z_d_new_.resize(nz_, nd_);

  return true;
}

std::size_t PLSA::Train() {
  InitProb();
  double pre_lik = LogLikelihood();
  double cur_lik = 0;
  VLOG(0) << "[begin] L=" << std::setprecision(10) << pre_lik;
  for (iter_ = 0 ; iter_ < options_.niters; ++iter_) {
    LOG_EVERY_N(INFO, options_.log_interval) << "Iteration#" << iter_;
    EMStep();
    if ((iter_ + 1) % options_.save_interval == 0) {
      SaveModel(iter_ + 1);
    }
    cur_lik = LogLikelihood();
    double diff_lik = cur_lik - pre_lik;
    LOG_EVERY_N(INFO, options_.log_interval) << std::setprecision(10) << "L=" << cur_lik << ", diff=" << diff_lik;
    CHECK(diff_lik >= 0.0);
    if (diff_lik < options_.eps) {
      VLOG(0) << "[break] Iteration#" << iter_ << " diff=" << diff_lik << ", eps=" << options_.eps;
      break;
    }
    pre_lik = cur_lik;
  }
  VLOG(0) << "[end] L=" << std::setprecision(10) << cur_lik;
  SaveModel(options_.finalsuffix);
  return std::min(iter_ + 1, options_.niters);
}

bool PLSA::SaveModel(int no) const {
  std::stringstream ss;
  ss << no;
  return SaveModel(ss.str());
}

bool PLSA::SaveModel(const std::string& suffix) const {
  VLOG(1) << "SaveModel suffix=" << suffix;
  bool ret = SaveTopics(Path(options_.topic_path, suffix));
  ret &= SaveMatrix(p_z_d_, Path(options_.zdpath, suffix));
  ret &= SaveMatrix(p_w_z_, Path(options_.wzpath, suffix));
  return ret;
}

bool PLSA::SaveTopics(const std::string& path) const {
  typedef std::pair<double, uint32_t> ProbWord;

  std::ofstream outf(path.c_str());
  if (!outf) {
    LOG(ERROR) << "Failed to save topics to " << path;
    return false;
  }

  std::vector<ProbWord> vec;
  vec.reserve(nw_);
  for (std::size_t z = 0; z < nz_; ++z) {
    vec.clear();
    for (std::size_t w = 0; w < nw_; ++w) {
      vec.push_back(ProbWord(p_w_z_(w, z), w));
    }
    std::sort(vec.begin(), vec.end(), std::greater<ProbWord>());

    outf << "Topic #" << z << ":\n";
    for (std::size_t i = 0; i < vec.size() && i < options_.topn; ++i) {
      outf << "\t" << dataset_->Word(vec[i].second) << "\t" << vec[i].first << "\n";
    }
  }

  outf.close();
  VLOG(2) << "Saved topics to " << path;
  return true;
}

double PLSA::LogLikelihood() {
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
      VLOG(5) << "d=" << d << ", w=" << w << ", p_dw=" << p_dw;
      if (p_dw > 0) {
        lik += n * log(p_dw);
      }
    }
  }
  return lik;
}

void PLSA::InitProb() {
  RandomizeMatrix(p_z_d_);
  RandomizeMatrix(p_w_z_);
}

void PLSA::EMStep() {
  VLOG(2) << "EMStep";

  p_d_new_.clear();
  p_z_new_.clear();
  p_w_z_new_.clear();
  p_z_d_new_.clear();

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
      for (uint32_t z = 0; z < nz_; ++z) {
        p_z_dw_(z) /= norm;
      }
      // Mstep
      for (uint32_t z = 0; z < nz_; ++z) {
        double np = n * p_z_dw_(z);
        p_w_z_new_(w, z) += np;
        p_z_d_new_(z, d) += np;
        p_z_new_(z) += np;
        p_d_new_(d) += np;
      }
    }
  }

  Normalize();
}

void PLSA::Normalize() {
  for (uint32_t z = 0; z < nz_; ++z) {
    for (uint32_t w = 0; w < nw_; ++w) {
      if (p_z_new_(z) > 0) {
        p_w_z_(w, z) = p_w_z_new_(w, z) / p_z_new_(z);
      } else {
        p_w_z_(w, z) = 0;
      }
//      CHECK(p_w_z_(w, z) > 0) << "Iter#" << iter_ << " " << NVC_(z) << NVC_(w) << NVC_(p_w_z_new_(w, z)) << NV_(p_z_new_(z));
    }
  }

  for (uint32_t d = 0; d < nd_; ++d) {
    for (uint32_t z = 0; z < nz_; ++z) {
      if (p_d_new_(d) > 0) {
        p_z_d_(z, d) = p_z_d_new_(z, d) / p_d_new_(d);
      } else {
        p_z_d_(z, d) = 0;
      }
    }
  }
}

void PLSA::RandomizeMatrix(ublas::matrix<double>& mat) {
  static int kMod = 10000;
  static bool s_srand_done = false;
  if (!s_srand_done) {
    s_srand_done = true;
    if (options_.random) {
      std::srand(std::time(NULL));
    } else {
      std::srand(0);
    }
  }

  for (std::size_t x = 0; x < mat.size2(); ++x) {
    double norm = 0;
    for (std::size_t y = 0; y < mat.size1(); ++y) {
      int r = std::rand() % kMod + 1;
      mat(y, x) = r;
      norm += r;
    }
    for (std::size_t y = 0; y < mat.size1(); ++y) {
      mat(y, x) /= norm;
    }
  }
}

bool PLSA::SaveMatrix(const ublas::matrix<double>& mat,
    const std::string& path) const {
  std::ofstream outf(path.c_str());
  if (!outf) {
    LOG(ERROR) << "Failed to save matrix to " << path;
    return false;
  }
  outf << mat.size2() << options_.seperator << mat.size1() << "\n";
  for (std::size_t x = 0; x < mat.size2(); ++x) {
    for (std::size_t y = 0; y < mat.size1(); ++y) {
      outf << mat(y, x) << options_.seperator;
    }
    outf << "\n";
  }
  outf.close();
  VLOG(2) << "Have saved matrix to " << path;
  return true;
}

std::string PLSA::Path(const std::string& fname, const std::string& suffix) const {
  if (suffix.length() == 0) {
    return options_.datadir + "/" + fname;
  } else {
    return options_.datadir + "/" + fname + "." + suffix;
  }
}

} /* namespace toyml */

