"""Shared benchmark objectives: the ONE place to change the formula.

Mirrors examples/objective.hpp (C++ keeps its own copy; same formula,
same bounds). Each function takes a list of floats, returns a float
(minimization).
"""


def sphere(x):
    return sum(v * v for v in x)


def rosenbrock(x):
    s = 0.0
    for i in range(len(x) - 1):
        d = x[i + 1] - x[i] * x[i]
        s += 100.0 * d * d + (x[i] - 1.0) ** 2
    return s


FUNCTIONS = {"sphere": sphere, "rosenbrock": rosenbrock}
