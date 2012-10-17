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
  op_ = options;
  dataset_ = &dataset;

  nd_ = dataset.DocSize();
  nw_ = dataset.DictSize();
  nz_ = op_.ntopics;
  nr_ = dataset.TotalWordOccurs();

  p_z_.resize(nz_);
  p_d_z_.resize(nd_, nz_);
  p_w_z_.resize(nw_, nz_);
  p_z_dw_.Resize(nd_, nw_, nz_);

  return true;
}

bool PLSA::Train() {
  InitProb();
  SaveModel(0); // debug
  double pre_lik = LogLikelihood();
  double cur_lik = 0;
  VLOG(0) << "[begin] L=" << pre_lik;
  std::size_t t = 0;
  for ( ; t < op_.niters; ++t) {
    LOG_EVERY_N(INFO, op_.log_interval) << "Iterator#" << t;
    Estep();
    Mstep();
    if ((t + 1) % op_.save_interval == 0) {
      SaveModel(t + 1);
    }
    cur_lik = LogLikelihood();
    double diff_lik = cur_lik - pre_lik;
    LOG_EVERY_N(INFO, op_.log_interval) << std::setprecision(10) << "L=" << cur_lik << ", diff=" << diff_lik;
    CHECK(diff_lik >= 0.0);
    if (diff_lik < op_.eps) {
      VLOG(0) << "[break] Iterator#" << t << " diff=" << diff_lik << ", eps=" << op_.eps;
      break;
    }
    pre_lik = cur_lik;
  }
  VLOG(0) << "[end] L=" << cur_lik;
  if ((t + 1) % op_.save_interval != 0) {
    SaveModel(t + 1);
  }
  return true;
}

