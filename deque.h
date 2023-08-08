template<typename T>
class Deque {
 public:
  template<typename Value>
  class base_iterator {
   public:
    using difference_type = size_t;
    using value_type = Value;
    using reference = value_type &;
    using pointer = value_type *;
    using iterator_category = std::random_access_iterator_tag;

   private:
    friend Deque;
    value_type **iter_map = nullptr;
    int chunk;
    int pos;

    int to_int() const { return chunk * chunk_sz + pos; }

   public:
    base_iterator() = default;

    base_iterator(T **map, std::pair<int, int> ptr) : iter_map(const_cast<value_type **>(map)), chunk(ptr.first),
                                                      pos(ptr.second) {}

    base_iterator(const base_iterator &other) : iter_map(other.iter_map), chunk(other.chunk), pos(other.pos) {}

    base_iterator &operator=(const base_iterator &) = default;

    operator base_iterator<const T>() const {
      return base_iterator<const T>(iter_map, {chunk, pos});
    }

    base_iterator &operator++() {
      if (pos == chunk_sz - 1) {
        ++chunk;
        pos = 0;
      } else {
        ++pos;
      }
      return *this;
    }

    base_iterator &operator--() {
      if (pos == 0) {
        --chunk;
        pos = chunk_sz - 1;
      } else {
        --pos;
      }
      return *this;
    }

    base_iterator operator++(int) {
      base_iterator cp = *this;
      ++(*this);
      return cp;
    }

    base_iterator operator--(int) {
      base_iterator cp = *this;
      --(*this);
      return cp;
    }

    base_iterator &operator+=(difference_type x) {
      int int_ptr = to_int() + x;
      chunk = int_ptr / chunk_sz;
      pos = int_ptr % chunk_sz;
      return *this;
    }

    base_iterator &operator-=(difference_type x) {
      *this += -x;
      return *this;
    }

    bool operator==(const base_iterator<value_type> &other) const {
      if (chunk == other.chunk && pos == other.pos) {
        return (iter_map == nullptr && other.iter_map == nullptr) ||
               iter_map == other.iter_map;
      }
      return false;
    };

    bool operator!=(const base_iterator<value_type> &other) const {
      return !(*this == other);
    }

    bool operator<(const base_iterator<value_type> &other) const {
      return to_int() < other.to_int() && iter_map == other.iter_map;
    }

    bool operator<=(const base_iterator<value_type> &other) const {
      return *this < other || *this == other;
    }

    bool operator>(const base_iterator<value_type> &other) const {
      return !(*this <= other);
    }

    bool operator>=(const base_iterator<value_type> &other) const {
      return !(*this < other);
    }

    base_iterator<value_type> operator+(difference_type x) const {
      base_iterator<value_type> tmp = *this;
      tmp += x;
      return tmp;
    }

    base_iterator<value_type> operator-(difference_type x) const {
      base_iterator<value_type> tmp = *this;
      tmp -= x;
      return tmp;
    }

    difference_type operator-(const base_iterator<value_type> &other) const {
      return to_int() - other.to_int();
    }

    value_type &operator*() const { return iter_map[chunk][pos]; }

    value_type *operator->() const { return iter_map[chunk] + pos; }
  };

  using iterator = base_iterator<T>;
  using const_iterator = base_iterator<const T>;

 private:
  size_t sz = 0;
  size_t map_cap = 0;
  T **map = nullptr;
  static const int chunk_sz = 32;
  iterator first = iterator(map, {0, 0}), last = iterator(map, {0, 0});

  void fill_map(size_t new_sz) {
    sz = new_sz;
    map_cap = 3 * (new_sz + chunk_sz - 1) / chunk_sz;
    try {
      map = new T *[map_cap];
    } catch (...) {
      throw;
    }
    first = iterator(map, {map_cap / 3, 0});
    last = iterator(map, {2 * map_cap / 3 - 1, ((new_sz - 1) % chunk_sz)});
    try {
      for (int i = first.chunk; i <= last.chunk; ++i) {
        map[i] = reinterpret_cast<T *>(new int8_t[sizeof(T) * chunk_sz]);
      }
    } catch (...) {
      clear_map(first.chunk, last.chunk);
    }
  }

