# How the test "wizardry" works

This directory is a tiny, dependency-free C++ test framework. There is no gtest, no
CMake test discovery — just a handful of C++ language features working together so that:

- each test lives in its own `.cpp` file,
- a new test is picked up **automatically** (no central list to edit),
- CTest can run each one as a separate test.

## Files

| File | Role |
|------|------|
| `test_framework.hpp` | The engine: registration + runner (`TEST` macro, `expect`, `run`). |
| `test_common.hpp` | Shared objectives (sphere/rosenbrock/Shekel) and invariant helpers. |
| `test_main.cpp` | The only `main()`; calls `gsa_test::run(argc, argv)`. |
| `tasks/` | One `test_*.cpp` per test, written as `TEST(name) { ... }`. |
| `TEST.md` | This file. |

## The big idea: tests register themselves

Each `TEST(...)` creates a **global static object**. In C++, a global/static object's
constructor runs automatically *before* `main()` (static initialization). So every test,
from every `.cpp`, "calls in" at startup and adds itself to a shared list. When `main()`
finally runs, the list is already full.

## Piece by piece

### 1. `struct Test` — what a test is

```cpp
struct Test {
    const char* name;   // e.g. "history_size"
    bool (*fn)();       // pointer to a function returning bool (true = passed)
};
```

Just a pair: a name + a function pointer to the test body.

### 2. `registry()` — the one shared list

```cpp
inline std::vector<Test>& registry() {
    static std::vector<Test> tests;   // created once, on first call
    return tests;                     // returned by reference
}
```

- The `static` local is created **exactly once** and lives for the whole program.
- Returning **by reference** (`&`) means every caller touches the *same* vector.
- `inline` lets this same function be defined in every translation unit without a link
  error (ODR). All files therefore share one vector.
- Using a function-local `static` instead of a plain global avoids the *static
  initialization order fiasco*: the vector is guaranteed built on first use, regardless
  of the order static objects construct.

### 3. `Registrar` — the auto-enroller

```cpp
struct Registrar {
    Registrar(const char* name, bool (*fn)()) {
        registry().push_back({name, fn});
    }
};
```

Its constructor does one thing: append `{name, fn}` to the registry. The "magic" is that
a `static Registrar` object constructed at startup triggers this automatically.

### 4. The `TEST(name)` macro — the glue

```cpp
#define TEST(name)                                                  \
    static bool test_##name();                                      \
    static ::gsa_test::Registrar reg_##name(#name, &test_##name);   \
    static bool test_##name()
```

The preprocessor rewrites `TEST(history_size) { ... }` into:

```cpp
static bool test_history_size();                                    // fwd decl
static ::gsa_test::Registrar reg_history_size("history_size", &test_history_size); // register
static bool test_history_size() { ... }                             // the body
```

- `##` **token-pastes**: `test_##name` → `test_history_size`.
- `#name` **stringizes**: `"history_size"`.

Line 2 is the key: it defines a `static` object whose constructor (at startup) registers
the test. Nothing else needs to know the test exists.

### 5. `expect()` — the assertion

```cpp
inline int failures{};   // shared counter, also `inline` so it's one global

inline void expect(bool cond, const char* msg) {
    if (!cond) { ++failures; std::cout << "  FAIL: " << msg << "\n"; }
}
```

Unlike `assert`, this is **not disabled** in Release (`-DNDEBUG`) — so it always runs.

### 6. `run()` — the dispatcher

```cpp
inline bool run(int argc, char** argv) {
    int passed{}, failed{};
    for (const auto& t : registry()) {           // iterate every registered test
        if (argc > 1 && std::string(argv[1]) != t.name) continue;  // optional filter
        failures = 0;                             // reset per test
        const bool ok = t.fn();                   // call the test body
        std::cout << (ok ? "[PASS] " : "[FAIL] ") << t.name << "\n";
        ok ? ++passed : ++failed;
    }
    if (argc > 1 && passed + failed == 0) {       // filtered to a name that doesn't exist
        std::cout << "no test named '" << argv[1] << "'\n";
        return false;
    }
    return failed == 0;                           // non-zero exit on any failure
}
```

`argv[1]` is the optional test-name filter; `argc`/`argv` come from the command line
(`argc` = count, `argv[0]` = program name, args start at `argv[1]`).

## Putting it together

```
program start
   │
   ├─ static Registrar objects construct  ──►  registry() gets {name, fn} entries
   │   (one per TEST, across all .cpp files)       (vector is now populated)
   ▼
main() ──► run(argc, argv) ──► loop over registry() ──► call each test ──► exit code
```

## How CTest uses it

`CMakeLists.txt` registers each test as its own CTest entry, calling the same binary with
a different test name as `argv[1]`:

```cmake
add_test(NAME gsa_history         COMMAND gsa_test history_size)
add_test(NAME gsa_stats           COMMAND gsa_test history_stats)
add_test(NAME gsa_determinism     COMMAND gsa_test determinism)
add_test(NAME gsa_modes           COMMAND gsa_test modes)
add_test(NAME gsa_thread_safety   COMMAND gsa_test thread_safety)
```

CTest runs the binary once per test, each filtered to one test, and treats the exit code as
pass/fail. Run them with `ctest --preset default`.

## Why `inline` matters everywhere

The `inline` keywords on `registry()`, `failures`, `expect()`, `run()`, and the
`test_common.hpp` helpers are what let a header be included by many `.cpp` files while
guaranteeing there is **one** function / **one** `failures` / **one** registry vector
program-wide. Omit `inline` and you get linker "multiple definition" errors.

## The three language tricks, summarized

| Feature | What it does here |
|---------|-------------------|
| Global static object ctor runs before `main` | Tests self-register at startup |
| Function-local `static` + return-by-reference | One shared registry vector, init-order safe |
| Preprocessor `##` (token paste) / `#` (stringize) | Generate unique function/registrar names from the test name |
| `inline` | Single definition shared across translation units |

That's the whole trick — no frameworks needed.
