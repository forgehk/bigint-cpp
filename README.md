# bigint-cpp

> Arbitrary-precision integer class in modern C++ — addition, subtraction, multiplication, division, factorial, Fibonacci, and the Collatz conjecture, all with operator overloading.

[![C++](https://img.shields.io/badge/C++-17-blue.svg)]() [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

---

## What this is

A `BigInt` class that handles integers larger than any built-in type.

The point isn't to ship a Boost-quality library — it's to demonstrate competent **operator overloading**, **digit-level arithmetic**, **tail-recursive algorithm design**, and a careful eye for **edge cases** (zero, one-digit, multi-digit overflow carries, scientific notation crossover at 12+ digits).

---

## Features

| Capability | Notes |
|---|---|
| Construct from `int` | `BigInt n(42);` |
| Construct from `std::string` | `BigInt s("9223372036854775807");` |
| Copy construct | `BigInt n3(n2);` |
| `operator+` | Digit-wise add with carry |
| `operator-` | Digit-wise subtract with borrow (positive results only — assignment spec) |
| `operator*` | School-method multiplication with carry propagation |
| `operator/`, `operator%` | Division by a `BigInt` that fits in `int` (assignment-bounded) |
| Prefix & postfix `operator++` | Proper temp semantics |
| `operator[]` | Returns the digit at index `i` as a `BigInt` |
| `operator==`, `!=`, `<` | Magnitude comparison |
| `operator<<` | Stream insertion — prints full digits for ≤12, switches to `1.234567e15` scientific notation above that |
| `.fibo()` | **Tail-recursive** Fibonacci |
| `.fact()` | **Tail-recursive** factorial |
| `.collatz(bool)` | Walks the 3n+1 sequence, optionally printing each step, returns step count |

---

## Sample output

```
n1(int)    :        25
s1(str)    :        25
n2(int)    :      1234
imax       :2147483647
big        :9.223372e18
1234/25 = 49 rem 9
fact(50) = 3.041409e64
steps for collatz(989345275647) = 1348
```

(1348 steps for `n = 989345275647` matches Wikipedia's reference value for the Collatz conjecture.)

---

## Build & run

```bash
g++ -std=c++17 -O2 bigint.cpp -o bigint
./bigint
```

No external dependencies. Pure standard library.

---

## Implementation notes

- **Digit storage:** `std::vector<char>` with **least-significant digit at index 0**. Makes carry propagation a one-direction loop.
- **Leading-zero trim:** Every mutating op calls `trim()` so equality and comparison stay correct.
- **Tail recursion:** `fiboHelper(n, a, b)` and `factHelper(n, acc)` are written so the recursive call is in tail position. (Modern C++ compilers don't guarantee TCO, but the shape is correct for any compiler that does, and depth is `O(n)` either way.)
- **Scientific output:** Crosses over at >12 digits, prints 7 significant digits (`d.dddddde<exp>`).
- **Collatz:** Uses `divByInt(2, remainder)` instead of full BigInt division when the number is even — much cheaper. `3n+1` reuses regular multiplication and addition.

---

## What I'd add next

- Negative numbers (sign field) — currently positive-only per assignment spec.
- Karatsuba or FFT multiplication for very large operands (`O(n^1.58)` or `O(n log n)` vs the current `O(n²)`).
- A proper unit test suite (currently driven by `main()`'s test harness).
- Header/implementation split for use as a real library.

---

## License

[MIT](LICENSE)

---

*Built by [@forgehk](https://github.com/forgehk) — [DarkForge AI](https://darkforgeai.com)*
