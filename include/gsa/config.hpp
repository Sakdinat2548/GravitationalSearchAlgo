#ifndef GSA_CONFIG_HPP
#define GSA_CONFIG_HPP

#include <fstream>
#include <string>

#include "gsa/gsa.hpp"
#include <nlohmann/json.hpp>

namespace gsa {

// Load GsaConfig from nlohmann::json object
inline GsaConfig LoadConfigFromJson(const nlohmann::json& j) {
  GsaConfig cfg;
  if (j.contains("n_agents")) cfg.n_agents = j["n_agents"].get<size_t>();
  if (j.contains("max_iter")) cfg.max_iter = j["max_iter"].get<size_t>();
  if (j.contains("g0")) cfg.g0 = j["g0"].get<double>();
  if (j.contains("alpha")) cfg.alpha = j["alpha"].get<double>();
  if (j.contains("minimize")) cfg.minimize = j["minimize"].get<bool>();
  if (j.contains("seed")) cfg.seed = j["seed"].get<uint64_t>();
  return cfg;
}

// Load GsaConfig from JSON file
inline GsaConfig LoadConfigFromFile(const std::string& path) {
  std::ifstream file(path);
  if (!file) throw std::invalid_argument("Cannot open config file: " + path);
  nlohmann::json j;
  file >> j;
  return LoadConfigFromJson(j);
}

// Load GsaConfig from JSON string
inline GsaConfig LoadConfigFromString(const std::string& json_str) {
  nlohmann::json j = nlohmann::json::parse(json_str);
  return LoadConfigFromJson(j);
}

}  // namespace gsa

#endif  // GSA_CONFIG_HPP