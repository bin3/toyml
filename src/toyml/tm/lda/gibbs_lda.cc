/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-11-22
 */

#include "gibbs_lda.h"
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <glog/logging.h>

namespace toyml {

GibbsLDA::~GibbsLDA() {
}

bool GibbsLDA::Init(const LDAOptions& options, const Dataset& dataset) {
  options_ = options;
  dataset_ = &dataset;
  Initialize();
  return true;
}

std::size_t GibbsLDA::Train() {
  SaveModel(0);
  VLOG(0) << "[begin]";
  for (iter_ = 1; iter_ <= options_.iters; ++iter_) {
    VLOG_EVERY_N(0, options_.nlog) << "Iteration#" << iter_;
    if (iter_ % options_.nsave == 0) {
      SaveModel(iter_);
    }
    for (std::size_t d = 0; d < nd_; ++d) {
      const Document& doc = dataset_->Doc(d);
      for (std::size_t wi = 0; wi < doc.Size(); ++wi) {
        Sampling(d, wi);
      }
    }
  }
  VLOG(0) << "[end]";
  SaveModel(options_.finalsuffix);
  return std::min(iter_ + 1, options_.iters);
}

void GibbsLDA::Initialize() {
  if (options_.random) {
    std::srand(std::time(NULL));
  } else {
    std::srand(0);
  }

  nd_ = dataset_->DocSize();
  nw_ = dataset_->DictSize();
  nz_ = options_.topics;

  alpha_ = options_.alpha / nz_;
  beta_ = options_.beta;
  kalpha_ = nz_ * alpha_;
  vbeta_ = nw_ * beta_;

  c_dz_ = ublas::matrix<Size>(nd_, nz_, 0);
  c_zw_ = ublas::matrix<Size>(nz_, nw_, 0);
  c_d_ = ublas::vector<Size>(nd_, 0);
  c_z_ = ublas::vector<Size>(nz_, 0);

  z_.resize(nd_);
  for (std::size_t d = 0; d < nd_; ++d) {
    const Document& doc = dataset_->Doc(d);
    z_[d].resize(doc.Size());
    for (std::size_t wi = 0; wi < doc.Size(); ++wi) {
      Size w = doc.Word(wi);
      Size z = static_cast<Size>((static_cast<double>(std::rand()) / RAND_MAX) * nz_);
      z_[d][wi] = z;
      ++c_dz_(d, z);
      ++c_d_(d);
      ++c_zw_(z, w);
      ++c_z_(z);
    }
  }

  p_z_.resize(nz_);

  theta_.resize(nd_, nz_);
  phi_.resize(nz_, nw_);
}

Size GibbsLDA::Sampling(Size d, Size wi) {
  Size w = dataset_->Doc(d).Word(wi);
  Size z = z_[d][wi];
  --c_dz_(d, z);
  --c_d_(d);
  --c_zw_(z, w);
  --c_z_(z);
  VLOG(4) << "old_z=" << z;
  for (z = 0; z < nz_; ++z) {
    p_z_(z) = (c_zw_(z, w) + beta_) / (c_z_(z) + vbeta_ ) *
      (c_dz_(d, z) + alpha_) / (c_d_(d) + kalpha_);
  }
  for (z = 1; z < nz_; ++z) {
    p_z_(z) += p_z_(z - 1);
  }
  double u = static_cast<double>(std::rand()) / RAND_MAX * p_z_(nz_ - 1);
  for (z = 0; z < nz_; ++z) {
    if (p_z_(z) >= u) {
      break;
    }
  }
  VLOG(4) << "new_z=" << z;
  z_[d][wi] = z;
  ++c_dz_(d, z);
  ++c_d_(d);
  ++c_zw_(z, w);
  ++c_z_(z);

  return 0;
}

std::string GibbsLDA::ToString() const {
  std::stringstream ss;
  ss << NVC_(nd_) << NVC_(nz_) << NVC_(nw_) << NVC_(alpha_) << NV_(beta_);
  return ss.str();
}

void GibbsLDA::CalcThetaPhi() {
  for (Size d = 0; d < nd_; ++d) {
    for (Size z = 0; z < nz_; ++z) {
      theta_(d, z) = (c_dz_(d, z) + alpha_) / (c_d_(d) + kalpha_);
    }
  }
  for (Size z = 0; z < nz_; ++z) {
    for (Size w = 0; w < nw_; ++w) {
      phi_(z, w) = (c_zw_(z, w) + beta_) / (c_z_(z) + vbeta_);
    }
  }
}

bool GibbsLDA::SaveModel(int no) {
  std::stringstream ss;
  ss << no;
  return SaveModel(ss.str());
}

bool GibbsLDA::SaveModel(const std::string& suffix) {
  VLOG(0) << "SaveModel suffix=" << suffix;
  CalcThetaPhi();
  bool ret = SaveTopics(Path(options_.zpath, suffix));
  ret &= Utils::SaveMatrix(phi_, Path(options_.zwpath, suffix));
  ret &= Utils::SaveMatrix(theta_, Path(options_.dzpath, suffix));
  return ret;
}

bool GibbsLDA::SaveTopics(const std::string& path) const {
  typedef std::pair<double, Size> ProbWord;

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
      vec.push_back(ProbWord(phi_(z, w), w));
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

std::string GibbsLDA::Path(const std::string& fname, const std::string& suffix) const {
  if (suffix.length() == 0) {
    return options_.datadir + "/" + fname;
  } else {
    return options_.datadir + "/" + fname + "." + suffix;
  }
}

} /* namespace toyml */
