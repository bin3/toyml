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

bool PLSA::Init(const PLSAOptions& options, const Dataset& dataset) {
  opts_ = options;
  dataset_ = &dataset;

  nd_ = dataset.DocSize();
  nw_ = dataset.DictSize();
  nz_ = opts_.ntopics;
  nr_ = dataset.TotalWordOccurs();

  p_z_.resize(nz_);
  p_d_z_.resize(nd_, nz_);
  p_w_z_.resize(nw_, nz_);

  p_z_new_.resize(nz_);
  p_d_z_new_.resize(nd_, nz_);
  p_w_z_new_.resize(nw_, nz_);
  p_z_dw_.resize(nz_);
  dnorm_.resize(nz_);
  wnorm_.resize(nz_);

  return true;
}

std::size_t PLSA::Train() {
  InitProb();
  double pre_lik = LogLikelihood();
  double cur_lik = 0;
  VLOG(0) << "[begin] L=" << std::setprecision(10) << pre_lik;
  std::size_t t = 0;
  for ( ; t < opts_.niters; ++t) {
    LOG_EVERY_N(INFO, opts_.log_interval) << "Iterator#" << t;
//    Estep();
//    Mstep();
    EMStep();
    if ((t + 1) % opts_.save_interval == 0) {
      SaveModel(t + 1);
    }
    cur_lik = LogLikelihood();
    double diff_lik = cur_lik - pre_lik;
    LOG_EVERY_N(INFO, opts_.log_interval) << std::setprecision(10) << "L=" << cur_lik << ", diff=" << diff_lik;
    CHECK(diff_lik >= 0.0);
    if (diff_lik < opts_.eps) {
      VLOG(0) << "[break] Iterator#" << t << " diff=" << diff_lik << ", eps=" << opts_.eps;
      break;
    }
    pre_lik = cur_lik;
  }
  VLOG(0) << "[end] L=" << std::setprecision(10) << cur_lik;
  SaveModel(opts_.finalsuffix);
  return std::min(t + 1, opts_.niters);
}

bool PLSA::SaveModel(int no) const {
  std::stringstream ss;
  ss << no;
  return SaveModel(ss.str());
}

bool PLSA::SaveModel(const std::string& suffix) const {
  VLOG(1) << "SaveModel suffix=" << suffix;
  bool ret = SaveTopics(Path(opts_.topic_path, suffix));
  ret &= SaveTModel(Path(opts_.tpath, suffix));
  ret &= SaveDZModel(Path(opts_.dzpath, suffix));
  ret &= SaveWZModel(Path(opts_.wzpath, suffix));
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
    for (std::size_t i = 0; i < vec.size() && i < opts_.topn; ++i) {
      outf << "\t" << dataset_->Word(vec[i].second) << "\t" << vec[i].first << "\n";
    }
  }

  outf.close();
  VLOG(2) << "Saved topics to " << path;
  return true;
}

bool PLSA::SaveTModel(const std::string& path) const {
  std::ofstream outf(path.c_str());
  if (!outf) {
    LOG(ERROR) << "Failed to save topic model to " << path;
    return false;
  }
  outf << nz_ << "\n";
  for (std::size_t z = 0; z < nz_; ++z) {
    outf << p_z_(z) << "\n";
  }
  outf.close();
  VLOG(2) << "Saved topic model to " << path;
  return true;
}

bool PLSA::SaveDZModel(const std::string& path) const {
  std::ofstream outf(path.c_str());
  if (!outf) {
    LOG(ERROR) << "Failed to save doc-topic model to " << path;
    return false;
  }
  outf << nd_ << opts_.seperator << nz_ << "\n";
  for (std::size_t d = 0; d < nd_; ++d) {
    for (std::size_t z = 0; z < nz_; ++z) {
      outf << p_d_z_(d, z) << opts_.seperator;
    }
    outf << "\n";
  }
  outf.close();
  VLOG(2) << "Saved doc-topic model to " << path;
  return true;
}

bool PLSA::SaveWZModel(const std::string& path) const {
  std::ofstream outf(path.c_str());
  if (!outf) {
    LOG(ERROR) << "Failed to save word-topic model to " << path;
    return false;
  }
  outf << nw_ << opts_.seperator << nz_ << "\n";
  for (std::size_t w = 0; w < nw_; ++w) {
    for (std::size_t z = 0; z < nz_; ++z) {
      outf << p_w_z_(w, z) << opts_.seperator;
    }
    outf << "\n";
  }
  outf.close();
  VLOG(2) << "Saved word-topic model to " << path;
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
        p_dw += p_z_(z) * p_d_z_(d, z) * p_w_z_(w, z);
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
  static int kMod = 10000;
  std::srand(std::time(NULL));

  double norm = 0;
  for (std::size_t z = 0; z < nz_; ++z) {
    int r = std::rand() % kMod + 1;
    p_z_(z) = r;
    norm += r;
  }
  for (std::size_t z = 0; z < nz_; ++z) {
    p_z_(z) /= norm;
  }

  for (std::size_t z = 0; z < nz_; ++z) {
    norm = 0;
    for (std::size_t d = 0; d < nd_; ++d) {
      int r = std::rand() % kMod + 1;
      p_d_z_(d, z) = r;
      norm += r;
    }
    for (std::size_t d = 0; d < nd_; ++d) {
      p_d_z_(d, z) /= norm;
    }
  }

  for (std::size_t z = 0; z < nz_; ++z) {
    norm = 0;
    for (std::size_t w = 0; w < nw_; ++w) {
      int r = std::rand() % kMod + 1;
      p_w_z_(w, z) = r;
      norm += r;
    }
    for (std::size_t w = 0; w < nw_; ++w) {
      p_w_z_(w, z) /= norm;
    }
  }
}

void PLSA::EMStep() {
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
      // Mstep
      for (uint32_t z = 0; z < nz_; ++z) {
        double np = n * p_z_dw_(z);
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
      p_d_z_(d, z) = p_d_z_new_(d, z) / dnorm_(z);
    }
    for (uint32_t w = 0; w < nw_; ++w) {
      p_w_z_(w, z) = p_w_z_new_(w, z) / wnorm_(z);
    }
    p_z_(z) = p_z_new_(z) / znorm;
  }
}

std::string PLSA::Path(const std::string& fname, const std::string& suffix) const {
  if (suffix.length() == 0) {
    return opts_.datadir + "/" + fname;
  } else {
    return opts_.datadir + "/" + fname + "." + suffix;
  }
}

} /* namespace toyml */

