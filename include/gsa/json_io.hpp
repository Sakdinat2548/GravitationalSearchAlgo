#ifndef GSA_JSON_IO_HPP
#define GSA_JSON_IO_HPP

#include <format>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "gsa/gsa.hpp"

namespace gsa {

namespace detail {

// Normalize nlohmann's exception types so every loader failure surfaces
// as std::invalid_argument.
template <std::invocable Fn>
inline auto Guard(Fn&& fn) -> decltype(fn()) {
  try {
    return fn();
  } catch (const nlohmann::json::exception& e) {
    throw std::invalid_argument(e.what());
  }
}

inline void ValidateConfig(const GsaConfig& cfg) {
  if (cfg.n_agents == 0) [[unlikely]] {
    throw std::invalid_argument(R"("n_agents" must be > 0)");
  }
  if (cfg.max_iter == 0) [[unlikely]] {
    throw std::invalid_argument(R"("max_iter" must be > 0)");
  }
  if (!(cfg.g0 > 0.0)) [[unlikely]] {
    throw std::invalid_argument(R"("g0" must be > 0)");
  }
  if (cfg.alpha < 0.0) [[unlikely]] {
    throw std::invalid_argument(R"("alpha" must be >= 0)");
  }
}

inline std::vector<double> ParseBoundsSide(std::string_view name,
                                           const nlohmann::json& v,
                                           size_t dims) {
  if (v.is_number()) {
    if (dims == 0) [[unlikely]] {
      throw std::invalid_argument(std::format(
          R"("dimensions" is required with scalar bounds for "{}")", name));
    }
    return std::vector<double>(dims, v.get<double>());
  }
  if (!v.is_array()) [[unlikely]] {
    throw std::invalid_argument(
        std::format(R"("{}") must be a number or array)", name));
  }
  std::vector<double> out = v.get<std::vector<double>>();
  if (out.empty()) [[unlikely]] {
    throw std::invalid_argument(std::format(R"("{}") array is empty)", name));
  }
  if (dims != 0 && out.size() != dims) [[unlikely]] {
    throw std::invalid_argument(std::format(
        R"("{}") array length must equal "dimensions" ({}))", name, dims));
  }
  return out;
}

}  // namespace detail

struct Bounds {
  size_t dimensions{};
  std::vector<double> lower, upper;
};

inline GsaConfig LoadConfigFromJson(const nlohmann::json& j) {
  return detail::Guard([&] {
    GsaConfig cfg;
    cfg.n_agents = j.value("n_agents", cfg.n_agents);
    cfg.max_iter = j.value("max_iter", cfg.max_iter);
    cfg.g0 = j.value("g0", cfg.g0);
    cfg.alpha = j.value("alpha", cfg.alpha);
    cfg.minimize = j.value("minimize", cfg.minimize);
    cfg.seed = j.value("seed", cfg.seed);
    detail::ValidateConfig(cfg);
    return cfg;
  });
}

inline Bounds LoadBoundsFromJson(const nlohmann::json& j) {
  return detail::Guard([&] {
    if (!j.contains("lower") || !j.contains("upper")) [[unlikely]] {
      throw std::invalid_argument(
          R"(config requires both "lower" and "upper")");
    }

    size_t dims{0};
    if (j.contains("dimensions")) {
      dims = j.value("dimensions", size_t{0});
      if (dims == 0) [[unlikely]] {
        throw std::invalid_argument(R"(("dimensions" must be > 0))");
      }
    }

    Bounds b;
    b.lower = detail::ParseBoundsSide("lower", j.at("lower"), dims);
    b.upper = detail::ParseBoundsSide("upper", j.at("upper"), dims);
    if (b.lower.size() != b.upper.size()) [[unlikely]] {
      throw std::invalid_argument(
          R"(("lower" and "upper" must have the same length))");
    }
    b.dimensions = b.lower.size();
    for (auto i : Range(b.dimensions)) {
      if (b.lower[i] > b.upper[i]) [[unlikely]] {
        throw std::invalid_argument(
            std::format("lower[{}] = {} > upper[{}] = {}: lower bound must be "
                        "<= upper bound",
                        i, b.lower[i], i, b.upper[i]));
      }
    }
    return b;
  });
}

inline GsaConfig LoadConfigFromString(const std::string& json_str) {
  return detail::Guard(
      [&] { return LoadConfigFromJson(nlohmann::json::parse(json_str)); });
}

}  // namespace gsa

#endif  // GSA_JSON_IO_HPP
