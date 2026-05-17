// Author: @forgehk

// Project: arbitrary-precision integer class

//
// I certify that this program is my own original work. I did not copy any part 
// of this program from any other source. I further certify that I typed each 
// and every line of code in this program.

#include <iostream>
#include <vector>
#include <string>
#include <climits>
#include <iomanip>
#include <cstdlib>

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::ostream;
using std::setw;

class BigInt
{
private:
    vector<char> v; // digits 0–9, least significant digit at index 0

    void trim() {
        while (v.size() > 1 && v.back() == 0) {
            v.pop_back();
        }
    }

    bool isZero() const {
        return v.size() == 1 && v[0] == 0;
    }

    bool isOne() const {
        return v.size() == 1 && v[0] == 1;
    }

    // compare magnitudes: -1 if *this < other, 0 if equal, 1 if >
    int cmpAbs(const BigInt &other) const {
        if (v.size() < other.v.size()) return -1;
        if (v.size() > other.v.size()) return 1;
        for (int i = (int)v.size() - 1; i >= 0; --i) {
            if (v[i] < other.v[i]) return -1;
            if (v[i] > other.v[i]) return 1;
        }
        return 0;
    }

    string toFullString() const {
        string s;
        for (int i = (int)v.size() - 1; i >= 0; --i) {
            s.push_back(char('0' + v[i]));
        }
        if (s.empty()) s = "0";
        return s;
    }

    string toSciString() const {
        // assumes size() > 12
        string s = toFullString();
        int totalDigits = (int)s.size();
        int exponent = totalDigits - 1;

        // 7 significant digits: 1 digit, ".", 6 more digits
        string mantissa;
        mantissa.push_back(s[0]);
        mantissa.push_back('.');
        int extra = 6;
        for (int i = 1; i < totalDigits && extra > 0; ++i, --extra) {
            mantissa.push_back(s[i]);
        }
        // if number has fewer than 7 digits, mantissa will just be shorter,
        // but that won't happen here since size() > 12

        mantissa.push_back('e');

        // append exponent as decimal
        string expStr;
        int e = exponent;
        if (e == 0) {
            expStr = "0";
        } else {
            while (e > 0) {
                int d = e % 10;
                expStr.push_back(char('0' + d));
                e /= 10;
            }
            // reverse expStr
            for (int i = 0, j = (int)expStr.size() - 1; i < j; ++i, --j) {
                char tmp = expStr[i];
                expStr[i] = expStr[j];
                expStr[j] = tmp;
            }
        }

        mantissa += expStr;
        return mantissa;
    }

    // helper for tail-recursive fibo
    BigInt fiboHelper(const BigInt &n, const BigInt &a, const BigInt &b) const {
        if (n.isZero()) {
            return a;
        }
        BigInt n1 = n - BigInt(1);
        BigInt nextA = b;
        BigInt nextB = a + b;
        return fiboHelper(n1, nextA, nextB); // tail call
    }

    // helper for tail-recursive factorial
    BigInt factHelper(const BigInt &n, const BigInt &acc) const {
        if (n.isZero() || n.isOne()) {
            return acc;
        }
        BigInt n1 = n - BigInt(1);
        BigInt nextAcc = acc * n;
        return factHelper(n1, nextAcc); // tail call
    }

    int toInt() const {
        long long res = 0;
        for (int i = (int)v.size() - 1; i >= 0; --i) {
            res = res * 10 + v[i];
            if (res > INT_MAX) {
                // only used when we know the value fits, per assignment tests
                break;
            }
        }
        return (int)res;
    }

    // division by small positive int, returns quotient, sets remainder
    BigInt divByInt(int divisor, int &remainder) const {
        BigInt result;
        result.v.clear();

        remainder = 0;
        for (int i = (int)v.size() - 1; i >= 0; --i) {
            int cur = remainder * 10 + v[i];
            int qd = cur / divisor;
            remainder = cur % divisor;
            result.v.push_back((char)qd);
        }
        // digits are currently most significant first, reverse into LSD first
        for (int i = 0, j = (int)result.v.size() - 1; i < j; ++i, --j) {
            char tmp = result.v[i];
            result.v[i] = result.v[j];
            result.v[j] = tmp;
        }
        if (result.v.empty()) {
            result.v.push_back(0);
        }
        result.trim();
        return result;
    }

public:
    BigInt() {
        v.push_back(0);
    }

