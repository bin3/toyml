/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-10-21
 */

#include "ex_plsa.h"

#include <omp.h>
#include <iomanip>

namespace toyml {

ExPLSA::~ExPLSA() {
}

bool ExPLSA::Init(const ExPLSAOptions& options, const Dataset& document_data,
    const Dataset& followee_data) {
  if (document_data.DocSize() != followee_data.DocSize()) {
    LOG(ERROR) << "The number of documents and users are not equal. docs="
        << document_data.DocSize() << ", users=" << followee_data.DocSize();
    return false;
  }
  opts_ = options;
  ddata_ = &document_data;
  fdata_ = &followee_data;

  nu_ = fdata_->DocSize();
  nc_ = fdata_->DictSize();
  nt_ = opts_.ntopics;
  nw_ = ddata_->DictSize();

  p_c_u_.resize(nc_, nu_);
  p_t_c_.resize(nt_, nc_);
  p_w_t_.resize(nw_, nt_);

  p_c_u_new_.resize(nc_, nu_);
  p_t_c_new_.resize(nt_, nc_);
  p_w_t_new_.resize(nw_, nt_);
//  p_ct_.resize(nc_, nt_);

  unorm_.resize(nu_);
  cnorm_.resize(nc_);
  tnorm_.resize(nt_);

  return true;
}

std::size_t ExPLSA::Train() {
  InitProb();
  double pre_lik = LogLikelihood();
  double cur_lik = 0;
  VLOG(0) << "[begin] L=" << std::setprecision(10) << pre_lik;
  std::size_t t = 0;
  for ( ; t < opts_.niters; ++t) {
    LOG_EVERY_N(INFO, opts_.log_interval) << "Iterator#" << t;
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

bool ExPLSA::SaveModel(int no) const {
  std::stringstream ss;
  ss << no;
  return SaveModel(ss.str());
}

bool ExPLSA::SaveModel(const std::string& suffix) const {
  VLOG(1) << "SaveModel suffix=" << suffix;
  bool ret = SaveTopics(Path(opts_.topic_path, suffix));
  ret &= SaveWTModel(Path(opts_.wtpath, suffix));
  ret &= SaveTCModel(Path(opts_.tcpath, suffix));
  ret &= SaveCUModel(Path(opts_.cupath, suffix));
  return ret;
}

bool ExPLSA::SaveTopics(const std::string& path) const {
  typedef std::pair<double, uint32_t> ProbWord;

  std::ofstream outf(path.c_str());
  if (!outf) {
    LOG(ERROR) << "Failed to save topics to " << path;
    return false;
  }

  std::vector<ProbWord> vec;
  vec.reserve(nw_);
  for (std::size_t z = 0; z < nt_; ++z) {
    vec.clear();
    for (std::size_t w = 0; w < nw_; ++w) {
      vec.push_back(ProbWord(p_w_t_(w, z), w));
    }
    std::sort(vec.begin(), vec.end(), std::greater<ProbWord>());

    outf << "Topic #" << z << ":\n";
    for (std::size_t i = 0; i < vec.size() && i < opts_.topn; ++i) {
      outf << "\t" << ddata_->Word(vec[i].second) << "\t" << vec[i].first << "\n";
    }
  }

  outf.close();
  VLOG(2) << "Saved topics to " << path;
  return true;
}

bool ExPLSA::SaveWTModel(const std::string& path) const {
  return SaveModel(path, p_w_t_, nw_, nt_);
}

bool ExPLSA::SaveTCModel(const std::string& path) const {
  return SaveModel(path, p_t_c_, nt_, nc_);
}

bool ExPLSA::SaveCUModel(const std::string& path) const {
  return SaveModel(path, p_c_u_, nc_, nu_);
}

double ExPLSA::LogLikelihood() {
  VLOG(2) << "LogLikelihood";
  double lik = 0;
#pragma omp parallel for reduction(+: lik)
  for (uint32_t u = 0; u < nu_; ++u) {
    VLOG_EVERY_N(3, opts_.em_log_interval) << "user#" << google::COUNTER;
    const Document& doc = ddata_->Doc(u);
    const Document& fol = fdata_->Doc(u);
    for (uint32_t p = 0; p < doc.Size(); ++p) {
      uint32_t w = doc.Word(p);
      uint32_t n = doc.Freq(p);
      double p_w_u = 0;
      for (std::size_t fi = 0; fi < fol.Size(); ++fi) {
        uint32_t c = fol.Word(fi);
        for (uint32_t t = 0; t < nt_; ++t) {
          p_w_u += p_w_t_(w, t) * p_t_c_(t, c) * p_c_u_(c, u);
        }
      }
      VLOG(5) << "d=" << u << ", w=" << w << ", p_w_u=" << p_w_u;
      if (p_w_u > 0) {
        lik += n * log(p_w_u);
      }
    }
  }
  return lik;
}

void ExPLSA::InitProb() {
  VLOG(4) << "InitProb";
  static int kMod = 10000;
  std::srand(std::time(NULL));

  double norm = 0;
  for (std::size_t u = 0; u < nu_; ++u) {
    const Document& fol = fdata_->Doc(u);
    norm = 0;
    for (std::size_t i = 0; i < fol.Size(); ++i) {
      uint32_t c = fol.Word(i);
      int r = std::rand() % kMod + 1;
      p_c_u_(c, u) = r;
      norm += r;
    }
    for (std::size_t i = 0; i < fol.Size(); ++i) {
      uint32_t c = fol.Word(i);
      p_c_u_(c, u) /= norm;
    }
  }

  for (std::size_t c = 0; c < nc_; ++c) {
    norm = 0;
    for (std::size_t t = 0; t < nt_; ++t) {
      int r = std::rand() % kMod + 1;
      p_t_c_(t, c) = r;
      norm += r;
    }
    for (std::size_t t = 0; t < nt_; ++t) {
      p_t_c_(t, c) /= norm;
    }
  }

  for (std::size_t t = 0; t < nt_; ++t) {
    norm = 0;
    for (std::size_t w = 0; w < nw_; ++w) {
      int r = std::rand() % kMod + 1;
      p_w_t_(w, t) = r;
      norm += r;
    }
    for (std::size_t w = 0; w < nw_; ++w) {
      p_w_t_(w, t) /= norm;
    }
  }
}

void ExPLSA::EMStep() {
  VLOG(2) << "EMStep";

  p_c_u_new_.clear();
  p_t_c_new_.clear();
  p_w_t_new_.clear();

  unorm_.clear();
  cnorm_.clear();
  tnorm_.clear();

//  ublas::matrix<double> p_ct_(nc_, nt_, 0);
//#pragma omp parallel for private(p_ct_)
#pragma omp parallel for
  for (uint32_t u = 0; u < nu_; ++u) {
    VLOG_EVERY_N(3, opts_.em_log_interval) << "user#" << google::COUNTER;
    ublas::matrix<double> p_ct_(nc_, nt_, 0);
    const Document& doc = ddata_->Doc(u);
    const Document& fol = fdata_->Doc(u);
    for (uint32_t p = 0; p < doc.Size(); ++p) {
      uint32_t w = doc.Word(p);
      uint32_t n = doc.Freq(p);

      // Estep
      double norm = 0;
//#pragma omp parallel for reduction(+: norm)
      for (std::size_t fi = 0; fi < fol.Size(); ++fi) {
        uint32_t c = fol.Word(fi);
        for (uint32_t t = 0; t < nt_; ++t) {
          double p_wtc_u = p_w_t_(w, t) * p_t_c_(t, c) * p_c_u_(c, u);
          p_ct_(c, t) = p_wtc_u;
          norm += p_wtc_u;
        }
      }
      // Mstep
//#pragma omp parallel for
      for (std::size_t fi = 0; fi < fol.Size(); ++fi) {
        uint32_t c = fol.Word(fi);
        for (uint32_t t = 0; t < nt_; ++t) {
          p_ct_(c, t) /= norm;
          double np = n * p_ct_(c, t);
//#pragma omp critical
          // TODO
          {
          p_c_u_new_(c, u) += np;
          p_t_c_new_(t, c) += np;
          p_w_t_new_(w, t) += np;
          unorm_(u) += np;
          cnorm_(c) += np;
          tnorm_(t) += np;
          }
        }
      }
    }
  }

  // normalize
//#pragma omp parallel for
  for (uint32_t u = 0; u < nu_; ++u) {
    const Document& fol = fdata_->Doc(u);
    for (std::size_t fi = 0; fi < fol.Size(); ++fi) {
      uint32_t c = fol.Word(fi);
      p_c_u_(c, u) = p_c_u_new_(c, u) / unorm_(u);
    }
  }

//#pragma omp parallel for
  for (uint32_t c = 0; c < nc_; ++c) {
    for (uint32_t t = 0; t < nt_; ++t) {
      p_t_c_(t, c) = p_t_c_new_(t, c) / cnorm_(c);
    }
  }

//#pragma omp parallel for
  for (uint32_t t = 0; t < nt_; ++t) {
    for (uint32_t w = 0; w < nw_; ++w) {
      p_w_t_(w, t) = p_w_t_new_(w, t) / tnorm_(t);
    }
  }
}

std::string ExPLSA::Path(const std::string& fname,
    const std::string& suffix) const {
  if (suffix.length() == 0) {
    return opts_.datadir + "/" + fname;
  } else {
    return opts_.datadir + "/" + fname + "." + suffix;
  }
}

bool ExPLSA::SaveModel(const std::string& path, const ublas::matrix<double>& mat,
    std::size_t size1, std::size_t size2) const {
  std::ofstream outf(path.c_str());
  if (!outf) {
    LOG(ERROR) << "Failed to save model to " << path;
    return false;
  }
  outf << size1 << opts_.seperator << size2 << "\n";
  for (std::size_t i = 0; i < size1; ++i) {
    for (std::size_t j = 0; j < size2; ++j) {
      outf << mat(i, j) << opts_.seperator;
    }
    outf << "\n";
  }
  outf.close();
  VLOG(2) << "Saved model to " << path;
  return true;
}

} /* namespace toyml */
