#include <cstring>
#include <iostream>
#include <string>
#include <vector>

class BigInteger;
BigInteger operator+(const BigInteger& a, const BigInteger& b);
BigInteger operator-(const BigInteger& a, const BigInteger& b);
BigInteger operator*(const BigInteger& a, const BigInteger& b);
bool operator<(const BigInteger& a, const BigInteger& b);
bool operator>=(const BigInteger& a, const BigInteger& b);
bool operator>(const BigInteger& a, const BigInteger& b);
bool operator<=(const BigInteger& a, const BigInteger& b);
bool operator==(const BigInteger& a, const BigInteger& b);
bool operator!=(const BigInteger& a, const BigInteger& b);
std::ostream& operator<<(std::ostream& out, const BigInteger& bi);
std::istream& operator>>(std::istream& in, BigInteger& bi);
BigInteger operator""_bi(unsigned long long number);
BigInteger operator""_bi(const char* number, size_t);

class BigInteger {
  friend bool operator<(const BigInteger& a, const BigInteger& b);
  friend bool operator==(const BigInteger& a, const BigInteger& b);
  friend std::ostream& operator<<(std::ostream& out, const BigInteger& bi);

 private:
  std::vector<long long> digits;
  bool isNegative = false;
  int static const MOD = 1000000000;
  int static const POW = 9;
  int static const BASE = 10;

  // Удалить нули в конце
  void delZero() {
    int sz = digits.size() - 1;
    while (sz >= 0 && digits[sz] == 0) {
      digits.pop_back();
      --sz;
    }
  }

  // Привести к виду, где цифры лежат в диапазоне [0, MOD - 1]
  void beautify() {
    long long add = 0;
    delZero();
    size_t sz = digits.size();
    for (size_t i = 0; i < sz; ++i) {
      digits[i] += add;
      if (digits[i] < 0) {
        add = -1;
      } else {
        add = digits[i] / MOD;
      }
      digits[i] = (digits[i] + MOD) % MOD;
    }
    while (add != 0) {
      digits.push_back(add);
      add = digits[sz] / MOD;
      digits[sz] %= MOD;
      ++sz;
    }
    delZero();
    if (digits.size() == 0) {
      isNegative = false;
    }
  }

  // <= по модулю
  bool absLessEq(const BigInteger& other) {
    if (digits.size() != other.digits.size()) {
      return digits.size() < other.digits.size();
    }
    for (int i = int(digits.size() - 1); i >= 0; --i) {
      if (digits[i] != other.digits[i]) {
        return digits[i] < other.digits[i];
      }
    }
    return true;
  }

  // < по модулю
  bool absLess(const BigInteger& other, int index = 0) {
    if (digits.size() == 0) {
      return true;
    }
    int start = digits.size() - 1;
    while (digits[start] == 0 && start - 1 >= 0) {
      --start;
    }
    if (start + 1 - index != int(other.digits.size())) {
      return start + 1 - index < int(other.digits.size());
    }
    int j = other.digits.size() - 1;
    for (int i = start; i >= index; --i) {
      if (digits[i] != other.digits[j]) {
        return digits[i] < other.digits[j];
      }
      --j;
    }
    return false;
  }

  // Увеличить модуль на 1
  void increaseAbs() {
    for (int i = 0; i < int(digits.size()); ++i) {
      if (digits[i] == BASE - 1) {
        digits[i] = 0;
      } else {
        ++digits[i];
        break;
      }
    }
  }

  // Уменьшить модуль на 1
  void decreaseAbs() {
    for (int i = 0; i < int(digits.size()); ++i) {
      if (digits[i] == 0) {
        digits[i] = BASE - 1;
      } else {
        --digits[i];
        break;
      }
    }
  }

  // Добавить цифру в конец
  void addDigit(char digit) {
    digits[digits.size() - 1] *= BASE;
    digits[digits.size() - 1] += digit;
  }