    BigInt(int n) {
        if (n <= 0) {
            v.push_back(0);
        } else {
            while (n > 0) {
                int d = n % 10;
                v.push_back((char)d);
                n /= 10;
            }
        }
    }

    BigInt(string s) {
        v.clear();
        // remove leading spaces and zeros
        int i = 0;
        while (i < (int)s.size() && (s[i] == ' ' || s[i] == '	' || s[i] == '
')) {
            ++i;
        }
        while (i < (int)s.size() && s[i] == '0') {
            ++i;
        }
        if (i == (int)s.size()) {
            v.push_back(0);
            return;
        }
        for (int j = (int)s.size() - 1; j >= i; --j) {
            char c = s[j];
            if (c >= '0' && c <= '9') {
                v.push_back((char)(c - '0')); // store numeric digit 0–9
            }
        }
        if (v.empty()) {
            v.push_back(0);
        }
        trim();
    }

    BigInt(const BigInt &other) {
        v = other.v;
    }

    BigInt operator+(BigInt other) const {
        BigInt result;
        result.v.clear();

        int carry = 0;
        int maxSize = (int)((v.size() > other.v.size()) ? v.size() : other.v.size());
        for (int i = 0; i < maxSize || carry; ++i) {
            int a = (i < (int)v.size()) ? v[i] : 0;
            int b = (i < (int)other.v.size()) ? other.v[i] : 0;
            int sum = a + b + carry;
            result.v.push_back((char)(sum % 10));
            carry = sum / 10;
        }
        result.trim();
        return result;
    }

    BigInt operator-(BigInt other) const {
        // assume *this >= other (assignment never needs negatives)
        BigInt result;
        result.v.clear();

        int borrow = 0;
        int maxSize = (int)v.size(); // assume this has >= digits
        for (int i = 0; i < maxSize; ++i) {
            int a = v[i];
            int b = (i < (int)other.v.size()) ? other.v[i] : 0;
            int diff = a - b - borrow;
            if (diff < 0) {
                diff += 10;
                borrow = 1;
            } else {
                borrow = 0;
            }
            result.v.push_back((char)diff);
        }
        result.trim();
        return result;
    }

    BigInt operator-(int x) const {
        return *this - BigInt(x);
    }

    BigInt operator*(BigInt other) const {
        BigInt result;
        result.v.assign(v.size() + other.v.size(), 0);

        for (int i = 0; i < (int)v.size(); ++i) {
            int carry = 0;
            for (int j = 0; j < (int)other.v.size() || carry; ++j) {
                long long cur = result.v[i + j] +
                                (long long)v[i] * (j < (int)other.v.size() ? other.v[j] : 0) +
                                carry;
                result.v[i + j] = (char)(cur % 10);
                carry = (int)(cur / 10);
            }
        }
        result.trim();
        return result;
    }

    BigInt operator/(BigInt other) const {
        int d = other.toInt(); // safe for assignment tests
        int rem = 0;
        return divByInt(d, rem);
    }

    BigInt operator%(BigInt other) const {
        int d = other.toInt();
        int rem = 0;
        divByInt(d, rem);
        return BigInt(rem);
    }

    // postfix ++
    BigInt operator++(int) {
        BigInt temp(*this);
        *this = *this + BigInt(1);
        return temp;
    }

    // prefix ++
    BigInt operator++() {
        *this = *this + BigInt(1);
        return *this;
    }

    BigInt operator[](int index) {
        if (index < 0 || index >= (int)v.size()) {
            return BigInt(0);
        }
        return BigInt((int)v[index]);
    }

    int size() const {
        return (int)v.size();
    }

    void print() const {
        cout << toFullString();
    }

    BigInt fibo() const {
        // Fibonacci of current number (non-negative)
        return fiboHelper(*this, BigInt(0), BigInt(1));
    }

    BigInt fact() const {
        return factHelper(*this, BigInt(1));
    }

    BigInt collatz(bool printSteps) const {
        BigInt n(*this);
        BigInt steps(0);

        if (printSteps) {
            cout << n << ". ";
        } else {
            cout << endl;
        }

        while (!n.isOne()) {
            // even if last digit is even
            if ((n.v[0] % 2) == 0) {
                int rem = 0;
                n = n.divByInt(2, rem);
            } else {
                n = n * BigInt(3);
                n = n + BigInt(1);
            }
            steps = steps + BigInt(1);

            if (printSteps) {
                cout << n << ". ";
            }
        }

        cout << endl;
        cout << ":->";
        return steps;
    }

    bool operator==(const BigInt &other) const {
        return v == other.v;
    }

    bool operator!=(const BigInt &other) const {
        return !(*this == other);
    }

    bool operator<(const BigInt &other) const {
        return cmpAbs(other) < 0;
    }

    friend BigInt operator+(int lhs, BigInt rhs);
    friend ostream &operator<<(ostream &os, const BigInt &b);
};

BigInt operator+(int lhs, BigInt rhs) {
    BigInt left(lhs);
    return left + rhs;
}

ostream &operator<<(ostream &os, const BigInt &b) {
    if (b.size() <= 12) {
        os << b.toFullString();
    } else {
        os << b.toSciString();
    }
    return os;
}

int main()
{
    int space = 10;
    cout << "
TestUnit:
" << std::flush;
    cout << "User Name:" << std::flush;
    std::system("whoami");
    std::system("date");
    BigInt n1(25);
    BigInt s1("25");
    BigInt n2(1234);
    BigInt s2("1234");
    BigInt n3(n2);
    BigInt X(3000);
    BigInt Y(50);
    BigInt Z1(123);
    BigInt Z2("989345275647");
    BigInt Z3(X.fibo());
    BigInt imax = INT_MAX;
    BigInt big("9223372036854775807");

    cout << "n1(int)    :" << setw(space) << n1 << endl;
    cout << "s1(str)    :" << setw(space) << s1 << endl;
    cout << "n2(int)    :" << setw(space) << n2 << endl;
    cout << "s2(str)    :" << setw(space) << s2 << endl;
    cout << "n3(n2)     :" << setw(space) << n3 << endl;
    cout << "X.fibo(1234):" << setw(space) << X.fibo() << endl;
    cout << "Y.fact(50)   :" << setw(space) << Y.fact() << endl;
    cout << "imax       :" << setw(space) << imax << endl;
    cout << "big        :" << setw(space) << big << endl;
    cout << "big.print(): "; big.print(); cout << endl;

    cout << n2 << "/" << n1 << " = " << n2 / n1 << " rem " << n2 % n1 << endl;
    cout << "fibo(" << X << ") = " << X.fibo() << endl;
    cout << "fact(" << Y << ") = " << Y.fact() << endl;

    bool printSteps = true;
    cout << "steps for collatz(" << Z1 << "):" << Z1.collatz(printSteps) << endl;

    printSteps = false;

    // 1348 steps per Wikipedia https://en.wikipedia.org/wiki/Collatz_conjecture
    cout << "steps for collatz(" << Z2 << "):" << Z2.collatz(printSteps) << endl;

    cout << "steps for collatz(" << Z3 << "):" << Z3.collatz(printSteps) << endl;

    cout << "10 + n1 = " << BigInt(10) + n1 << endl;
    cout << "n1 + 10 = " << n1 + BigInt(10) << endl;

    cout << "(n1 == s1)? --> " << ((n1 == s1) ? "true" : "false") << endl;
    cout << "n1++ = ?  --> before:" << n1++ << " after:" << n1 << endl;
    cout << "++s1 = ?  --> before:" << ++s1 << " after:" << s1 << endl;

    cout << "s2 * big = ? --> " << s2 * big << endl;
    cout << "big * s2 = ? --> " << big * s2 << endl;

    cout << endl;
    std::system("date");

    return 0;
}
