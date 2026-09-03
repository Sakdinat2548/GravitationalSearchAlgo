#include "../test_common.hpp"
#include "../test_framework.hpp"

#include <filesystem>
#include <format>
#include <fstream>
#include <string>

#include "gsa/csv_io.hpp"

using gsa_test::Config;
using gsa_test::Expect;
using gsa_test::Optimize;
using gsa_test::Sphere;

namespace {

gsa::GsaConfig SnapConfig(size_t count) {
  auto cfg = Config();
  cfg.snapshot_count = count;
  return cfg;
}

bool CheckIters(const std::vector<size_t>& got,
                const std::vector<size_t>& want, std::string_view what) {
  if (got != want) {
    Expect(false, std::format("{}: iters mismatch (got {}, want {})", what,
                              got.size(), want.size()));
    return false;
  }
  return true;
}

size_t CountLines(const std::filesystem::path& path) {
  std::ifstream in{path};
  size_t n{};
  std::string line;
  while (std::getline(in, line)) ++n;
  return n;
}

}  // namespace

TEST(snapshots) {
  bool ok{true};

  const auto off = Optimize(2, -5.0, 5.0, Sphere, SnapConfig(0));
  ok = off.snapshot_iters.empty() && off.snapshot_positions.empty() && ok;
  if (!off.snapshot_iters.empty()) Expect(false, "count 0: iters not empty");
  if (off.snapshot_dims != 2) {
    Expect(false, "count 0: snapshot_dims != dims");
    ok = false;
  }

  const auto one = Optimize(2, -5.0, 5.0, Sphere, SnapConfig(1));
  ok = CheckIters(one.snapshot_iters, {500}, "count 1") && ok;

  const auto six = Optimize(2, -5.0, 5.0, Sphere, SnapConfig(6));
  ok = CheckIters(six.snapshot_iters, {0, 100, 200, 300, 400, 500},
                  "count 6") &&
       ok;
  if (six.snapshot_positions.size() != 6 * 50 * 2) {
    Expect(false, "count 6: flat size != snaps*agents*dims");
    ok = false;
  }

  auto full_cfg = Config();
  full_cfg.snapshot_count = full_cfg.max_iter + 1;
  const auto full = Optimize(2, -5.0, 5.0, Sphere, full_cfg);
  ok = full.snapshot_iters.size() == 501 &&
       full.snapshot_iters.front() == 0 &&
       full.snapshot_iters.back() == 500 && ok;
  if (full.snapshot_iters.size() != 501) Expect(false, "full: size != 501");

  const auto again = Optimize(2, -5.0, 5.0, Sphere, SnapConfig(6));
  ok = six.snapshot_iters == again.snapshot_iters &&
       six.snapshot_positions == again.snapshot_positions && ok;
  if (six.snapshot_positions != again.snapshot_positions) {
    Expect(false, "same seed: snapshots differ");
  }

  try {
    auto bad = Config();
    bad.snapshot_count = bad.max_iter + 2;
    gsa::GravitationalSearchAlgorithm gsa(std::vector<double>{-5.0},
                                          std::vector<double>{5.0}, Sphere, bad);
    Expect(false, "count > max_iter+1: no throw");
    ok = false;
  } catch (const std::invalid_argument&) {
  }

  const auto dir{std::filesystem::temp_directory_path() / "gsa_snap_test"};
  std::filesystem::create_directories(dir);
  gsa::WriteHistoryCsv(six, dir / "history.csv");
  gsa::WriteSnapshotsCsv(six, dir / "snapshots.csv");
  ok = CountLines(dir / "history.csv") == 501 && ok;
  if (CountLines(dir / "history.csv") != 501) {
    Expect(false, "history.csv: line count != max_iter + 1");
  }
  ok = CountLines(dir / "snapshots.csv") == 6 * 50 && ok;
  if (CountLines(dir / "snapshots.csv") != 6 * 50) {
    Expect(false, "snapshots.csv: line count != snaps*agents");
  }
  std::ifstream hist{dir / "history.csv"};
  std::string first;
  std::getline(hist, first);
  const double back{std::stod(first.substr(0, first.find(',')))};
  ok = back == six.history[0].best_so_far && ok;
  if (back != six.history[0].best_so_far) {
    Expect(false, "history.csv: best_so_far does not round-trip");
  }
  std::error_code ec;
  std::filesystem::remove_all(dir, ec);

  return ok;
}
