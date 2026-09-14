// Unit tests for BigInt.
//
// bigint.cpp is a single translation unit with its own demo main(), so the
// tests define BIGINT_NO_MAIN and include it directly. That keeps the
// documented build command (g++ -std=c++17 -O2 bigint.cpp -o bigint) working
// unchanged while still giving the class a real test harness.

#include <sstream>
#include <string>

#define BIGINT_NO_MAIN
#include "bigint.cpp"

static int failures = 0;
static int checks = 0;

static void check(bool ok, const std::string &name, const std::string &got,
                  const std::string &want) {
    ++checks;
    if (ok) {
        std::cout << "ok   " << name << "\n";
    } else {
        ++failures;
        std::cout << "FAIL " << name << "\n       got:  " << got
                  << "\n       want: " << want << "\n";
    }
}

// Render a BigInt through operator<< (full digits up to 12, scientific above).
static std::string str(const BigInt &b) {
    std::ostringstream os;
    os << b;
    return os.str();
}

static void expectStr(const BigInt &b, const std::string &want,
                      const std::string &name) {
    std::string got = str(b);
    check(got == want, name, got, want);
}

static void expectInt(int got, int want, const std::string &name) {
    check(got == want, name, std::to_string(got), std::to_string(want));
}

static void expectTrue(bool got, const std::string &name) {
    check(got, name, got ? "true" : "false", "true");
}

// collatz() writes its trace to cout; swallow it so test output stays readable.
static BigInt quietCollatz(const BigInt &n) {
    std::ostringstream sink;
    std::streambuf *old = std::cout.rdbuf(sink.rdbuf());
    BigInt steps = n.collatz(false);
    std::cout.rdbuf(old);
    return steps;
}

static void testConstruction() {
    expectStr(BigInt(), "0", "default constructor is zero");
    expectStr(BigInt(25), "25", "construct from int");
    expectStr(BigInt("25"), "25", "construct from string");
    expectStr(BigInt(0), "0", "construct zero from int");
    expectStr(BigInt("0"), "0", "construct zero from string");
    expectStr(BigInt("000123"), "123", "leading zeros are trimmed");
    expectStr(BigInt("   42"), "42", "leading spaces are skipped");
    expectStr(BigInt("\t\n7"), "7", "leading tab and newline are skipped");
    expectStr(BigInt("00000"), "0", "all-zero string is zero");
    expectStr(BigInt(-5), "0", "negative int clamps to zero (positive-only)");

    BigInt n(1234);
    BigInt copy(n);
    expectStr(copy, "1234", "copy constructor");
    expectTrue(copy == n, "copy compares equal to original");
}

static void testAddition() {
    expectStr(BigInt(25) + BigInt(10), "35", "25 + 10");
    expectStr(BigInt(10) + BigInt(25), "35", "10 + 25 (commutes)");
    expectStr(BigInt(0) + BigInt(0), "0", "0 + 0");
    expectStr(BigInt(999) + BigInt(1), "1000", "carry ripples across digits");
    expectStr(BigInt("99999999999") + BigInt(1), "100000000000",
              "carry grows the number by a digit");
    expectStr(10 + BigInt(25), "35", "int + BigInt free function");
    expectStr(BigInt("123456789") + BigInt(0), "123456789", "adding zero");
}

static void testSubtraction() {
    expectStr(BigInt(35) - BigInt(10), "25", "35 - 10");
    expectStr(BigInt(1000) - BigInt(1), "999", "borrow ripples across digits");
    expectStr(BigInt(25) - BigInt(25), "0", "x - x is zero");
    expectStr(BigInt(100) - 1, "99", "subtracting a plain int");
    expectStr(BigInt("100000000000") - BigInt(1), "99999999999",
              "borrow shrinks the number by a digit");
}

static void testMultiplication() {
    expectStr(BigInt(25) * BigInt(4), "100", "25 * 4");
    expectStr(BigInt(1234) * BigInt(0), "0", "anything * 0 is 0");
    expectStr(BigInt(1234) * BigInt(1), "1234", "anything * 1 is itself");
    expectStr(BigInt(999) * BigInt(999), "998001", "999 * 999");
    // 1234 * 9223372036854775807 == 11381641093478793345838
    expectStr(BigInt("1234") * BigInt("9223372036854775807"), "1.138164e22",
              "1234 * INT64_MAX in scientific form");
    expectStr(BigInt("9223372036854775807") * BigInt("1234"), "1.138164e22",
              "multiplication commutes on large operands");
}