 public:
  BigInteger() : digits({}), isNegative(false) {}
  BigInteger(long long x) : digits({}), isNegative(x < 0) {
    if (x < 0) {
      x *= -1;
    }
    while (x > 0) {
      digits.push_back(x % MOD);
      x /= MOD;
    }
  }

  BigInteger(const std::string str)
      : digits({}), isNegative(str.size() > 0 && str[0] == '-') {
    int len = str.size();
    if (len == 0) {
      return;
    }
    int board = 0;
    if (isNegative) board = 1;
    for (int i = len - POW; i >= board; i -= POW) {
      digits.push_back(0);
      for (int j = i; j < i + POW; ++j) {
        addDigit(str[j] - '0');
      }
    }
    if ((len - isNegative) % POW != 0) {
      digits.push_back(0);
      for (int j = board; j < (len - isNegative) % POW + isNegative; ++j) {
        addDigit(str[j] - '0');
      }
    }
  }

  explicit operator bool() const { return !(digits.size() == 0); }

  BigInteger& operator=(long long x) {
    *this = BigInteger(x);
    return *this;
  }

  BigInteger& operator+=(const BigInteger& other) {
    int my_sz = digits.size();
    int other_sz = other.digits.size();
    if (other_sz > my_sz) {
      digits.resize(other_sz);
      my_sz = other_sz;
    }
    if (isNegative == other.isNegative) {
      for (int i = 0; i < std::min(my_sz, other_sz); ++i) {
        digits[i] += other.digits[i];
      }
    } else {
      if (absLessEq(other)) {
        isNegative = other.isNegative;
        for (int i = 0; i < std::min(my_sz, other_sz); ++i) {
          digits[i] = other.digits[i] - digits[i];
        }
      } else {
        for (int i = 0; i < std::min(my_sz, other_sz); ++i) {
          digits[i] -= other.digits[i];
        }
      }
    }
    beautify();
    return *this;
  }

  BigInteger& operator++() {
    digits.push_back(0);
    if (isNegative) {
      decreaseAbs();
    } else {
      increaseAbs();
    }
    delZero();
    return *this;
  }

  BigInteger operator-() {
    isNegative = !isNegative;
    return *this;
  }

  BigInteger operator++(int) {
    BigInteger copy = *this;
    ++*this;
    return copy;
  }

  BigInteger& operator-=(const BigInteger& other) {
    BigInteger copy = other;
    copy.isNegative = !copy.isNegative;
    *this += copy;
    return *this;
  }

  BigInteger& operator--() {
    digits.push_back(0);
    if (isNegative) {
      increaseAbs();
    } else {
      decreaseAbs();
    }
    delZero();
    return *this;
  }

  BigInteger operator--(int) {
    BigInteger copy = *this;
    --*this;
    return copy;
  }

  BigInteger& operator*=(const BigInteger& other) {
    BigInteger res;
    if (other == 1_bi) {
      return *this;
    }
    if (other == -1_bi) {
      isNegative = !isNegative;
      return *this;
    }
    res.isNegative = (isNegative != other.isNegative);
    res.digits.assign(digits.size() + other.digits.size(), 0);
    for (int j = 0; j < int(other.digits.size()); ++j) {
      for (int i = 0; i < int(digits.size()); ++i) {
        res.digits[i + j] += digits[i] * other.digits[j];
        if (i + j < int(res.digits.size()) - 1) {
          res.digits[i + j + 1] += res.digits[i + j] / MOD;
          res.digits[i + j] %= MOD;
        }
      }
    }
    res.beautify();
    *this = res;
    return *this;
  }

  BigInteger& operator/=(BigInteger other) {
    BigInteger add = 0_bi, ans = 0_bi;
    ans.isNegative = (isNegative != other.isNegative);
    other.isNegative = false;
    for (int i = digits.size(); i > 0; --i) {
      add *= MOD;
      add += digits[i - 1];
      if (add < other) {
        ans.digits.push_back(0);
        continue;
      }
      int l = 0, r = MOD;
      while (r - l > 1) {
        int mid = (l + r) / 2;
        if (other * mid <= add) {
          l = mid;
        } else {
          r = mid;
        }
      }
      ans.digits.push_back(l);
      add -= other * l;
    }
    std::reverse(ans.digits.begin(), ans.digits.end());
    *this = ans;
    beautify();
    return *this;
  }

