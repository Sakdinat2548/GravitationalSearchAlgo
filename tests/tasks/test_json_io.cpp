#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "gsa/gsa.hpp"
#include "gsa/json_io.hpp"

TEST(JsonIo, ConfigDefaultsAndErrors) {
  const auto cfg{gsa::LoadConfigFromString(R"({"n_agents": 7})")};
  EXPECT_TRUE(cfg.n_agents == 7 && cfg.max_iter == 500 && cfg.g0 == 100.0 &&
              cfg.alpha == 20.0 && cfg.minimize && cfg.seed == 0)
      << "defaults fill omitted fields";

  EXPECT_THROW(gsa::LoadConfigFromString(R"({"n_agents": 0})"),
               std::invalid_argument)
      << "zero n_agents rejected";
  EXPECT_THROW(gsa::LoadConfigFromString(R"({"max_iter": 0})"),
               std::invalid_argument)
      << "zero max_iter rejected";
  EXPECT_THROW(gsa::LoadConfigFromString(R"({"g0": -1.0})"),
               std::invalid_argument)
      << "negative g0 rejected";
  EXPECT_THROW(gsa::LoadConfigFromString(R"({"alpha": -0.5})"),
               std::invalid_argument)
      << "negative alpha rejected";
  EXPECT_THROW(
      gsa::LoadConfigFromString(R"({"max_iter": 11, "snapshot_count": 13})"),
      std::invalid_argument)
      << "snapshot_count above max_iter + 1 rejected";
  EXPECT_THROW(gsa::LoadConfigFromString(R"({"seed": "abc"})"),
               std::invalid_argument)
      << "type mismatch rejected";
  EXPECT_THROW(gsa::LoadConfigFromString(R"({not valid json])"),
               std::invalid_argument)
      << "malformed JSON rejected";

  const auto all{gsa::LoadConfigFromString(
      R"({"n_agents": 9, "max_iter": 11, "g0": 2.5, "alpha": 0.25,
          "minimize": false, "seed": 42, "snapshot_count": 5})")};
  EXPECT_TRUE(all.n_agents == 9 && all.max_iter == 11 && all.g0 == 2.5 &&
              all.alpha == 0.25 && !all.minimize && all.seed == 42 &&
              all.snapshot_count == 5)
      << "all fields parsed";
}

TEST(JsonIo, BoundsParsingAndErrors) {
  const auto scalar{gsa::LoadBoundsFromJson(nlohmann::json::parse(
      R"({"dimensions": 4, "lower": -2.0, "upper": 3.0})"))};
  EXPECT_TRUE(scalar.dimensions == 4 && scalar.lower.size() == 4 &&
              scalar.upper.size() == 4 && scalar.lower[2] == -2.0 &&
              scalar.upper[2] == 3.0)
      << "scalar bounds expand to dims";

  const auto arrays{gsa::LoadBoundsFromJson(nlohmann::json::parse(
      R"({"lower": [-5, 0, -1], "upper": [5, 50, 1]})"))};
  EXPECT_TRUE(arrays.dimensions == 3 && arrays.lower[1] == 0.0 &&
              arrays.upper[1] == 50.0)
      << "arrays define dims when omitted";

  const auto mixed{gsa::LoadBoundsFromJson(nlohmann::json::parse(
      R"({"dimensions": 3, "lower": -2.048, "upper": [5, 6, 7]})"))};
  EXPECT_TRUE(mixed.dimensions == 3 && mixed.lower.size() == 3 &&
              mixed.lower[0] == -2.048 && mixed.upper[1] == 6.0)
      << "scalar lower + array upper";

  const auto rev{gsa::LoadBoundsFromJson(nlohmann::json::parse(
      R"({"dimensions": 2, "lower": [-1, -2], "upper": 5})"))};
  EXPECT_TRUE(rev.lower.size() == 2 && rev.upper[0] == 5.0 &&
              rev.upper[1] == 5.0)
      << "array lower + scalar upper";

  auto load_bounds = [](std::string_view s) {
    return gsa::LoadBoundsFromJson(nlohmann::json::parse(s));
  };
  EXPECT_THROW(load_bounds(R"({"lower": -1, "upper": [1, 2, 3]})"),
               std::invalid_argument)
      << "scalar side requires dimensions";
  EXPECT_THROW(load_bounds(R"({"upper": [1]})"), std::invalid_argument)
      << "missing lower rejected";
  EXPECT_THROW(load_bounds(R"({"dimensions": 3,
                               "lower": [-1, -1],
                               "upper": [1, 1, 1]})"),
               std::invalid_argument)
      << "length mismatch rejected";
  EXPECT_THROW(load_bounds(R"({"lower": [], "upper": []})"),
               std::invalid_argument)
      << "empty array rejected";
  EXPECT_THROW(load_bounds(R"({"lower": 5.0, "upper": 1.0})"),
               std::invalid_argument)
      << "inverted bound rejected";
  EXPECT_THROW(load_bounds(R"({"dimensions": 0, "lower": [],
                               "upper": []})"),
               std::invalid_argument)
      << "zero dims rejected";
}
