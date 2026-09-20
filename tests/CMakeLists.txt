add_executable(gsa_test
    tasks/test_history.cpp
    tasks/test_stats.cpp
    tasks/test_determinism.cpp
    tasks/test_modes.cpp
    tasks/test_thread_safety.cpp
    tasks/test_median.cpp
    tasks/test_convergence.cpp
    tasks/test_validation.cpp
    tasks/test_json_io.cpp
    tasks/test_snapshots.cpp)
target_link_libraries(gsa_test PRIVATE gsa::gsa GTest::gtest_main)
target_compile_options(gsa_test PRIVATE -Wall -Wextra)

add_test(NAME gsa_history COMMAND gsa_test --gtest_filter=History.SizeAndInvariants)
add_test(NAME gsa_stats COMMAND gsa_test --gtest_filter=History.StatsFinite)
add_test(NAME gsa_determinism COMMAND gsa_test --gtest_filter=Determinism.SameSeedIdentical)
add_test(NAME gsa_modes COMMAND gsa_test --gtest_filter=Modes.MinimizeMaximizeAndEdgeSizes)
add_test(NAME gsa_thread_safety COMMAND gsa_test --gtest_filter=ThreadSafety.ConcurrentOptimizeIdentical)
add_test(NAME gsa_median COMMAND gsa_test --gtest_filter=FitnessStats.MedianCorrectness)
add_test(NAME gsa_convergence COMMAND gsa_test --gtest_filter=Convergence.SphereBelowThreshold)
add_test(NAME gsa_validation COMMAND gsa_test --gtest_filter=Validation.RejectsBadConfig)
add_test(NAME gsa_json_io_config COMMAND gsa_test --gtest_filter=JsonIo.ConfigDefaultsAndErrors)
add_test(NAME gsa_json_io_bounds COMMAND gsa_test --gtest_filter=JsonIo.BoundsParsingAndErrors)
add_test(NAME gsa_snapshots COMMAND gsa_test --gtest_filter=Snapshots.CaptureAndCsvRoundTrip)
