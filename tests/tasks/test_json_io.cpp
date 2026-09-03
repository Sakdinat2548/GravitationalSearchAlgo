#include "gsa/gsa.hpp"
#include "gsa/json_io.hpp"
#include "../test_framework.hpp"

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

template <typename Fn>
bool Throws(std::string_view label, std::string_view json_str, Fn&& load) {
  try {
    load(json_str);
  } catch (const std::invalid_argument&) {
    return true;
  }
  gsa_test::Expect(false, std::string(label));
  return false;
}

}  // namespace

TEST(json_io_config) {
  bool ok{true};

  const auto cfg{gsa::LoadConfigFromString(R"({"n_agents": 7})")};
  gsa_test::Expect(cfg.n_agents == 7 && cfg.max_iter == 500 &&
                       cfg.g0 == 100.0 && cfg.alpha == 20.0 &&
                       cfg.minimize && cfg.seed == 0,
                   "defaults fill omitted fields");

  ok = Throws("zero n_agents rejected", R"({"n_agents": 0})",
              gsa::LoadConfigFromString) &&
       ok;
  ok = Throws("zero max_iter rejected", R"({"max_iter": 0})",
              gsa::LoadConfigFromString) &&
       ok;
  ok = Throws("negative g0 rejected", R"({"g0": -1.0})",
              gsa::LoadConfigFromString) &&
       ok;
  ok = Throws("negative alpha rejected", R"({"alpha": -0.5})",
               gsa::LoadConfigFromString) &&
        ok;
  ok = Throws("snapshot_count above max_iter + 1 rejected",
              R"({"max_iter": 11, "snapshot_count": 13})",
              gsa::LoadConfigFromString) &&
       ok;
  ok = Throws("type mismatch rejected", R"({"seed": "abc"})",
              gsa::LoadConfigFromString) &&
       ok;
  ok = Throws("malformed JSON rejected", R"({not valid json])",
              gsa::LoadConfigFromString) &&
       ok;

  const auto all{gsa::LoadConfigFromString(
      R"({"n_agents": 9, "max_iter": 11, "g0": 2.5, "alpha": 0.25,
          "minimize": false, "seed": 42, "snapshot_count": 5})")};
  gsa_test::Expect(all.n_agents == 9 && all.max_iter == 11 && all.g0 == 2.5 &&
                       all.alpha == 0.25 && !all.minimize && all.seed == 42 &&
                       all.snapshot_count == 5,
                   "all fields parsed");
  return ok;
}

TEST(json_io_bounds) {
  bool ok{true};

  const auto scalar{gsa::LoadBoundsFromJson(nlohmann::json::parse(
      R"({"dimensions": 4, "lower": -2.0, "upper": 3.0})"))};
  gsa_test::Expect(scalar.dimensions == 4 && scalar.lower.size() == 4 &&
                       scalar.upper.size() == 4 && scalar.lower[2] == -2.0 &&
                       scalar.upper[2] == 3.0,
                   "scalar bounds expand to dims");

  const auto arrays{gsa::LoadBoundsFromJson(nlohmann::json::parse(
      R"({"lower": [-5, 0, -1], "upper": [5, 50, 1]})"))};
  gsa_test::Expect(arrays.dimensions == 3 && arrays.lower[1] == 0.0 &&
                       arrays.upper[1] == 50.0,
                   "arrays define dims when omitted");

  const auto mixed{gsa::LoadBoundsFromJson(nlohmann::json::parse(
      R"({"dimensions": 3, "lower": -2.048, "upper": [5, 6, 7]})"))};
  gsa_test::Expect(mixed.dimensions == 3 && mixed.lower.size() == 3 &&
                       mixed.lower[0] == -2.048 && mixed.upper[1] == 6.0,
                   "scalar lower + array upper");

  const auto rev{gsa::LoadBoundsFromJson(nlohmann::json::parse(
      R"({"dimensions": 2, "lower": [-1, -2], "upper": 5})"))};
  gsa_test::Expect(rev.lower.size() == 2 && rev.upper[0] == 5.0 &&
                       rev.upper[1] == 5.0,
                   "array lower + scalar upper");

  ok = Throws("scalar side requires dimensions",
              R"({"lower": -1, "upper": [1, 2, 3]})",
              [](std::string_view s) {
                return gsa::LoadBoundsFromJson(nlohmann::json::parse(s));
              }) &&
       ok;
  ok = Throws("missing lower rejected",
              R"({"upper": [1]})", [](std::string_view s) {
                return gsa::LoadBoundsFromJson(nlohmann::json::parse(s));
              }) &&
       ok;
  ok = Throws("length mismatch rejected", R"({"dimensions": 3,
                                              "lower": [-1, -1],
                                              "upper": [1, 1, 1]})",
              [](std::string_view s) {
                return gsa::LoadBoundsFromJson(nlohmann::json::parse(s));
              }) &&
       ok;
  ok = Throws("empty array rejected", R"({"lower": [], "upper": []})",
              [](std::string_view s) {
                return gsa::LoadBoundsFromJson(nlohmann::json::parse(s));
              }) &&
       ok;
  ok = Throws("inverted bound rejected", R"({"lower": 5.0, "upper": 1.0})",
              [](std::string_view s) {
                return gsa::LoadBoundsFromJson(nlohmann::json::parse(s));
              }) &&
       ok;
  ok = Throws("zero dims rejected", R"({"dimensions": 0, "lower": [],
                                      "upper": []})",
              [](std::string_view s) {
                return gsa::LoadBoundsFromJson(nlohmann::json::parse(s));
              }) &&
       ok;
  return ok;
}