static void testDivision() {
    expectStr(BigInt(1234) / BigInt(25), "49", "1234 / 25");
    expectStr(BigInt(1234) % BigInt(25), "9", "1234 % 25");
    expectStr(BigInt(100) / BigInt(10), "10", "exact division");
    expectStr(BigInt(100) % BigInt(10), "0", "exact division leaves no remainder");
    expectStr(BigInt(7) / BigInt(10), "0", "quotient smaller than divisor is 0");
    expectStr(BigInt(7) % BigInt(10), "7", "remainder when quotient is 0");
}

static void testComparison() {
    expectTrue(BigInt(25) == BigInt("25"), "int and string forms compare equal");
    expectTrue(BigInt(25) != BigInt(26), "different values are not equal");
    expectTrue(BigInt(25) < BigInt(26), "25 < 26");
    expectTrue(BigInt(99) < BigInt(100), "fewer digits compares smaller");
    expectTrue(!(BigInt(100) < BigInt(99)), "more digits compares larger");
    expectTrue(!(BigInt(25) < BigInt(25)), "equal values are not less than");
    expectTrue(BigInt("000025") == BigInt(25), "trimmed zeros compare equal");
}

static void testIncrement() {
    BigInt n(25);
    expectStr(n++, "25", "postfix ++ returns the old value");
    expectStr(n, "26", "postfix ++ leaves the incremented value");

    BigInt s(25);
    expectStr(++s, "26", "prefix ++ returns the new value");
    expectStr(s, "26", "prefix ++ leaves the incremented value");

    BigInt nine(99);
    ++nine;
    expectStr(nine, "100", "increment carries across digits");
}

static void testIndexAndSize() {
    BigInt n(1234); // digits stored least significant first
    expectStr(n[0], "4", "index 0 is the least significant digit");
    expectStr(n[3], "1", "index 3 is the most significant digit");
    expectStr(n[9], "0", "out-of-range index returns zero");
    expectStr(n[-1], "0", "negative index returns zero");
    expectInt(n.size(), 4, "size counts digits");
    expectInt(BigInt(0).size(), 1, "zero has one digit");
}

static void testStreamFormatting() {
    // 12 digits or fewer print in full, 13 or more switch to scientific.
    expectStr(BigInt("999999999999"), "999999999999", "12 digits print in full");
    expectStr(BigInt("1000000000000"), "1.000000e12",
              "13 digits switch to scientific notation");
    expectStr(BigInt("9223372036854775807"), "9.223372e18",
              "INT64_MAX in scientific notation");

    // print() bypasses the scientific-notation crossover and always emits
    // every digit, so capture cout to check it.
    std::ostringstream os;
    std::streambuf *old = std::cout.rdbuf(os.rdbuf());
    BigInt("9223372036854775807").print();
    std::cout.rdbuf(old);
    check(os.str() == "9223372036854775807", "print() always emits full digits",
          os.str(), "9223372036854775807");
}

static void testFibonacci() {
    expectStr(BigInt(0).fibo(), "0", "fibo(0)");
    expectStr(BigInt(1).fibo(), "1", "fibo(1)");
    expectStr(BigInt(12).fibo(), "144", "fibo(12)");
    expectStr(BigInt(50).fibo(), "12586269025", "fibo(50)");
    expectStr(BigInt(90).fibo(), "2.880067e18", "fibo(90)");
    expectStr(BigInt(3000).fibo(), "4.106158e626", "fibo(3000)");
}

static void testFactorial() {
    expectStr(BigInt(0).fact(), "1", "fact(0) is 1");
    expectStr(BigInt(1).fact(), "1", "fact(1) is 1");
    expectStr(BigInt(5).fact(), "120", "fact(5)");
    expectStr(BigInt(20).fact(), "2.432902e18", "fact(20)");
    expectStr(BigInt(50).fact(), "3.041409e64", "fact(50)");
}

static void testCollatz() {
    expectStr(quietCollatz(BigInt(1)), "0", "collatz(1) takes no steps");
    expectStr(quietCollatz(BigInt(2)), "1", "collatz(2) takes one step");
    expectStr(quietCollatz(BigInt(27)), "111", "collatz(27) takes 111 steps");
    expectStr(quietCollatz(BigInt(123)), "46", "collatz(123) takes 46 steps");
    // Reference value from the Collatz conjecture literature.
    expectStr(quietCollatz(BigInt("989345275647")), "1348",
              "collatz(989345275647) takes 1348 steps");
    expectStr(quietCollatz(BigInt(3000).fibo()), "15322",
              "collatz(fibo(3000)) takes 15322 steps");
}

int main() {
    testConstruction();
    testAddition();
    testSubtraction();
    testMultiplication();
    testDivision();
    testComparison();
    testIncrement();
    testIndexAndSize();
    testStreamFormatting();
    testFibonacci();
    testFactorial();
    testCollatz();

    std::cout << "\n" << (checks - failures) << "/" << checks << " checks passed\n";
    return failures == 0 ? 0 : 1;
}
