#ifndef GSA_TEST_FRAMEWORK_HPP
#define GSA_TEST_FRAMEWORK_HPP

#include <iostream>
#include <string>
#include <vector>

namespace gsa_test {

struct Test {
    const char* name;
    bool (*fn)();
};

inline std::vector<Test>& registry() {
    static std::vector<Test> tests;
    return tests;
}

struct Registrar {
    Registrar(const char* name, bool (*fn)()) {
        registry().push_back({name, fn});
    }
};

inline int failures = 0;

inline void expect(bool cond, const char* msg) {
    if (!cond) {
        ++failures;
        std::cout << "  FAIL: " << msg << "\n";
    }
}

inline bool run(int argc, char** argv) {
    int passed = 0, failed = 0;
    for (const auto& t : registry()) {
        if (argc > 1 && std::string(argv[1]) != t.name) continue;
        failures = 0;
        const bool ok = t.fn();
        std::cout << (ok ? "[PASS] " : "[FAIL] ") << t.name << "\n";
        ok ? ++passed : ++failed;
    }
    if (argc > 1 && passed + failed == 0) {
        std::cout << "no test named '" << argv[1] << "'\n";
        return false;
    }
    return failed == 0;
}

}  // namespace gsa_test

#define TEST(name)                                                  \
    static bool test_##name();                                      \
    static ::gsa_test::Registrar reg_##name(#name, &test_##name);   \
    static bool test_##name()

#endif  // GSA_TEST_FRAMEWORK_HPP