bool PLSA::SaveModel(int no) const {
  VLOG(1) << "SaveModel no=" << no;
  bool ret = SaveTopics(Path(op_.topic_path, no));
  ret &= SaveTModel(Path(op_.tpath, no));
  ret &= SaveDZModel(Path(op_.dzpath, no));
  ret &= SaveWZModel(Path(op_.wzpath, no));
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
    for (std::size_t i = 0; i < vec.size() && i < op_.topn; ++i) {
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
    outf << p_z_[z] << "\n";
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
  outf << nd_ << op_.seperator << nz_ << "\n";
  for (std::size_t d = 0; d < nd_; ++d) {
    for (std::size_t z = 0; z < nz_; ++z) {
      outf << p_d_z_(d, z) << op_.seperator;
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
  outf << nw_ << op_.seperator << nz_ << "\n";
  for (std::size_t w = 0; w < nw_; ++w) {
    for (std::size_t z = 0; z < nz_; ++z) {
      outf << p_w_z_(w, z) << op_.seperator;
    }
    outf << "\n";
  }
  outf.close();
  VLOG(2) << "Saved word-topic model to " << path;
  return true;
}

double PLSA::LogLikelihood() {
  double lik = 0;
  for (uint32_t d = 0; d < dataset_->DocSize(); ++d) {
    const Document& doc = dataset_->Doc(d);
    for (uint32_t p = 0; p < doc.Size(); ++p) {
      uint32_t w = doc.Word(p);
      uint32_t n = doc.Freq(p);
      double p_dw = 0;
      for (uint32_t z = 0; z < nz_; ++z) {
        p_dw += p_z_[z] * p_d_z_(d, z) * p_w_z_(w, z);
      }
      VLOG(5) << "d=" << d << ", w=" << w << ", p_dw=" << p_dw;
      lik += n * log(p_dw);
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
    p_z_[z] = r;
    norm += r;
  }
  for (std::size_t z = 0; z < nz_; ++z) {
    p_z_[z] /= norm;
  }

  for (std::size_t d = 0; d < nd_; ++d) {
    norm = 0;
    for (std::size_t z = 0; z < nz_; ++z) {
      int r = std::rand() % kMod + 1;
      p_d_z_(d, z) = r;
      norm += r;
    }
    for (std::size_t z = 0; z < nz_; ++z) {
      p_d_z_(d, z) /= norm;
    }
  }

  for (std::size_t w = 0; w < nw_; ++w) {
    norm = 0;
    for (std::size_t z = 0; z < nz_; ++z) {
      int r = std::rand() % kMod + 1;
      p_w_z_(w, z) = r;
      norm += r;
    }
    for (std::size_t z = 0; z < nz_; ++z) {
      p_w_z_(w, z) /= norm;
    }
  }
}

void PLSA::Estep() {
  for (uint32_t d = 0; d < dataset_->DocSize(); ++d) {
    const Document& doc = dataset_->Doc(d);
    for (uint32_t p = 0; p < doc.Size(); ++p) {
      uint32_t w = doc.Word(p);
      double norm = 0;
      for (uint32_t z = 0; z < nz_; ++z) {
        double p_zdw = p_z_[z] * p_d_z_(d, z) * p_w_z_(w, z);
        p_z_dw_(d, w, z) = p_zdw;
        norm += p_zdw;
        VLOG(5) << "d=" << d << ", w=" << w << ", z=" << z << ", p_zdw=" << p_zdw << ", p_z_dw_(d, w, z)=" << p_z_dw_(d, w, z);
      }
      VLOG(4) << "d=" << d << ", w=" << w << ", norm=" << norm;
      for (uint32_t z = 0; z < nz_; ++z) {
        p_z_dw_(d, w, z) /= norm;
//        CHECK(p_z_dw_(d, w, z) > 0) << "d=" << d << ", w=" << w << ", z=" << z;
      }
    }
  }
}

void PLSA::Mstep() {
  double znorm = 0;
  for (uint32_t z = 0; z < nz_; ++z) {
    // p(d|z)
    double dnorm = 0;
    for (uint32_t d = 0; d < dataset_->DocSize(); ++d) {
      const Document& doc = dataset_->Doc(d);
      double p_d_z = 0;
      for (uint32_t p = 0; p < doc.Size(); ++p) {
        uint32_t w = doc.Word(p);
        uint32_t n = doc.Freq(p);
//        CHECK(n > 0) << "z=" << z << ", d=" << d << ", w=" << w;
//        CHECK(p_z_dw_(d, w, z) > 0) << "z=" << z << ", d=" << d << ", w=" << w;
        p_d_z += n * p_z_dw_(d, w, z);
      }
      VLOG(5) << "z=" << z << ", d=" << d << ", p_d_z=" << p_d_z;
      p_d_z_(d, z) = p_d_z;
      dnorm += p_d_z;
    }
    VLOG(4) << "z=" << z << ", dnorm=" << dnorm;
    for (uint32_t d = 0; d < dataset_->DocSize(); ++d) {
      p_d_z_(d, z) /= dnorm;
    }
    p_z_[z] = dnorm;
    znorm += dnorm;

    // p(w|z)
    double wnorm = 0;
    for (uint32_t w = 0; w < dataset_->DictSize(); ++w) {
      const PostingList& post = dataset_->Post(w);
      double p_w_z = 0;
      for (uint32_t p = 0; p < post.Size(); ++p) {
        uint32_t d = post.Doc(p);
        uint32_t n = post.Freq(p);
        p_w_z += n * p_z_dw_(d, w, z);
      }
      p_w_z_(w, z) = p_w_z;
      wnorm += p_w_z;
    }
    for (uint32_t w = 0; w < dataset_->DictSize(); ++w) {
      p_w_z_(w, z) /= wnorm;
    }
  }
  for (uint32_t z = 0; z < nz_; ++z) {
    p_z_[z] /= znorm;
  }
}

std::string PLSA::Path(const std::string& path, int no) const {
  if (no < 0) return op_.datadir + "/" + path;
  std::stringstream ss;
  ss << op_.datadir << "/" << path << "." << no;
  return ss.str();
}

} /* namespace toyml */

