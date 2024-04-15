#include <istream>
#include <string>
#include <vector>

class BigInteger {
  private:
    std::vector<long long> digits;
    int base = 1e9;
    int len = 9;
    bool isNegative = false;
    void normalize();

  public:
    BigInteger();
    BigInteger(int x);
    BigInteger(const std::string& s);

    bool getNegative() const;
    const std::vector<long long int>& getDigits();
    const std::vector<long long int>& getDigits() const;
    long long getBase() const;

    BigInteger& operator+=(const BigInteger& second);
    BigInteger& operator-=(const BigInteger& second);
    BigInteger& operator*=(const BigInteger& second);
    BigInteger& operator/=(const BigInteger& second);
    BigInteger& operator%=(const BigInteger& second);
    BigInteger& operator++();
    BigInteger operator++(int);
    BigInteger& operator--();
    BigInteger operator--(int);
    std::string toString() const;

    explicit operator bool();

    BigInteger& operator>>=(int x);
};

BigInteger operator+(const BigInteger& first, const BigInteger& second);
BigInteger operator-(const BigInteger& first, const BigInteger& second);
BigInteger operator-(const BigInteger& first);
BigInteger operator*(BigInteger first, const BigInteger& second);
BigInteger operator/(BigInteger first, const BigInteger& second);
BigInteger operator%(BigInteger first, const BigInteger& second);

bool operator==(const BigInteger& first, const BigInteger& second);
bool operator!=(const BigInteger& first, const BigInteger& second);
bool operator<(const BigInteger& first, const BigInteger& second);
bool operator<=(const BigInteger& first, const BigInteger& second);
bool operator>(const BigInteger& first, const BigInteger& second);
bool operator>=(const BigInteger& first, const BigInteger& second);

BigInteger operator""_bi(const char* value, size_t /*unused*/);
BigInteger operator""_bi(unsigned long long value);

std::istream& operator>>(std::istream& is, BigInteger& result);
std::ostream& operator<<(std::ostream& os, const BigInteger& toprint);

class Rational {
  private:
    BigInteger denominator;
    BigInteger numerator;
    bool isNegative = false;

  public:
    Rational();
    Rational(long long first);
    Rational(BigInteger first);
    Rational(BigInteger first, BigInteger second);

    const BigInteger& getNumerator() const;
    const BigInteger& getDenominator() const;
    bool getSign() const;

    Rational& operator+=(const Rational& second);
    Rational& operator-=(const Rational& second);
    Rational& operator*=(const Rational& second);
    Rational& operator/=(const Rational& second);
    Rational& operator++();
    Rational operator++(int);
    Rational& operator--();
    Rational operator--(int);

    std::string toString() const;
    std::string asDecimal(size_t precision);

    explicit operator double();
};

void gcd(BigInteger& first, BigInteger& second);

Rational operator+(const Rational& first, const Rational& second);
Rational operator-(const Rational& first, const Rational& second);
Rational operator-(const Rational& first);
Rational operator*(Rational first, const Rational& second);
Rational operator/(Rational first, const Rational& second);

bool operator==(const Rational& first, const Rational& second);
bool operator!=(const Rational& first, const Rational& second);
bool operator<(const Rational& first, const Rational& second);
bool operator<=(const Rational& first, const Rational& second);
bool operator>(const Rational& first, const Rational& second);
bool operator>=(const Rational& first, const Rational& second);
