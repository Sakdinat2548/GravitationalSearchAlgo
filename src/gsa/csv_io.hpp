#ifndef GSA_CSV_IO_HPP
#define GSA_CSV_IO_HPP

#include <cstddef>
#include <filesystem>
#include <format>
#include <fstream>
#include <stdexcept>

#include "gsa/gsa.hpp"

namespace gsa {

inline void WriteHistoryCsv(const GsaResult& result,
                            const std::filesystem::path& path) {
  std::ofstream out{path};
  if (!out) {
    throw std::runtime_error(
        std::format("cannot open '{}' for writing", path.string()));
  }
  out << "best_so_far,best_iter,worst_iter,mean_fitness,median_fitness,"
         "stddev_fitness\n";
  for (const auto& it : result.history) {
    out << std::format("{},{},{},{},{},{}\n", it.best_so_far, it.best_iter,
                       it.worst_iter, it.mean_fitness, it.median_fitness,
                       it.stddev_fitness);
  }
  out.flush();
  if (!out) {
    throw std::runtime_error(
        std::format("failed writing '{}'", path.string()));
  }
}

inline void WriteSnapshotsCsv(const GsaResult& result,
                               const std::filesystem::path& path) {
  const size_t dims{result.snapshot_dims};
  const size_t snaps{result.snapshot_iters.size()};
  const size_t masses{result.snapshot_masses.size()};
  if (snaps == 0) {
    if (!result.snapshot_positions.empty() || masses != 0 ||
        !result.snapshot_fitnesses.empty()) [[unlikely]] {
      throw std::invalid_argument(
          "snapshot vectors must be empty when snapshot_iters is empty");
    }
  } else {
    if (dims == 0) [[unlikely]] {
      throw std::invalid_argument("snapshot_dims must be > 0 for snapshots");
    }
    if (masses == 0 || masses != result.snapshot_fitnesses.size() ||
        masses % snaps != 0 ||
        result.snapshot_positions.size() != masses * dims) [[unlikely]] {
      throw std::invalid_argument("snapshot vectors have inconsistent sizes");
    }
  }
  std::ofstream out{path};
  if (!out) {
    throw std::runtime_error(
        std::format("cannot open '{}' for writing", path.string()));
  }
  const size_t agents{snaps == 0 ? 0 : masses / snaps};
  out << "iter,agent,mass,fitness";
  for (size_t d{}; d < dims; ++d) {
    out << std::format(",x{}", d + 1);
  }
  out << '\n';
  for (size_t s{}; s < result.snapshot_iters.size(); ++s) {
    for (size_t a{}; a < agents; ++a) {
      const size_t flat{(s * agents) + a};
      out << std::format("{},{},{},{}", result.snapshot_iters[s], a,
                         result.snapshot_masses[flat],
                         result.snapshot_fitnesses[flat]);
      const size_t base{flat * dims};
      for (size_t d{}; d < dims; ++d) {
        out << std::format(",{}", result.snapshot_positions[base + d]);
      }
      out << '\n';
    }
  }
  out.flush();
  if (!out) {
    throw std::runtime_error(
        std::format("failed writing '{}'", path.string()));
  }
}

}  // namespace gsa

#endif