  void destroy_T(iterator beg, iterator end) {
    if (map_cap == 0) return;
    for (iterator del = beg; del != end; ++del) {
      (map[del.chunk] + del.pos)->~T();
    }
  }

  void clear_map(int fir_ch, int last_ch) {
    if (map_cap == 0) return;
    for (int j = fir_ch; j <= last_ch; ++j) {
      delete[] reinterpret_cast<int8_t *>(map[j]);
    }
    delete[] map;
  }

  void swap(Deque<T> &other) {
    std::swap(map, other.map);
    std::swap(sz, other.sz);
    std::swap(map_cap, other.map_cap);
    std::swap(first, other.first);
    std::swap(last, other.last);
  }

  template<typename... Args>
  void construct(const int new_sz, Args &&... args) {
    fill_map(new_sz);
    iterator cur = first;
    try {
      for (; cur != last + 1; ++cur) {
        new(map[cur.chunk] + cur.pos) T(std::forward<Args>(args)...);
      }
    } catch (...) {
      destroy_T(first, cur);
      clear_map(first.chunk, last.chunk);
      throw;
    }
  }

 public:
  Deque() = default;

  ~Deque() {
    destroy_T(first, last + 1);
    clear_map(first.chunk, last.chunk);
  }

  Deque(const Deque<T> &other) : sz(other.sz), map_cap(other.map_cap), first(other.first), last(other.last) {
    try {
      map = new T *[other.map_cap];
    } catch (...) {
      throw;
    }
    first.iter_map = map;
    last.iter_map = map;
    try {
      for (int i = other.first.chunk; i <= other.last.chunk; ++i) {
        map[i] = reinterpret_cast<T *>(new int8_t[sizeof(T) * chunk_sz]);
      }
    } catch (...) {
      clear_map(first.chunk, last.chunk);
      throw;
    }

    if (map_cap == 0) return;
    iterator cur = other.first;
    try {
      for (; cur != other.last + 1; ++cur) {
        new(map[cur.chunk] + cur.pos) T(other.map[cur.chunk][cur.pos]);
      }
    } catch (...) {
      destroy_T(first, cur);
      clear_map(first.chunk, last.chunk);
      throw;
    }
  }

  Deque(const int new_sz) { construct(new_sz); }

  Deque(const int new_sz, const T &val) { construct(new_sz, val); }

  Deque &operator=(const Deque<T> &other) {
    Deque<T> tmp(other);
    swap(tmp);
    return *this;
  }

  size_t size() const { return sz; }

  T &operator[](size_t ind) {
    size_t x = first.chunk + (first.pos + ind) / chunk_sz;
    size_t y = (first.pos + ind) % chunk_sz;
    return map[x][y];
  }

  const T &operator[](size_t ind) const {
    size_t x = first.chunk + (first.pos + ind) / chunk_sz;
    size_t y = (first.pos + ind) % chunk_sz;
    return map[x][y];
  }

  T &at(size_t ind) {
    if (ind < 0 || ind >= sz) {
      throw std::out_of_range("");
    }
    return (*this)[ind];
  }

  const T &at(size_t ind) const {
    if (ind < 0 || ind >= sz) {
      throw std::out_of_range("");
    }
    return (*this)[ind];
  }

 private:
  void emplace_chunk(iterator &place, int delta, int board, const T &val) {
    try {
      if (place.pos == board) {
        map[place.chunk] = reinterpret_cast<T *>(new int8_t[sizeof(T) * chunk_sz]);
      }
    } catch (...) {
      throw;
    }
    try {
      new(map[place.chunk] + place.pos) T(val);
      ++sz;
    } catch (...) {
      if (place.pos == 0) {
        delete[] reinterpret_cast<char *>(map[place.chunk]);
      }
      place += delta;
      throw;
    }
  }

