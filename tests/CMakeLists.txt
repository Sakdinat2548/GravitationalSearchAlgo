add_executable(gsa_test
    test_main.cpp
    tasks/test_history.cpp
    tasks/test_stats.cpp
    tasks/test_determinism.cpp
    tasks/test_modes.cpp
    tasks/test_thread_safety.cpp
    tasks/test_median.cpp
    tasks/test_convergence.cpp
    tasks/test_validation.cpp
    tasks/test_json_io.cpp)
target_link_libraries(gsa_test PRIVATE gsa::gsa)
target_compile_options(gsa_test PRIVATE -Wall -Wextra)

add_test(NAME gsa_history COMMAND gsa_test history_size)
add_test(NAME gsa_stats COMMAND gsa_test history_stats)
add_test(NAME gsa_determinism COMMAND gsa_test determinism)
add_test(NAME gsa_modes COMMAND gsa_test modes)
add_test(NAME gsa_thread_safety COMMAND gsa_test thread_safety)
add_test(NAME gsa_median COMMAND gsa_test median_correctness)
add_test(NAME gsa_convergence COMMAND gsa_test convergence)
add_test(NAME gsa_validation COMMAND gsa_test validation)
add_test(NAME gsa_json_io_config COMMAND gsa_test json_io_config)
add_test(NAME gsa_json_io_bounds COMMAND gsa_test json_io_bounds)
