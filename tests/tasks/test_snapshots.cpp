#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include "../test_common.hpp"
#include "gsa/csv_io.hpp"

using gsa_test::Config;
using gsa_test::Optimize;
using gsa_test::Sphere;

namespace {

gsa::GsaConfig SnapConfig(size_t count) {
  auto cfg = Config();
  cfg.snapshot_count = count;
  return cfg;
}

size_t CountLines(const std::filesystem::path& path) {
  std::ifstream in{path};
  size_t n{};
  std::string line;
  while (std::getline(in, line)) ++n;
  return n;
}

}  // namespace

TEST(Snapshots, CaptureAndCsvRoundTrip) {
  const auto off = Optimize(2, -5.0, 5.0, Sphere, SnapConfig(0));
  EXPECT_TRUE(off.snapshot_iters.empty() && off.snapshot_positions.empty())
      << "count 0: snapshots not empty";
  EXPECT_EQ(off.snapshot_dims, 2) << "count 0: snapshot_dims != dims";

  const auto one = Optimize(2, -5.0, 5.0, Sphere, SnapConfig(1));
  EXPECT_EQ(one.snapshot_iters, std::vector<size_t>({500})) << "count 1";

  const auto six = Optimize(2, -5.0, 5.0, Sphere, SnapConfig(6));
  EXPECT_EQ(six.snapshot_iters,
            std::vector<size_t>({0, 100, 200, 300, 400, 500}))
      << "count 6";
  EXPECT_EQ(six.snapshot_positions.size(), 6 * 50 * 2)
      << "count 6: flat size != snaps*agents*dims";
  EXPECT_EQ(six.snapshot_masses.size(), 6 * 50)
      << "count 6: mass size != snaps*agents";
  EXPECT_EQ(six.snapshot_fitnesses.size(), 6 * 50)
      << "count 6: fitness size != snaps*agents";
  for (size_t s{}; s < six.snapshot_iters.size(); ++s) {
    SCOPED_TRACE(std::format("snap {}", s));
    double mass_sum{};
    double fmin{1e300};
    double fmax{-1e300};
    for (size_t a{}; a < 50; ++a) {
      const size_t flat{(s * 50) + a};
      const double m{six.snapshot_masses[flat]};
      EXPECT_GE(m, 0.0) << "snapshot mass outside [0, 1]";
      EXPECT_LE(m, 1.0) << "snapshot mass outside [0, 1]";
      mass_sum += m;
      const size_t base{flat * 2};
      const double fx{six.snapshot_fitnesses[flat]};
      const double x{six.snapshot_positions[base]};
      const double y{six.snapshot_positions[base + 1]};
      fmin = std::min(fmin, fx);
      fmax = std::max(fmax, fx);
      EXPECT_EQ(fx, (x * x) + (y * y)) << "snapshot fitness != sphere(position)";
    }
    if (fmax > fmin) {
      EXPECT_GE(mass_sum, 0.999) << "snapshot masses do not sum to 1";
      EXPECT_LE(mass_sum, 1.001) << "snapshot masses do not sum to 1";
    }
  }

  auto full_cfg = Config();
  full_cfg.snapshot_count = full_cfg.max_iter + 1;
  const auto full = Optimize(2, -5.0, 5.0, Sphere, full_cfg);
  EXPECT_EQ(full.snapshot_iters.size(), 501) << "full: size != 501";
  EXPECT_EQ(full.snapshot_iters.front(), 0);
  EXPECT_EQ(full.snapshot_iters.back(), 500);

  const auto again = Optimize(2, -5.0, 5.0, Sphere, SnapConfig(6));
  EXPECT_EQ(six.snapshot_iters, again.snapshot_iters);
  EXPECT_EQ(six.snapshot_positions, again.snapshot_positions)
      << "same seed: snapshots differ";
  EXPECT_EQ(six.snapshot_masses, again.snapshot_masses);
  EXPECT_EQ(six.snapshot_fitnesses, again.snapshot_fitnesses);

  auto snapshot_count_above_max = [] {
    auto bad = Config();
    bad.snapshot_count = bad.max_iter + 2;
    gsa::GravitationalSearchAlgorithm gsa(std::vector<double>{-5.0},
                                          std::vector<double>{5.0}, Sphere, bad);
  };
  EXPECT_THROW(snapshot_count_above_max(), std::invalid_argument)
      << "count > max_iter+1: no throw";

  const auto dir{std::filesystem::temp_directory_path() / "gsa_snap_test"};
  std::filesystem::create_directories(dir);
  gsa::WriteHistoryCsv(six, dir / "history.csv");
  gsa::WriteSnapshotsCsv(six, dir / "snapshots.csv");
  EXPECT_EQ(CountLines(dir / "history.csv"), 502)
      << "history.csv: line count != max_iter + 2";
  EXPECT_EQ(CountLines(dir / "snapshots.csv"), (6 * 50) + 1)
      << "snapshots.csv: line count != snaps*agents + 1";
  std::ifstream hist{dir / "history.csv"};
  std::string header;
  std::getline(hist, header);
  EXPECT_EQ(header,
            "best_so_far,best_iter,worst_iter,mean_fitness,median_fitness,"
            "stddev_fitness")
      << "history.csv: header mismatch";
  std::ifstream snaps{dir / "snapshots.csv"};
  std::string snap_header;
  std::getline(snaps, snap_header);
  EXPECT_EQ(snap_header, "iter,agent,mass,fitness,x1,x2")
      << "snapshots.csv: header mismatch";
  std::string first;
  std::getline(hist, first);
  const double back{std::stod(first.substr(0, first.find(',')))};
  EXPECT_EQ(back, six.history[0].best_so_far)
      << "history.csv: best_so_far does not round-trip";

  auto ragged = six;
  ragged.snapshot_positions.pop_back();
  EXPECT_THROW(gsa::WriteSnapshotsCsv(ragged, dir / "ragged.csv"),
               std::invalid_argument)
      << "ragged snapshots accepted";
  auto zero_dims = six;
  zero_dims.snapshot_dims = 0;
  EXPECT_THROW(gsa::WriteSnapshotsCsv(zero_dims, dir / "zero_dims.csv"),
               std::invalid_argument)
      << "zero snapshot_dims accepted";
  EXPECT_THROW(gsa::WriteHistoryCsv(six, dir), std::runtime_error)
      << "directory path accepted for writing";

  std::error_code ec;
  std::filesystem::remove_all(dir, ec);
}
