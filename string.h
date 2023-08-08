#include <algorithm>
#include <iostream>
#include <cstring>

class String {
 private:
  size_t sz = 0;
  size_t cap = 0;
  char* arr = nullptr;

  // Resize
  void realloc(int new_cap = -1) {
    int calc_cap = new_cap == -1 ? std::max(size_t(1), cap * 2) : new_cap;
    char* new_arr = new char[calc_cap + 1];
    std::copy(arr, arr + sz + 1, new_arr);
    delete[] arr;
    arr = new_arr;
    cap = calc_cap;
  }

  void swap(String& str) {
    std::swap(arr, str.arr);
    std::swap(sz, str.sz);
    std::swap(cap, str.cap);
  }

 public:
  // конструктор по умолчанию
  String() : sz(0), cap(0), arr(new char[1]) { arr[0] = '\0'; }

  // конструктор заполнения
  String(size_t n, char c) : sz(n), cap(n), arr(new char[n + 1]) {
    memset(arr, c, n);
    arr[n] = '\0';
  }

  // конструктор копирования c-style
  String(const char* str) : sz(strlen(str)), cap(sz), arr(new char[sz + 1]) {
    memcpy(arr, str, sz + 1);
  }

  // копирующий конструктор
  String(const String& str) : sz(str.sz), cap(str.cap), arr(new char[cap + 1]) {
    memcpy(arr, str.arr, sz + 1);
  }

  // присваивание
  String& operator=(String str) {
    swap(str);
    return *this;
  }

  // оператор []
  char& operator[](size_t index) { return arr[index]; }

  const char& operator[](size_t index) const { return arr[index]; }

  // указатель на начало
  const char* data() const { return arr; }

  char* data() { return arr; }

  // длина
  size_t length() const { return sz; }

  size_t size() const { return sz; }

  size_t capacity() const { return cap; }

  // operator +=
  String& operator+=(const String& other) {
    // здесь менять cap не надо, потому что в cap не учитывается '\0'
    if (cap < sz + other.sz) {
      realloc(other.sz + sz);
    }
    memcpy(arr + sz, other.arr, other.sz + 1);
    sz += other.sz;
    return *this;
  }

  String& operator+=(const char& c) {
    // здесь менять cap не надо, потому что в cap не учитывается '\0'
    if (cap < sz + 1) {
      realloc();
    }
    arr[sz] = c;
    ++sz;
    arr[sz] = '\0';
    return *this;
  }

  // push_back
  void push_back(const char& c) { *this += c; }

  // pop_back
  void pop_back() {
    --sz;
    arr[sz] = '\0';
  }

  // front
  char& front() { return arr[0]; }

  const char& front() const { return arr[0]; }

  // back
  char& back() { return arr[sz - 1]; }

  const char& back() const { return arr[sz - 1]; }

  // левое вхождение подстроки
  size_t find(const String& sub) const {
    for (size_t i = 0; i <= sz - sub.sz; ++i) {
      if (!memcmp(arr + i, sub.arr, sub.sz)) {
        return i;
      }
    }
    return sz;
  }

  // правое вхождение подстроки
  size_t rfind(const String& sub) const {
    for (int i = int(sz - sub.sz); i >= 0; --i) {
      if (!memcmp(arr + i, sub.arr, sub.sz)) {
        return i;
      }
    }
    return sz;
  }

  // подстрока
  String substr(int start, int count) const {
    String sub(count, 'a');
    // после этого конструктора на count+1 месте уже '\0'
    memcpy(sub.arr, arr + start, count);
    return sub;
  }

  // проверка на пустоту
  bool empty() { return sz == 0; }

  // очистка
  void clear() {
    sz = 0;
    arr[sz] = '\0';
  }

  // убрать ненужную память
  void shrink_to_fit() { 
    realloc(sz);
  }

  ~String() { delete[] arr; }
};

// operator +
String operator+(String a, const String& b) {
  a += b;
  return a;
}

String operator+(String a, const char& c) {
  a += c;
  return a;
}

String operator+(const char& c, const String& a) {
  String result(1, c);
  result += a;
  return result;
}

// оператор ==
bool operator==(const String& a, const String& b) {
  if (a.length() != b.length()) {
    return false;
  }
  if (memcmp(a.data(), b.data(), a.length())) {
    return false;
  }
  return true;
}

// operator !=
bool operator!=(const String& a, const String& b) { return !(a == b); }

// оператор <
bool operator<(const String& a, const String& b) {
  const char* ptr_a = a.data();
  const char* ptr_b = b.data();
  int sz = std::min(strlen(ptr_a), strlen(ptr_b));
  return memcmp(ptr_a, ptr_b, sz + 1) < 0;
}

// оператор <=
bool operator<=(const String& a, const String& b) { return !(b < a); }

// оператор >
bool operator>(const String& a, const String& b) { return b < a; }

// оператор >=
bool operator>=(const String& a, const String& b) { return !(a < b); }

// ввод - вывод

std::ostream& operator<<(std::ostream& os, const String& str) {
  os << str.data();
  return os;
}

std::istream& operator>>(std::istream& is, String& str) {
  str.clear();
  while (true) {
    char c = is.get();
    if (isspace(c) || c == EOF) {
      break;
    }
    str += c;
  }
  int sz = strlen(str.data());
  str[sz] = '\0';
  return is;
}
