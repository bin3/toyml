/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-10-21
 */

#include "ex_plsa.h"

#include <omp.h>
#include <iomanip>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

namespace toyml {

static double kZeroEps = 1e-10;

ExPLSA::~ExPLSA() {
}

bool ExPLSA::Init(const ExPLSAOptions& options, const DocumentSet& document_data,
    const DocumentSet& followee_data) {
  if (document_data.DocSize() != followee_data.DocSize()) {
    LOG(ERROR) << "The number of documents and users are not equal. docs="
        << document_data.DocSize() << ", users=" << followee_data.DocSize();
    return false;
  }
  opts_ = options;
  ddata_ = &document_data;
  fdata_ = &followee_data;

  lambda_ = opts_.lambda;
  nu_ = fdata_->DocSize();
  nc_ = fdata_->DictSize();
  nt_ = opts_.ntopics;
  nw_ = ddata_->DictSize();

//  ow_ = opts_.ow;
//  ot_ = opts_.ot / nt_;
//  oc_ = opts_.oc;
  ow_ = ot_ = oc_ = opts_.ow / (nu_ * nc_ * nt_ * nw_);

  p_superc_u_ = 1.0 / nc_;
  p_t_superc_ = 1.0 / nt_;
  p_t_superc_u_ = p_superc_u_ * p_t_superc_;

//  p_c_u_.resize(nc_, nu_);
  p_c_u_ = ublas::matrix<double>(nc_, nu_, 0);  // filled value of double is not 0.0, so we should set it explicitly.
  p_t_c_.resize(nt_, nc_);
  p_w_t_.resize(nw_, nt_);
  p_w_b_.resize(nw_);

  p_c_u_new_.resize(nc_, nu_);
  p_t_c_new_.resize(nt_, nc_);
  p_w_t_new_.resize(nw_, nt_);

  unorm_.resize(nu_);
  cnorm_.resize(nc_);
  tnorm_.resize(nt_);

  p_c_u_new_vec_.resize(opts_.threads, ublas::matrix<double>(nc_, nu_, 0));
  p_t_c_new_vec_.resize(opts_.threads, ublas::matrix<double>(nt_, nc_));
  p_w_t_new_vec_.resize(opts_.threads, ublas::matrix<double>(nw_, nt_));
  p_ct_vec_.resize(opts_.threads, ublas::matrix<double>(nc_, nt_));
  unorm_vec_.resize(opts_.threads, ublas::vector<double>(nu_));
  cnorm_vec_.resize(opts_.threads, ublas::vector<double>(nc_));
  tnorm_vec_.resize(opts_.threads, ublas::vector<double>(nt_));

  return true;
}

std::size_t ExPLSA::Train() {
  InitProb();
  double pre_lik = LogLikelihood();
  double cur_lik = 0;
  VLOG(0) << "[begin] L=" << std::setprecision(10) << pre_lik;
  for (iter_ = 1 ; iter_ <= opts_.niters; ++iter_) {
    LOG_EVERY_N(INFO, opts_.log_interval) << "Iteration#" << iter_;
    EMStep();
    if (iter_ % opts_.save_interval == 0) {
      SaveModel(iter_);
    }
    cur_lik = LogLikelihood();
    double diff_lik = cur_lik - pre_lik;
    LOG_EVERY_N(INFO, opts_.log_interval) << std::setprecision(10) << "L=" << cur_lik << ", diff=" << diff_lik;
    CHECK(diff_lik >= 0.0);
    if (diff_lik < opts_.eps) {
      VLOG(0) << "[break] Iterator#" << iter_ << " diff=" << diff_lik << ", eps=" << opts_.eps;
      break;
    }
    pre_lik = cur_lik;
  }
  VLOG(0) << "[end] L=" << std::setprecision(10) << cur_lik;
  SaveModel(opts_.finalsuffix);
  return std::min(iter_, opts_.niters);
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
  typedef std::pair<double, uint32_t> ProbId;

  std::ofstream outf(path.c_str());
  if (!outf) {
    LOG(ERROR) << "Failed to save topics to " << path;
    return false;
  }

  std::vector<ProbId> vec;
  vec.reserve(nw_);
  for (std::size_t t = 0; t < nt_; ++t) {
    outf << "Topic #" << t << ":\n";

    vec.clear();
    for (std::size_t w = 0; w < nw_; ++w) {
      vec.push_back(ProbId(p_w_t_(w, t), w));
    }
    std::sort(vec.begin(), vec.end(), std::greater<ProbId>());
    outf << "  Top " << opts_.topn << " words:\n";
    for (std::size_t i = 0; i < vec.size() && i < opts_.topn; ++i) {
      outf << "\t" << ddata_->Word(vec[i].second) << "\t" << vec[i].first << "\n";
    }

    vec.clear();
    for (std::size_t c = 0; c < nc_; ++c) {
      double p_c = 0;
      for (std::size_t u = 0; u < nu_; ++u) {
        p_c += p_c_u_(c, u) / nu_;
        VLOG_IF(0, p_c > 1) << "p_c=" << p_c << ", p_c_u_=" << p_c_u_(c, u) << ", c=" << c << ", u=" << u;
      }
      double p_tc = p_c * p_t_c_(t, c);
//      double p_tc = p_t_c_(t, c);
      VLOG_IF(0, p_tc > 1) << "[ERROR] p_tc=" << p_tc << ", p_c=" << p_c << ", p_t_c=" << p_t_c_(t, c);
      vec.push_back(ProbId(p_tc, c));
    }
    std::sort(vec.begin(), vec.end(), std::greater<ProbId>());
    outf << "  Top " << opts_.topn << " celebrities:\n";
    for (std::size_t i = 0; i < vec.size() && i < opts_.topn; ++i) {
      std::string twitter_id = fdata_->Word(vec[i].second);
      outf << "\t" << twitter_id << "\t" << vec[i].first << "\n";
      VLOG_IF(0, vec[i].first > 1) << "t=" << t << ", i=" << i << ", vec[i].first=" << vec[i].first;
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
    VLOG_IF(3, u % opts_.em_log_interval == 0) << "user#" << u;
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
      if (p_w_u > 0) {
//        lik += ((1 - p_zuw_(u, w)) * log(p_w_u * lambada_) + p_zuw_(u, w) * log(p_w_b_(w) * (1 - lambada_))) * n;
        lik += n * log(p_w_u * (1 - lambda_) + p_w_b_(w) * lambda_);
      }
    }
  }
  return lik;
}

void ExPLSA::InitProb() {
  VLOG(2) << "InitProb";
  static bool s_srand_done = false;
  if (!s_srand_done) {
    s_srand_done = true;
    if (opts_.random) {
      std::srand(std::time(NULL));
    } else {
      std::srand(0);
    }
  }
  static int kMod = 10000;

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
    if (opts_.super_celebrity) {
      norm += norm * p_superc_u_ / (1 - p_superc_u_);
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

  // p_w_b_
  ddata_->CalcWordProb(p_w_b_);
}

void ExPLSA::DoEM(std::size_t tid) {
  VLOG(3) << "DoEM thread#" << tid;

  p_c_u_new_vec_[tid].clear();
  p_t_c_new_vec_[tid].clear();
  p_w_t_new_vec_[tid].clear();
  CHECK(p_t_c_new_vec_[tid](0, 0) == 0) << "p_t_c_new_vec_[tid](0, 0)=" << p_t_c_new_vec_[tid](0, 0);

  unorm_vec_[tid].clear();
  cnorm_vec_[tid].clear();
  tnorm_vec_[tid].clear();
  CHECK(unorm_vec_[tid](0) == 0) << " unorm_vec_[tid](0)=" << unorm_vec_[tid](0);

  ublas::matrix<double> p_ct_(nc_, nt_);
  for (uint32_t u = 0; (u = uid_++) < nu_; ) {
    VLOG_IF(3, u % opts_.em_log_interval == 0) << "user#" << u;
    const Document& doc = ddata_->Doc(u);
    const Document& fol = fdata_->Doc(u);
    p_ct_.clear();
    for (uint32_t p = 0; p < doc.Size(); ++p) {
      uint32_t w = doc.Word(p);
      uint32_t n = doc.Freq(p);

      // Estep
      double norm = 0;
      for (std::size_t fi = 0; fi < fol.Size(); ++fi) {
        uint32_t c = fol.Word(fi);
        for (uint32_t t = 0; t < nt_; ++t) {
          double p_wtc_u = p_w_t_(w, t) * p_t_c_(t, c) * p_c_u_(c, u);
          p_ct_(c, t) = p_wtc_u;
          norm += p_wtc_u;
        }
      }
      if (opts_.super_celebrity) {
        for (uint32_t t = 0; t < nt_; ++t) {
          double p_wtc_u = p_w_t_(w, t) * p_t_superc_u_;
//          p_ct_(c, t) = p_wtc_u;
          norm += p_wtc_u;
        }
      }
      double p_w_b = lambda_ * p_w_b_(w);
      double p_uw_b = p_w_b / ((1 - lambda_) * norm + p_w_b);

      // Mstep
      for (std::size_t fi = 0; fi < fol.Size(); ++fi) {
        uint32_t c = fol.Word(fi);
        for (uint32_t t = 0; t < nt_; ++t) {
          double p_ct = p_ct_(c, t) / norm;
          double np = n * p_ct * (1 - p_uw_b);
          p_c_u_new_vec_[tid](c, u) += np + oc_;
          p_t_c_new_vec_[tid](t, c) += np + ot_;
          p_w_t_new_vec_[tid](w, t) += np + ow_;
          unorm_vec_[tid](u) += np + oc_;
          cnorm_vec_[tid](c) += np + ot_;
          tnorm_vec_[tid](t) += np + ow_;
        }
      }
      if (opts_.super_celebrity) {
        for (uint32_t t = 0; t < nt_; ++t) {
          double p_ct = p_w_t_(w, t) * p_t_superc_u_;
          p_ct = p_ct / norm;
          double np = n * p_ct * (1 - p_uw_b);
//          p_c_u_new_vec_[tid](c, u) += np + oc_;
//          p_t_c_new_vec_[tid](t, c) += np + ot_;
          p_w_t_new_vec_[tid](w, t) += np + ow_;
          unorm_vec_[tid](u) += np + oc_;
//          cnorm_vec_[tid](c) += np + ot_;
          tnorm_vec_[tid](t) += np + ow_;
        }
      }
    }
  }
}

void ExPLSA::EMStep() {
  VLOG(2) << "EMStep";

  // EM using multi-thread
  uid_ = 0;
  boost::thread_group threads;
  for (std::size_t tid = 0; tid < opts_.threads; ++tid) {
    threads.create_thread(boost::bind(&ExPLSA::DoEM, this, tid));
  }
  threads.join_all();

  // normalize
//#pragma omp parallel for
  for (uint32_t u = 0; u < nu_; ++u) {
    double norm_sum = 0;
    for (std::size_t tid = 0; tid < opts_.threads; ++tid) {
      norm_sum += unorm_vec_[tid](u);
    }
    const Document& fol = fdata_->Doc(u);
    for (std::size_t fi = 0; fi < fol.Size(); ++fi) {
      uint32_t c = fol.Word(fi);
      double sum = 0;
      for (std::size_t tid = 0; tid < opts_.threads; ++tid) {
        sum += p_c_u_new_vec_[tid](c, u);
      }
      if (norm_sum > kZeroEps) {
        p_c_u_(c, u) = sum / norm_sum;
      } else {
        p_c_u_(c, u) = 0;
      }
      CHECK(norm_sum > kZeroEps) << NVC_(iter_) << NVC_(u) << NVC_(c) << NVC_(sum) << NV_(norm_sum);
    }
  }

//#pragma omp parallel for
  for (uint32_t c = 0; c < nc_; ++c) {
    double norm_sum = 0;
    for (std::size_t tid = 0; tid < opts_.threads; ++tid) {
      norm_sum += cnorm_vec_[tid](c);
    }
    for (uint32_t t = 0; t < nt_; ++t) {
      double sum = 0;
      for (std::size_t tid = 0; tid < opts_.threads; ++tid) {
        sum += p_t_c_new_vec_[tid](t, c);
      }
      if (norm_sum > kZeroEps) {
        p_t_c_(t, c) = sum / norm_sum;
      } else {
        p_t_c_(t, c) = 0;
      }
      CHECK(norm_sum > kZeroEps) << NVC_(iter_) << NVC_(c) << NVC_(t) << NVC_(sum) << NV_(norm_sum);
    }
  }

//#pragma omp parallel for
  for (uint32_t t = 0; t < nt_; ++t) {
    double norm_sum = 0;
    for (std::size_t tid = 0; tid < opts_.threads; ++tid) {
      norm_sum += tnorm_vec_[tid](t);
    }
    for (uint32_t w = 0; w < nw_; ++w) {
      double sum = 0;
      for (std::size_t tid = 0; tid < opts_.threads; ++tid) {
        sum += p_w_t_new_vec_[tid](w, t);
      }
      if (norm_sum > kZeroEps) {
        p_w_t_(w, t) = sum / norm_sum;
      } else {
        p_w_t_(w, t) = 0;
      }
      CHECK(norm_sum > kZeroEps) << NVC_(iter_) << NVC_(t) << NVC_(w) << NVC_(sum) << NV_(norm_sum);
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
  outf << size2 << opts_.seperator << size1 << "\n";
  for (std::size_t col = 0; col < size2; ++col) {
    for (std::size_t row = 0; row < size1; ++row) {
      outf << mat(row, col) << opts_.seperator;
    }
    outf << "\n";
  }
  outf.close();
  VLOG(2) << "Saved model to " << path;
  return true;
}

} /* namespace toyml */