  void move_map_ptrs(int start, T **new_map) {
    if (sz != 0) {
      for (int i = 0; i <= last.chunk - first.chunk; ++i) {
        new_map[start + i] = map[first.chunk + i];
      }
    }
  }

  void change_map(iterator &place, T **new_map, int new_cap, const T &val, int board) {
    try {
      if (place.pos == board) {
        new_map[place.chunk] = reinterpret_cast<T *>(new int8_t[sizeof(T) * chunk_sz]);
      }
    } catch (...) {
      throw;
    }
    try {
      new(new_map[place.chunk] + place.pos) T(val);
    } catch (...) {
      delete[] new_map;
      throw;
    }
    std::swap(map, new_map);
    map_cap = new_cap;
    ++sz;
    delete[] new_map;
  }

 private:
  void resize_helper(const T &val, bool back) {
    size_t new_cap = 3 * (sz + chunk_sz) / chunk_sz;
    T **new_map = nullptr;
    try {
      new_map = new T *[new_cap];
    } catch (...) {
      throw;
    }
    iterator new_first = iterator(new_map, {new_cap / 3, first.pos});
    iterator new_last = back ? new_first + sz : new_first + (sz - 1);
    move_map_ptrs(new_first.chunk, new_map);
    if (!back) {
      --new_first;
    }
    try {
      change_map(back ? new_last : new_first, new_map, new_cap, val, back ? 0 : chunk_sz - 1);
    } catch (...) {
      throw;
    }
    first = new_first;
    last = new_last;
  }

 public:
  void push_back(const T &val) {
    ++last;
    if (last.chunk < int(map_cap)) {
      emplace_chunk(last, -1, 0, val);
    } else {
      --last;
      resize_helper(val, true);
    }
  }

  void push_front(const T &val) {
    --first;
    if (first.chunk >= 0) {
      emplace_chunk(first, 1, chunk_sz - 1, val);
    } else {
      ++first;
      resize_helper(val, false);
    }
  }

  void pop_back() {
    (*this)[sz - 1].~T();
    if (last.pos == 0) {
      delete[] reinterpret_cast<char *>(map[last.chunk]);
    }
    --last;
    --sz;
  }

  void pop_front() {
    (*this)[0].~T();
    if (first.pos == chunk_sz - 1) {
      delete[] reinterpret_cast<char *>(map[first.chunk]);
    }
    ++first;
    --sz;
  }

 public:
  iterator begin() { return first; }

  const_iterator begin() const { return const_iterator(first); }

  const_iterator cbegin() const { return const_iterator(first); }

  iterator end() {
    if (map_cap != 0) {
      return last + 1;
    } else {
      return last;
    }
  }

  const_iterator end() const {
    if (map_cap != 0) {
      return const_iterator(last + 1);
    } else {
      return const_iterator(last);
    }
  }

  const_iterator cend() const {
    if (map_cap != 0) {
      return const_iterator(last + 1);
    } else {
      return const_iterator(last);
    }
  }

  void insert(iterator it, const T &val) {
    T cp = val;
    while (it != this->end()) {
      std::swap(*it, cp);
      ++it;
    }
    push_back(cp);
  }

  void erase(iterator it) {
    while (it + 1 != (*this).end()) {
      *it = *(it + 1);
      ++it;
    }
    (*this).pop_back();
  }

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  auto rbegin() { return std::reverse_iterator<iterator>(end()); }

  auto rend() { return std::reverse_iterator<iterator>(begin()); }

  auto crbegin() const { return std::reverse_iterator<iterator>(cend()); }

  auto crend() const { return std::reverse_iterator<iterator>(cbegin()); }
};
