#include "../test_common.hpp"
#include "../test_framework.hpp"

#include <stdexcept>

using gsa_test::Config;
using gsa_test::Sphere;

namespace {

template <typename Fn>
bool ExpectThrows(std::string_view label, Fn&& f) {
  try {
    f();
  } catch (const std::invalid_argument&) {
    return true;
  }
  gsa_test::Expect(false, label);
  return false;
}

}  // namespace

TEST(validation) {
  bool ok{true};

  ok = ExpectThrows("empty bounds",
                    [] {
                      GravitationalSearchAlgorithm gsa(std::vector<double>{},
                                                       std::vector<double>{1.0},
                                                       Sphere);
                    }) &&
       ok;
  ok = ExpectThrows("bound size mismatch",
                    [] {
                      GravitationalSearchAlgorithm gsa(std::vector<double>{1.0},
                                                       std::vector<double>{1.0,
                                                                           2.0},
                                                       Sphere);
                    }) &&
       ok;
  ok = ExpectThrows("zero agents",
                    [] {
                      GravitationalSearchAlgorithm gsa(std::vector<double>{-5.0},
                                                       std::vector<double>{5.0},
                                                       Sphere,
                                                       Config(true, 0));
                    }) &&
       ok;
  ok = ExpectThrows("zero iterations",
                    [] {
                      GravitationalSearchAlgorithm gsa(
                          std::vector<double>{-5.0}, std::vector<double>{5.0},
                          Sphere,
                          {.n_agents = 50,
                           .max_iter = 0,
                           .g0 = 100.0,
                           .alpha = 20.0,
                           .minimize = true,
                           .seed = 12345});
                    }) &&
       ok;
  ok = ExpectThrows("lower above upper",
                    [] {
                      GravitationalSearchAlgorithm gsa(std::vector<double>{5.0},
                                                       std::vector<double>{1.0},
                                                       Sphere);
                    }) &&
       ok;

  return ok;
}