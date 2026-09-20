#include <stdexcept>

#include <gtest/gtest.h>

#include "../test_common.hpp"

using gsa_test::Config;
using gsa_test::Sphere;

TEST(Validation, RejectsBadConfig) {
  auto empty_bounds = [] {
    gsa::GravitationalSearchAlgorithm gsa(std::vector<double>{},
                                          std::vector<double>{1.0}, Sphere);
  };
  EXPECT_THROW(empty_bounds(), std::invalid_argument);

  auto bound_size_mismatch = [] {
    gsa::GravitationalSearchAlgorithm gsa(std::vector<double>{1.0},
                                          std::vector<double>{1.0, 2.0},
                                          Sphere);
  };
  EXPECT_THROW(bound_size_mismatch(), std::invalid_argument);

  auto zero_agents = [] {
    gsa::GravitationalSearchAlgorithm gsa(std::vector<double>{-5.0},
                                          std::vector<double>{5.0}, Sphere,
                                          Config(true, 0));
  };
  EXPECT_THROW(zero_agents(), std::invalid_argument);

  auto zero_iterations = [] {
    gsa::GravitationalSearchAlgorithm gsa(
        std::vector<double>{-5.0}, std::vector<double>{5.0}, Sphere,
        {.n_agents = 50,
         .max_iter = 0,
         .g0 = 100.0,
         .alpha = 20.0,
         .minimize = true,
         .seed = 12345});
  };
  EXPECT_THROW(zero_iterations(), std::invalid_argument);

  auto lower_above_upper = [] {
    gsa::GravitationalSearchAlgorithm gsa(std::vector<double>{5.0},
                                          std::vector<double>{1.0}, Sphere);
  };
  EXPECT_THROW(lower_above_upper(), std::invalid_argument);

  auto snapshot_count_above_max = [] {
    gsa::GravitationalSearchAlgorithm gsa(
        std::vector<double>{-5.0}, std::vector<double>{5.0}, Sphere,
        {.n_agents = 50,
         .max_iter = 500,
         .g0 = 100.0,
         .alpha = 20.0,
         .minimize = true,
         .seed = 12345,
         .snapshot_count = 502});
  };
  EXPECT_THROW(snapshot_count_above_max(), std::invalid_argument);
}
