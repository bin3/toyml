/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-10-21
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <boost/numeric/ublas/matrix.hpp>

namespace toyml {

namespace ublas = boost::numeric::ublas;

#define NAME_VAL(v) #v << "=" << v
#define NAME_VAL_COMMA(v) #v << "=" << v << ", "
#define NV_ NAME_VAL
#define NVC_ NAME_VAL_COMMA

/**
 * @brief 
 */
class Utils {
public:
  Utils();
  virtual ~Utils();

  static bool SaveMatrix(const ublas::matrix<double>& mat,
      const std::string& path);
};

} /* namespace toyml */
#endif /* UTILS_H_ */
