#ifndef GSA_JSON_IO_HPP
#define GSA_JSON_IO_HPP

#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

#include "gsa/gsa.hpp"

namespace gsa {

namespace detail {

// Normalize nlohmann's exception types so every loader failure surfaces
// as std::invalid_argument.
template <typename Fn>
inline auto Guard(Fn&& fn) -> decltype(fn()) {
  try {
    return fn();
  } catch (const nlohmann::json::exception& e) {
    throw std::invalid_argument(e.what());
  }
}

// Reject semantically invalid values that slipped through type parsing.
inline void ValidateConfig(const GsaConfig& cfg) {
  if (cfg.n_agents == 0) {
    throw std::invalid_argument(R"("n_agents" must be > 0)");
  }
  if (cfg.max_iter == 0) {
    throw std::invalid_argument(R"("max_iter" must be > 0)");
  }
  if (!(cfg.g0 > 0.0)) {
    throw std::invalid_argument(R"("g0" must be > 0)");
  }
  if (cfg.alpha < 0.0) {
    throw std::invalid_argument(R"("alpha" must be >= 0)");
  }
}

// One side of the bounds: a scalar filled to `dims`, or an array whose
// length must match `dims` when given.
inline std::vector<double> ParseBoundsSide(const std::string& name,
                                           const nlohmann::json& v,
                                           size_t dims) {
  if (v.is_number()) {
    if (dims == 0) {
      throw std::invalid_argument(
          R"(("dimensions" is required with scalar bounds))");
    }
    return std::vector<double>(dims, v.get<double>());
  }
  if (!v.is_array()) {
    throw std::invalid_argument('"' + name + R"(" must be a number or array)");
  }
  std::vector<double> out;
  out.reserve(v.size());
  for (const auto& e : v) out.push_back(e.get<double>());
  if (out.empty()) {
    throw std::invalid_argument('"' + name + R"(" array is empty)");
  }
  if (dims != 0 && out.size() != dims) {
    throw std::invalid_argument(
        '"' + name + R"(" array length must equal "dimensions")");
  }
  return out;
}

}  // namespace detail

struct Bounds {
  size_t dimensions{};
  std::vector<double> lower, upper;
};

// Load GsaConfig from nlohmann::json object
inline GsaConfig LoadConfigFromJson(const nlohmann::json& j) {
  return detail::Guard([&] {
    GsaConfig cfg;
    if (j.contains("n_agents")) cfg.n_agents = j["n_agents"].get<size_t>();
    if (j.contains("max_iter")) cfg.max_iter = j["max_iter"].get<size_t>();
    if (j.contains("g0")) cfg.g0 = j["g0"].get<double>();
    if (j.contains("alpha")) cfg.alpha = j["alpha"].get<double>();
    if (j.contains("minimize")) cfg.minimize = j["minimize"].get<bool>();
    if (j.contains("seed")) cfg.seed = j["seed"].get<uint64_t>();
    detail::ValidateConfig(cfg);
    return cfg;
  });
}

// Load problem dimensions and bounds from a JSON object.
// "lower"/"upper" may be scalars or per-dimension arrays.
inline Bounds LoadBoundsFromJson(const nlohmann::json& j) {
  return detail::Guard([&] {
    if (!j.contains("lower") || !j.contains("upper")) {
      throw std::invalid_argument(R"(config requires both "lower" and "upper")");
    }

    size_t dims{0};
    if (j.contains("dimensions")) {
      dims = j["dimensions"].get<size_t>();
      if (dims == 0) {
        throw std::invalid_argument(R"(("dimensions" must be > 0))");
      }
    }

    Bounds b;
    b.lower = detail::ParseBoundsSide("lower", j.at("lower"), dims);
    b.upper = detail::ParseBoundsSide("upper", j.at("upper"), dims);
    if (b.lower.size() != b.upper.size()) {
      throw std::invalid_argument(
          R"(("lower" and "upper" must have the same length))");
    }
    b.dimensions = b.lower.size();
    for (auto i : Range(b.dimensions)) {
      if (b.lower[i] > b.upper[i]) {
        throw std::invalid_argument(
            R"((each lower bound must be <= its upper bound))");
      }
    }
    return b;
  });
}

// Load GsaConfig from JSON file
inline GsaConfig LoadConfigFromFile(const std::string& path) {
  std::ifstream file(path);
  if (!file) throw std::invalid_argument("Cannot open config file: " + path);
  return detail::Guard([&] {
    nlohmann::json j;
    file >> j;
    return LoadConfigFromJson(j);
  });
}

// Load GsaConfig from JSON string
inline GsaConfig LoadConfigFromString(const std::string& json_str) {
  return detail::Guard(
      [&] { return LoadConfigFromJson(nlohmann::json::parse(json_str)); });
}

}  // namespace gsa

#endif  // GSA_JSON_IO_HPP
