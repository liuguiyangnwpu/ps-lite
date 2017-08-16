/**
 * Copyright (c) 2016 by Contributors
 */
#ifndef PS_INTERNAL_ENV_H_
#define PS_INTERNAL_ENV_H_
#include <cstdlib>
#include <unordered_map>
#include <memory>
#include <string>

namespace ps {

class Environment {
public:
    static inline Environment* Get() {
        return _GetSharedRef(nullptr).get();
    }

    static inline std::shared_ptr<Environment> _GetSharedRef() {
        return _GetSharedRef(nullptr);
    }
  
    static inline Environment* Init(const std::unordered_map<std::string, std::string>& envs) {
        return _GetSharedRef(&envs).get();
    }

    const char* find(const char* k) {
        std::string key(k);
        return kvs.find(key) == kvs.end() ? getenv(k) : kvs[key].c_str();
    }

private:
    explicit Environment(const std::unordered_map<std::string, std::string>* envs) {
        if (envs) kvs = *envs;
    }

    static std::shared_ptr<Environment> _GetSharedRef(const std::unordered_map<std::string, std::string>* envs) {
        static std::shared_ptr<Environment> inst_ptr(new Environment(envs));
        return inst_ptr;
    }

    std::unordered_map<std::string, std::string> kvs;
};

}  // namespace ps
#endif  // PS_INTERNAL_ENV_H_
