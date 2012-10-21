/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-10-21
 */

#include "ex_plsa.h"

#include <iomanip>

namespace toyml {

ExPLSA::~ExPLSA() {
  // TODO Auto-generated destructor stub
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
  p_ct_uw_.Resize(nu_, nw_, nc_, nt_);

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
    Estep();
    Mstep();
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
  VLOG(4) << "LogLikelihood";
  double lik = 0;
  for (uint32_t u = 0; u < ddata_->DocSize(); ++u) {
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
      lik += n * log(p_w_u);
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

void ExPLSA::Estep() {
  VLOG(4) << "Estep";
  double norm = 0;
  for (uint32_t u = 0; u < ddata_->DocSize(); ++u) {
    const Document& doc = ddata_->Doc(u);
    const Document& fol = fdata_->Doc(u);
    for (uint32_t p = 0; p < doc.Size(); ++p) {
      uint32_t w = doc.Word(p);
      norm = 0;
      for (std::size_t fi = 0; fi < fol.Size(); ++fi) {
        uint32_t c = fol.Word(fi);
        for (uint32_t t = 0; t < nt_; ++t) {
          double p_wtc_u = p_w_t_(w, t) * p_t_c_(t, c) * p_c_u_(c, u);
          p_ct_uw_(u, w, c, t) = p_wtc_u;
          norm += p_wtc_u;
        }
      }
      for (std::size_t fi = 0; fi < fol.Size(); ++fi) {
        uint32_t c = fol.Word(fi);
        for (uint32_t t = 0; t < nt_; ++t) {
          p_ct_uw_(u, w, c, t) /= norm;
        }
      }
    }
  }
}

void ExPLSA::Mstep() {
  VLOG(4) << "Mstep";
  // p(w|t)
  double norm = 0;
  for (uint32_t t = 0; t < nt_; ++t) {
    norm = 0;
    for (uint32_t w = 0; w < nw_; ++w) {
      const PostingList& post = ddata_->Post(w);
      double p_w_t = 0;
      for (uint32_t ui = 0; ui < post.Size(); ++ui) {
        uint32_t u = post.Doc(ui);
        uint32_t n = post.Freq(ui);
        const Document& fol = fdata_->Doc(u);
        for (uint32_t fi = 0; fi < fol.Size(); ++fi) {
          uint32_t c = fol.Word(fi);
          p_w_t += n * p_ct_uw_(u, w, c, t);
        }
      }
      p_w_t_(w, t) = p_w_t;
      norm += p_w_t;
    }
    for (uint32_t w = 0; w < nw_; ++w) {
      p_w_t_(w, t) /= norm;
    }
  }

  // p(t|c)
  for (uint32_t c = 0; c < nc_; ++c) {
    norm = 0;
    const PostingList& post = fdata_->Post(c);
    for (uint32_t t = 0; t < nt_; ++t) {
      double p_t_c = 0;
      for (uint32_t ui = 0; ui < post.Size(); ++ui) {
        uint32_t u = post.Doc(ui);
        const Document& doc = ddata_->Doc(u);
        for (uint32_t wi = 0; wi < doc.Size(); ++wi) {
          uint32_t w = doc.Word(wi);
          uint32_t n = doc.Freq(wi);
          p_t_c += n * p_ct_uw_(u, w, c, t);
        }
      }
      p_t_c_(t, c) = p_t_c;
      norm += p_t_c;
    }
    for (uint32_t t = 0; t < nt_; ++t) {
      p_t_c_(t, c) /= norm;
    }
  }

  // p(c|u)
  for (uint32_t u = 0; u < nu_; ++u) {
    norm = 0;
    const Document& fol = fdata_->Doc(u);
    for (uint32_t fi = 0; fi < fol.Size(); ++fi) {
      uint32_t c = fol.Word(fi);
      double p_c_u = 0;
      const Document& doc = ddata_->Doc(u);
      for (uint32_t wi = 0; wi < doc.Size(); ++wi) {
        uint32_t w = doc.Word(wi);
        uint32_t n = doc.Freq(wi);
        for (uint32_t t = 0; t < nt_; ++t) {
          p_c_u += n * p_ct_uw_(u, w, c, t);
        }
      }
      p_c_u_(c, u) = p_c_u;
      norm += p_c_u;
    }
    for (uint32_t fi = 0; fi < fol.Size(); ++fi) {
      uint32_t c = fol.Word(fi);
      p_c_u_(c, u) /= norm;
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

bool ExPLSA::SaveModel(const std::string& path, const matrix<double>& mat,
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
