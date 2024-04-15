#include "biginteger.h"
#include <iostream>
#include <vector>

BigInteger::BigInteger() : digits({0}) {}

BigInteger::BigInteger(int x) {
    isNegative = (x < 0);
    x = abs(x);
    while (x != 0) {
        digits.push_back(x % base);
        x /= base;
    }
    if (digits.empty()) {
        digits.push_back(0);
    }
}

BigInteger::BigInteger(const std::string& s) {
    int ind = static_cast<int>(s.size() - 1);
    int flag = static_cast<int>(s.front() == '-');
    isNegative = (flag != 0);
    for (; ind >= flag; ind -= len) {
        int localFlag = 0;
        if (s.substr(std::max(0, ind - len + 1),
                     std::min(ind + 1, len) - localFlag)
                .front() == '-') {
            localFlag = 1;
        }
        digits.push_back(
            std::stoi(s.substr(std::max(localFlag, ind - len + 1),
                               std::min(ind + 1, len) - localFlag)));
    }
}

const std::vector<long long int>& BigInteger::getDigits() {
    return digits;
}

const std::vector<long long int>& BigInteger::getDigits() const {
    return digits;
}

bool BigInteger::getNegative() const {
    return isNegative;
}

void BigInteger::normalize() {
    if (digits.empty()) {
        return;
    }
    for (size_t i = 0; i < digits.size() - 1; ++i) {
        digits[i + 1] += digits[i] / base;
        digits[i] = (digits[i] + base) % base;
    }
    if (digits.back() >= base) {
        digits.push_back(0);
        digits.back() += digits[digits.size() - 2] / base;
        digits[digits.size() - 2] %= base;
    }
}

BigInteger& BigInteger::operator+=(const BigInteger& second) {
    if (isNegative != second.isNegative) {
        isNegative ^= 1;
        *this -= second;
        isNegative ^= 1;
        return *this;
    }
    for (size_t i = 0; i < std::min(digits.size(), second.digits.size()); ++i) {
        digits[i] += second.digits[i];
    }
    for (size_t i = digits.size(); i < second.digits.size(); ++i) {
        digits.push_back(second.digits[i]);
    }
    normalize();
    return *this;
}

BigInteger& BigInteger::operator-=(const BigInteger& second) {
    if (isNegative != second.isNegative) {
        isNegative ^= 1;
        *this += second;
        isNegative ^= 1;
        return *this;
    }
    int len = second.digits.size() - digits.size() + 1;
    for (int i = 0; i < len; ++i) {
        digits.push_back(0);
    }
    for (size_t i = 0; i < std::max(digits.size() - 1, second.digits.size());
         ++i) {
        int val = 0;
        if (i < second.digits.size()) {
            val = second.digits[i];
        }
        if (digits[i] >= val) {
            digits[i] -= val;
            continue;
        }
        --digits[i + 1];
        digits[i] = base + digits[i] - val;
    }
    if (digits.back() == -1) {
        digits = (second - (*this + second)).getDigits();
        isNegative ^= 1;
    }
    while (digits.back() == 0 && digits.size() > 1) {
        digits.pop_back();
    }
    return *this;
}

BigInteger& BigInteger::operator*=(const BigInteger& second) {
    BigInteger answer(0);
    answer.digits.assign(digits.size() + second.digits.size(), 0);
    for (size_t j = 0; j < second.digits.size(); ++j) {
        for (size_t i = 0; i < digits.size(); ++i) {
            answer.digits[i + j] += digits[i] * second.digits[j];
        }
        answer.normalize();
    }
    while (answer.digits.back() == 0 && answer.digits.size() > 1) {
        answer.digits.pop_back();
    }
    digits = answer.digits;
    isNegative ^= static_cast<int>(second.isNegative);
    return *this;
}

BigInteger& BigInteger::operator>>=(int x) {
    if (digits.empty()) {
        digits.push_back(0);
        return *this;
    }
    for (int j = 0; j < x; ++j) {
        digits.push_back(digits[digits.size() - 1]);
        for (size_t i = digits.size() - 2; i > 0; --i) {
            digits[i] = digits[i - 1];
        }
        digits[0] = 0;
    }
    return *this;
}

BigInteger& BigInteger::operator/=(const BigInteger& second) {
    auto sec = second;
    sec.isNegative = false;
    isNegative ^= static_cast<int>(second.isNegative);
    if (*this == 0) {
        return *this;
    }
    BigInteger result, current;
    result.digits.resize(digits.size());
    result.isNegative = false;
    for (long long i = static_cast<long long>(digits.size()) - 1; i >= 0; --i) {
        current >>= 1;
        current.digits[0] = digits[i];
        while (current.digits.back() == 0 && current.digits.size() > 1) {
            current.digits.pop_back();
        }
        int x = 0, l = 0, r = base;
        while (l <= r) {
            int m = (l + r) / 2;
            BigInteger t = sec * m;
            if (t <= current) {
                x = m;
                l = m + 1;
            } else {
                r = m - 1;
            }
        }
        result.digits[i] = x;
        current = current - sec * x;
    }
    while (result.digits.back() == 0 && result.digits.size() > 1) {
        result.digits.pop_back();
    }
    digits = result.digits;
    normalize();
    return *this;
}