  BigInteger& operator%=(BigInteger other) {
    BigInteger temp = *this;
    temp /= other;
    temp *= other;
    *this -= temp;
    return *this;
  }

  std::string toString() const {
    std::string str;
    if (digits.size() == 0) {
      return "0";
    }
    if (isNegative) {
      str.push_back('-');
    }
    for (int i = int(digits.size()) - 1; i >= 0; --i) {
      std::string add = std::to_string(digits[i]);
      if (i != int(digits.size()) - 1) {
        for (int j = 0; j < POW - int(add.size()); ++j) {
          str.push_back('0');
        }
      }
      str += add;
    }
    return str;
  }
};

BigInteger operator+(const BigInteger& a, const BigInteger& b) {
  BigInteger result = a;
  result += b;
  return result;
}

BigInteger operator-(const BigInteger& a, const BigInteger& b) {
  BigInteger result = a;
  result -= b;
  return result;
}

BigInteger operator*(const BigInteger& a, const BigInteger& b) {
  BigInteger result = a;
  result *= b;
  return result;
}

BigInteger operator/(const BigInteger& a, const BigInteger& b) {
  BigInteger result = a;
  result /= b;
  return result;
}

BigInteger operator%(const BigInteger& a, const BigInteger& b) {
  BigInteger result = a;
  result %= b;
  return result;
}

bool operator<(const BigInteger& a, const BigInteger& b) {
  if (a.isNegative && !b.isNegative) {
    return true;
  }
  if (!a.isNegative && b.isNegative) {
    return false;
  }
  bool ans;
  bool got = false;
  if (a.digits.size() != b.digits.size()) {
    ans = (a.digits.size() < b.digits.size());
    got = true;
  } else {
    for (int i = int(a.digits.size()) - 1; i >= 0; --i) {
      if (a.digits[i] != b.digits[i]) {
        ans = (a.digits[i] < b.digits[i]);
        got = true;
        break;
      }
    }
  }
  if (!got) {
    return false;
  }
  if (a.isNegative && b.isNegative) {
    return !ans;
  }
  return ans;
}

bool operator>=(const BigInteger& a, const BigInteger& b) { return !(a < b); }

bool operator>(const BigInteger& a, const BigInteger& b) { return b < a; }

bool operator<=(const BigInteger& a, const BigInteger& b) { return !(a > b); }

bool operator==(const BigInteger& a, const BigInteger& b) {
  if (a.isNegative != b.isNegative) {
    return false;
  }
  if (a.digits.size() != b.digits.size()) {
    return false;
  }
  for (int i = int(a.digits.size()) - 1; i >= 0; --i) {
    if (a.digits[i] != b.digits[i]) {
      return false;
    }
  }
  return true;
}

bool operator!=(const BigInteger& a, const BigInteger& b) { return !(a == b); }

std::ostream& operator<<(std::ostream& out, const BigInteger& bi) {
  if (bi.isNegative) {
    out << '-';
  }
  for (int i = bi.digits.size() - 1; i >= 0; --i) {
    if (bi.digits[i] == 0) {
      for (int j = 0; j < bi.POW; ++j) {
        out << 0;
      }
      continue;
    }
    if (i != int(bi.digits.size()) - 1) {
      std::string out_str = std::to_string(bi.digits[i]);
      for (int j = 0; j < bi.POW - int(out_str.size()); ++j) {
        out << 0;
      }
    }
    out << bi.digits[i];
  }
  if (bi.digits.size() == 0) {
    out << 0;
  }
  return out;
}

std::istream& operator>>(std::istream& in, BigInteger& bi) {
  std::string cin_str;
  in >> cin_str;
  bi = cin_str;
  return in;
}

