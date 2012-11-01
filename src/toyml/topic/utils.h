/*
 * Copyright (c) 2012 Binson Zhang. All rights reserved.
 *
 * @author	Binson Zhang <bin183cs@gmail.com>
 * @date		2012-10-21
 */

#ifndef UTILS_H_
#define UTILS_H_

namespace toyml {

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
};

} /* namespace toyml */
#endif /* UTILS_H_ */
