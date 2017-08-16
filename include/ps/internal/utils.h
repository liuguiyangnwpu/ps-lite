/**
 *  Copyright (c) 2015 by Contributors
 */
#ifndef PS_INTERNAL_UTILS_H_
#define PS_INTERNAL_UTILS_H_
#include "dmlc/logging.h"
#include "ps/internal/env.h"
namespace ps {

#include <inttypes.h>

/*!
 * \brief Get environment variable as int with default.
 * \param key the name of environment variable.
 * \param default_val the default value of environment vriable.
 * \return The value received
 */
template<typename V>
inline V GetEnv(const char *key, V default_val) {
  	const char *val = Environment::Get()->find(key);
  	if (val == nullptr) {
    	return default_val;
  	} else {
    	return atoi(val);
  	}
}

#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
	TypeName(const TypeName&);               \
  	void operator=(const TypeName&)
#endif

#define LL LOG(ERROR)

}  // namespace ps
#endif  // PS_INTERNAL_UTILS_H_
