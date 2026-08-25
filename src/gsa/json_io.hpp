#ifndef GSA_JSON_IO_HPP
#define GSA_JSON_IO_HPP

#include <concepts>
#include <cstddef>
#include <format>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#include "gsa/gsa.hpp"

namespace gsa {

struct Bounds {
  std::size_t dimensions{0};
  std::vector<double> lower;
  std::vector<double> upper;
};

namespace detail {

template <std::invocable Fn>
inline auto Guard(Fn&& fn) -> decltype(fn()) {
  try {
    return std::forward<Fn>(fn)();
  } catch (const nlohmann::json::exception& e) {
    throw std::invalid_argument(e.what());
  }
}

inline void ValidateConfig(const GsaConfig& cfg) {
  if (cfg.n_agents == 0) [[unlikely]] {
    throw std::invalid_argument("\"n_agents\" must be > 0");
  }
  if (cfg.max_iter == 0) [[unlikely]] {
    throw std::invalid_argument("\"max_iter\" must be > 0");
  }
  if (cfg.g0 <= 0.0) [[unlikely]] {
    throw std::invalid_argument("\"g0\" must be > 0");
  }
  if (cfg.alpha < 0.0) [[unlikely]] {
    throw std::invalid_argument("\"alpha\" must be >= 0");
  }
}

inline std::vector<double> ParseBoundsSide(std::string_view name,
                                           const nlohmann::json& v,
                                           std::size_t dims) {
  if (v.is_number()) {
    if (dims == 0) [[unlikely]] {
      throw std::invalid_argument(
          "\"dimensions\" is required when specifying scalar bounds");
    }
    return std::vector<double>(dims, v.get<double>());
  }

  if (v.is_array()) {
    auto vec = v.get<std::vector<double>>();
    if (vec.empty()) [[unlikely]] {
      throw std::invalid_argument(
          std::format("\"{}\" array cannot be empty", name));
    }
    if (dims != 0 && vec.size() != dims) [[unlikely]] {
      throw std::invalid_argument(
          std::format(R"("{}" array length must match "dimensions")", name));
    }
    return vec;
  }

  throw std::invalid_argument(
      std::format("\"{}\" must be a number or array of numbers", name));
}

}  // namespace detail

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
          R"(Config requires both "lower" and "upper" bounds)");
    }

    std::size_t dims = 0;
    if (j.contains("dimensions")) {
      dims = j["dimensions"].get<std::size_t>();
      if (dims == 0) [[unlikely]] {
        throw std::invalid_argument("\"dimensions\" must be > 0");
      }
    }

    Bounds b;
    b.lower = detail::ParseBoundsSide("lower", j.at("lower"), dims);
    b.upper = detail::ParseBoundsSide("upper", j.at("upper"), dims);

    if (b.lower.size() != b.upper.size()) [[unlikely]] {
      throw std::invalid_argument(
        R"("lower" and "upper" bounds must have equal lengths)");
    }

    b.dimensions = b.lower.size();

    for (const auto i : Range(b.dimensions)) {
      if (b.lower[i] > b.upper[i]) [[unlikely]] {
        throw std::invalid_argument(
            std::format("Lower bound ({}) cannot be greater than upper bound "
                        "({}) at index {}",
                        b.lower[i], b.upper[i], i));
      }
    }

    return b;
  });
}

inline GsaConfig LoadConfigFromString(const std::string_view json_str) {
  return detail::Guard(
      [&] { return LoadConfigFromJson(nlohmann::json::parse(json_str)); });
}

}  // namespace gsa

#endif  // GSA_JSON_IO_HPP