BigInteger operator""_bi(unsigned long long number) {
  BigInteger result(number);
  return result;
}

BigInteger operator""_bi(const char* number, size_t) {
  BigInteger result(number);
  return result;
}

class Rational {
  bool friend operator<(const Rational& a, const Rational& b);
  bool friend operator==(const Rational& a, const Rational& b);

 private:
  BigInteger Up;
  BigInteger Down;

  // НОД
  BigInteger GCD(const BigInteger& a, const BigInteger& b) {
    if (a == 0_bi) {
      if (b < 0_bi) {
        return b * -1;
      }
      return b;
    }
    return GCD(b % a, a);
  }

  // привести к несократимой дроби
  void beautify() {
    BigInteger gcd = GCD(Up, Down);
    Up /= gcd;
    Down /= gcd;
    if (Up == 0_bi) {
      Down = 1;
      return;
    }
    if (Down < 0_bi) {
      Down *= -1;
      Up *= -1;
    }
  }

 public:
  Rational() : Up(0_bi), Down(1_bi) {}
  Rational(const BigInteger& bi) : Up(bi), Down(1){};
  Rational(long long x) : Up(x), Down(1){};

  Rational& operator+=(const Rational& other) {
    Up *= other.Down;
    Up += other.Up * Down;
    Down *= other.Down;
    beautify();
    return *this;
  }

  Rational& operator-=(const Rational& other) {
    Up *= other.Down;
    Up -= other.Up * Down;
    Down *= other.Down;
    beautify();
    return *this;
  }

  Rational& operator/=(const Rational& other) {
    Up *= other.Down;
    Down *= other.Up;
    beautify();
    return *this;
  }

  Rational& operator*=(const Rational& other) {
    Up *= other.Up;
    Down *= other.Down;
    beautify();
    return *this;
  }

  Rational operator-() {
    Rational copy = *this;
    copy.Up *= -1_bi;
    return copy;
  }

  std::string toString() {
    std::string ans = Up.toString();
    if (Down != 1_bi) {
      ans.push_back('/');
      ans += Down.toString();
    }
    return ans;
  }

  std::string asDecimal(size_t precision = 0) {
    if (precision == 0) {
      return toString();
    }
    BigInteger ten(1);
    for (size_t i = 1; i <= precision; ++i) {
      ten *= 10_bi;
    }
    BigInteger div = (Up * ten / Down);
    std::string temp = div.toString();
    std::string ans;
    if (temp[0] == '-') {
      ans += '-';
      temp = temp.substr(1, temp.size());
    }
    if (precision >= temp.size()) {
      ans += '0';
      ans += '.';
    }
    for (int i = 0; i < int(precision) - int(temp.size()); ++i) {
      ans += '0';
    }
    for (int i = 0; i < int(temp.size()); ++i) {
      if (i == int(temp.size()) - int(precision)) {
        ans += '.';
      }
      ans += temp[i];
    }
    return ans;
  }

  explicit operator double() { return std::atof(asDecimal(20).c_str()); }
};

Rational operator-(const Rational& a, const Rational& b) {
  Rational result = a;
  result -= b;
  return result;
}

Rational operator+(const Rational& a, const Rational& b) {
  Rational result = a;
  result += b;
  return result;
}

Rational operator*(const Rational& a, const Rational& b) {
  Rational result = a;
  result *= b;
  return result;
}

Rational operator/(const Rational& a, const Rational& b) {
  Rational result = a;
  result /= b;
  return result;
}

bool operator<(const Rational& a, const Rational& b) {
  return a.Up * b.Down < a.Down * b.Up;
}

bool operator>(const Rational& a, const Rational& b) { return b < a; }

bool operator>=(const Rational& a, const Rational& b) { return !(a < b); }

bool operator<=(const Rational& a, const Rational& b) { return !(b < a); }

bool operator==(const Rational& a, const Rational& b) {
  return a.Up * b.Down == a.Down * b.Up;
}

bool operator!=(const Rational& a, const Rational& b) { return !(a == b); }