BigInteger& BigInteger::operator%=(const BigInteger& second) {
    *this -= (*this / second) * second;
    return *this;
}

BigInteger& BigInteger::operator++() {
    *this += 1;
    return *this;
}

BigInteger BigInteger::operator++(int) {
    auto it(*this);
    ++(*this);
    return it;
}

BigInteger& BigInteger::operator--() {
    *this -= 1;
    return *this;
}

BigInteger BigInteger::operator--(int) {
    auto it(*this);
    --(*this);
    return it;
}

BigInteger operator+(const BigInteger& first, const BigInteger& second) {
    BigInteger answer(first);
    answer += second;
    return answer;
}

BigInteger operator-(const BigInteger& first, const BigInteger& second) {
    BigInteger answer(first);
    answer -= second;
    return answer;
}

BigInteger operator-(const BigInteger& first) {
    return (0 - first);
}

BigInteger operator*(BigInteger first, const BigInteger& second) {
    first *= second;
    return first;
}

BigInteger operator/(BigInteger first, const BigInteger& second) {
    first /= second;
    return first;
}

BigInteger operator%(BigInteger first, const BigInteger& second) {
    return (first %= second);
}

bool operator==(const BigInteger& first, const BigInteger& second) {
    return first.getDigits() == second.getDigits();
}

bool operator!=(const BigInteger& first, const BigInteger& second) {
    return !(first == second);
}

bool operator<(const BigInteger& first, const BigInteger& second) {
    if (first.getNegative() ^ second.getNegative()) {
        return first.getNegative();
    }
    if (first.getDigits().size() != second.getDigits().size() ||
        first.getDigits().empty()) {
        return first.getDigits().size() < second.getDigits().size();
    }
    int ind = static_cast<int>(first.getDigits().size()) - 1;
    while (ind >= 0 && first.getDigits()[ind] == second.getDigits()[ind]) {
        --ind;
    }
    return ind > -1 && first.getDigits()[ind] < second.getDigits()[ind] &&
           (!first.getNegative());
}
bool operator>(const BigInteger& first, const BigInteger& second) {
    return second < first;
}

bool operator<=(const BigInteger& first, const BigInteger& second) {
    return !(first > second);
}

bool operator>=(const BigInteger& first, const BigInteger& second) {
    return !(first < second);
}

std::string BigInteger::toString() const {
    std::string answer;
    if (isNegative && (*this) != 0) {
        answer += '-';
    }
    answer += std::to_string(digits.back());
    for (int i = static_cast<int>(digits.size()) - 2; i >= 0; --i) {
        auto cur = std::to_string(digits[i]);
        int x = (std::to_string(base).size() - cur.size()) - 1;
        for (int j = 0; j < x; ++j) {
            answer += '0';
        }
        answer += std::to_string(digits[i]);
    }
    return answer;
}

BigInteger operator""_bi(const char* value, size_t /*unused*/) {
    BigInteger answer(value);
    return answer;
}

BigInteger operator""_bi(unsigned long long value) {
    BigInteger answer(value);
    return answer;
}

std::istream& operator>>(std::istream& is, BigInteger& result) {
    std::string rawInput;
    is >> rawInput;
    result = static_cast<BigInteger>(rawInput);
    return is;
}

std::ostream& operator<<(std::ostream& os, const BigInteger& toPrint) {
    return os << toPrint.toString();
}

BigInteger::operator bool() {
    return (*this) != 0;
}

Rational::Rational() {
    numerator = 0;
    denominator = 1;
}

Rational::Rational(long long first) {
    numerator = first;
    denominator = 1;
    isNegative ^= static_cast<int>(first < 0);
    if (numerator < 0) {
        numerator *= -1;
    }
}

Rational::Rational(BigInteger first) {
    numerator = first;
    denominator = 1;
    isNegative ^= static_cast<int>(first.getNegative());
    if (numerator < 0) {
        numerator *= -1;
    }
}

Rational::Rational(BigInteger first, BigInteger second) {
    numerator = first;
    denominator = second;
    gcd(numerator, denominator);
}

void gcd(BigInteger& first, BigInteger& second) {
    auto saveFirst = first, saveSecond = second;
    while (second != 0) {
        first %= second;
        std::swap(first, second);
    }
    saveFirst /= first;
    saveSecond /= first;
    first = saveFirst;
    second = saveSecond;
}

const BigInteger& Rational::getDenominator() const {
    return denominator;
}

const BigInteger& Rational::getNumerator() const {
    return numerator;
}

