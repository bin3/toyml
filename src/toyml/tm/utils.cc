/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-10-21
 */

#include "utils.h"
#include <fstream>
#include <glog/logging.h>

namespace toyml {

Utils::Utils() {
}

Utils::~Utils() {
}

bool Utils::SaveMatrix(const ublas::matrix<double>& mat,
    const std::string& path) {
  std::ofstream outf(path.c_str());
  if (!outf) {
    LOG(ERROR)<< "Failed to save matrix to " << path;
    return false;
  }
  outf << mat.size1() << '\t' << mat.size2() << '\n';
  for (std::size_t x = 0; x < mat.size1(); ++x) {
    for (std::size_t y = 0; y < mat.size2(); ++y) {
      outf << mat(x, y) << '\t';
    }
    outf << '\n';
  }
  outf.close();
  VLOG(2) << "Saved matrix to " << path;
  return true;
}

} /* namespace toyml */