Rational& Rational::operator+=(const Rational& second) {
    if (isNegative != second.isNegative) {
        isNegative ^= 1;
        *this -= second;
        isNegative ^= 1;
        return *this;
    }
    numerator = numerator * second.denominator + second.numerator * denominator;
    denominator *= second.denominator;
    isNegative = ((isNegative ^ (numerator < 0) ^
                   static_cast<int>(denominator < 0)) != 0);
    if (numerator < 0) {
        numerator *= -1;
    }
    if (denominator < 0) {
        denominator *= -1;
    }
    gcd(denominator, numerator);
    return *this;
}

Rational& Rational::operator-=(const Rational& second) {
    if (isNegative != second.isNegative) {
        isNegative ^= 1;
        *this += second;
        isNegative ^= 1;
        return *this;
    }
    numerator = numerator * second.denominator - second.numerator * denominator;
    denominator *= second.denominator;
    isNegative = ((isNegative ^ (numerator < 0) ^
                   static_cast<int>(denominator < 0)) != 0);
    if (numerator < 0) {
        numerator *= -1;
    }
    if (denominator < 0) {
        denominator *= -1;
    }
    gcd(denominator, numerator);
    return *this;
}

Rational& Rational::operator*=(const Rational& second) {
    numerator *= second.numerator;
    denominator *= second.denominator;
    isNegative ^= static_cast<int>(second.isNegative);
    gcd(denominator, numerator);
    return *this;
}

Rational& Rational::operator/=(const Rational& second) {
    numerator *= second.denominator;
    denominator *= second.numerator;
    isNegative ^= static_cast<int>(second.isNegative);
    gcd(denominator, numerator);
    return *this;
}

Rational& Rational::operator++() {
    numerator += denominator;
    gcd(denominator, numerator);
    return *this;
}

Rational Rational::operator++(int) {
    auto it(*this);
    ++(*this);
    return it;
}

Rational& Rational::operator--() {
    numerator -= denominator;
    isNegative ^= static_cast<int>(denominator.getNegative());
    if (denominator.getNegative()) {
        denominator *= -1;
    }
    gcd(denominator, numerator);
    return *this;
}

Rational Rational::operator--(int) {
    auto it(*this);
    --(*this);
    return it;
}

Rational operator+(const Rational& first, const Rational& second) {
    Rational answer(first);
    answer += second;
    return answer;
}

Rational operator-(const Rational& first, const Rational& second) {
    Rational answer(first);
    answer -= second;
    return answer;
}

Rational operator-(const Rational& first) {
    return (0 - first);
}

Rational operator*(Rational first, const Rational& second) {
    first *= second;
    return first;
}

Rational operator/(Rational first, const Rational& second) {
    first /= second;
    return first;
}

bool operator==(const Rational& first, const Rational& second) {
    return first.getNumerator() == second.getNumerator() &&
           first.getDenominator() == second.getDenominator();
}

bool operator!=(const Rational& first, const Rational& second) {
    return !(first == second);
}

bool operator<(const Rational& first, const Rational& second) {
    int firstSign = 1, secondSign = 1;
    if (first.getSign()) {
        firstSign = -1;
    }
    if (second.getSign()) {
        secondSign = -1;
    }
    return (firstSign * first.getNumerator() * second.getDenominator() -
                secondSign * second.getNumerator() * first.getDenominator() <
            0);
}

bool operator<=(const Rational& first, const Rational& second) {
    return !(first > second);
}

bool operator>(const Rational& first, const Rational& second) {
    return second < first;
}

bool operator>=(const Rational& first, const Rational& second) {
    return !(first < second);
}

std::string Rational::toString() const {
    std::string answer;
    if (isNegative) {
        answer += '-';
    }
    answer += numerator.toString();
    if (denominator == 1 || numerator == 0) {
        return answer;
    }
    answer += '/' + denominator.toString();
    return answer;
}

long long BigInteger::getBase() const {
    return base;
}

std::string Rational::asDecimal(size_t precision) {
    std::string answer;
    if (isNegative) {
        answer += '-';
    }
    auto doub = (numerator / denominator).toString();
    std::string cntZero;
    auto save = numerator;
    while (numerator < denominator) {
        numerator *= 10;
        cntZero += '0';
    }
    if (!cntZero.empty()) {
        cntZero.pop_back();
    }
    numerator = save;
    for (size_t i = 0; i < std::max(static_cast<size_t>(3), precision / 5);
         ++i) {
        numerator *= numerator.getBase();
    }
    auto cur = (numerator / denominator).toString();
    numerator = save;
    answer += doub + '.' + cntZero +
              cur.substr(
                  doub.size() - 1 + static_cast<unsigned long>(doub != "0"),
                  std::max(static_cast<size_t>(0), precision - cntZero.size()));
    return answer;
}

Rational::operator double() {
    return static_cast<double>(std::stod((*this).asDecimal(30)));
}

bool Rational::getSign() const {
    return isNegative;
}
