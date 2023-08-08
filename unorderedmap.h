#include <iostream>
#include <vector>

template<
    typename Key,
    typename Value,
    typename Hash = std::hash<Key>,
    typename Equal = std::equal_to<Key>,
    typename Alloc = std::allocator<std::pair<const Key, Value>>>
class UnorderedMap;


template<typename T, typename Alloc>
class List {
  template<
      typename Key,
      typename Value,
      typename Hash,
      typename Equal,
      typename MapAlloc>
  friend class UnorderedMap;

 public:
  using value_type = T;

  template<typename Value>
  class base_iterator;

  using iterator = base_iterator<value_type>;
  using const_iterator = base_iterator<const value_type>;
 private:
  struct BaseNode {
    BaseNode *prev = nullptr;
    BaseNode *next = nullptr;

    BaseNode() = default;

    BaseNode(BaseNode *prev, BaseNode *next) : prev(prev), next(next) {}
  };

  struct Node : public BaseNode {
    value_type *value_ptr;

    Node() = default;

    Node(value_type *value_ptr) : value_ptr(value_ptr) {}
  };

  using base_node_alloc_type = typename std::allocator_traits<Alloc>::template rebind_alloc<BaseNode>;
  using node_alloc_type = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;

  using node_alloc_traits = typename std::allocator_traits<node_alloc_type>;
  using value_alloc_traits = typename std::allocator_traits<Alloc>;

  size_t list_size = 0;
  Alloc value_alloc;
  base_node_alloc_type base_node_alloc;
  node_alloc_type node_alloc;
  BaseNode *fake_node = nullptr;

 public:
  iterator begin() noexcept { return ++iterator(fake_node); }

  const_iterator begin() const noexcept { return ++const_iterator(fake_node); }

  const_iterator cbegin() const noexcept { return ++const_iterator(fake_node); }

  iterator end() noexcept {
    return iterator(fake_node);
  }

  const_iterator end() const noexcept {
    return const_iterator(fake_node);
  }

  const_iterator cend() const noexcept {
    return const_iterator(fake_node);
  }

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  auto rbegin() noexcept { return std::reverse_iterator<iterator>(end()); }

  auto rend() noexcept { return std::reverse_iterator<iterator>(begin()); }

  auto rbegin() const noexcept { return std::reverse_iterator<const_iterator>(cend()); }

  auto rend() const noexcept { return std::reverse_iterator<const_iterator>(cbegin()); }

  auto crbegin() const noexcept { return std::reverse_iterator<iterator>(cend()); }

  auto crend() const noexcept { return std::reverse_iterator<iterator>(cbegin()); }

 public:
  iterator emplace(const_iterator it, value_type *value_ptr) {
    Node *new_node = node_alloc.allocate(1);
    try {
      node_alloc_traits::construct(node_alloc, new_node, value_ptr);
    } catch (...) {
      node_alloc.deallocate(new_node, 1);
      throw;
    }

    new_node->next = it.get_ptr();
    new_node->prev = it.get_ptr()->prev;
    it.get_ptr()->prev->next = new_node;
    it.get_ptr()->prev = new_node;

    ++list_size;
    return iterator(reinterpret_cast<BaseNode *>(new_node));
  }

  template<typename... Args>
  iterator emplace(const_iterator it, Args &&... args) {
    value_type *value_ptr = value_alloc.allocate(1);
    try {
      value_alloc_traits::construct(value_alloc, value_ptr, std::forward<Args>(args)...);
    } catch (...) {
      value_alloc.deallocate(value_ptr, 1);
      throw;
    }
    return emplace(it, value_ptr);
  }

  iterator destroy_node(const_iterator it) noexcept {
    auto prev = it.get_ptr()->prev;
    auto next = it.get_ptr()->next;

    node_alloc_traits::destroy(node_alloc, static_cast<Node *>(it.get_ptr()));
    node_alloc.deallocate(static_cast<Node *>(it.get_ptr()), 1);

    next->prev = prev;
    prev->next = next;
    --list_size;

    return iterator(next);
  }

 private:
  template<typename... Args>
  void construct_list(size_t sz, Args &&... args) {
    for (int i = 0; i < int(sz); ++i) {
      try {
        emplace(end(), std::forward<Args>(args)...);
      } catch (...) {
        clear_list();
        throw;
      }
    }
  }

  void clear_list() noexcept {
    BaseNode *cur = fake_node->next;
    BaseNode *next = cur->next;
    for (int i = 0; i < int(list_size); ++i) {
      node_alloc_traits::destroy(node_alloc, static_cast<Node *>(cur));
      node_alloc.deallocate(static_cast<Node *>(cur), 1);
      cur = next;
      next = cur->next;
    }
    list_size = 0;
  }

  void copy(const List &other) {
    fake_node = base_node_alloc.allocate(1);
    fake_node->next = fake_node;
    fake_node->prev = fake_node;
    list_size = 0;
    auto it = other.cbegin();
    for (int i = 0; i < int(other.list_size); ++i) {
      try {
        push_back(*it);
      } catch (...) {
        clear_list();
        throw;
      }
      ++it;
    }
  }

  void swap_data(List &other) noexcept {
    std::swap(fake_node, other.fake_node);
    std::swap(list_size, other.list_size);
  }

  void swap_alloc(List &other) noexcept {
    std::swap(value_alloc, other.value_alloc);
    std::swap(base_node_alloc, other.base_node_alloc);
    std::swap(node_alloc, other.node_alloc);
  }

 public:
  List(const Alloc &alloc = Alloc()) noexcept: list_size(0), value_alloc(alloc), base_node_alloc(alloc),
                                               node_alloc(alloc) {
    fake_node = base_node_alloc.allocate(1);
    try {
      std::allocator_traits<base_node_alloc_type>::construct(base_node_alloc, fake_node, fake_node, fake_node);
    } catch (...) {
      base_node_alloc.deallocate(fake_node, 1);
    }
  };

  List(size_t size, const Alloc &alloc = Alloc()) : List(alloc) {
    construct_list(size);
  }

  List(size_t size, const value_type &val, const Alloc &alloc = Alloc()) : List(alloc) {
    construct_list(size, val);
  }

  List(const List &other) :
      List(node_alloc_traits::select_on_container_copy_construction(other.get_allocator())) {
    copy(other);
  }

  List(List &&other) noexcept:
      List(std::move(
          node_alloc_traits::select_on_container_copy_construction(other.get_allocator()))) {
    swap_data(other);
    other.clear_list();
  }

  void swap(List &other) noexcept {
    swap_alloc(other);
    swap_data(other);
  }

  List &operator=(const List &other) {
    if (value_alloc_traits::propagate_on_container_copy_assignment::value) {
      List tmp(other, other.value_alloc);
      swap(tmp);
    } else {
      List tmp(other, value_alloc_traits::select_on_container_copy_construction(other.value_alloc));
      swap(tmp);
    }
    return *this;
  }

  List &operator=(List &&other) noexcept {
    if (this != &other) {
      if (value_alloc_traits::propagate_on_container_copy_assignment::value) {
        value_alloc = std::move(other.value_alloc);
        node_alloc = std::move(other.node_alloc);
        base_node_alloc = std::move(other.base_node_alloc);
      }
      swap_data(other);
      other.clear_list();
    }
    return *this;
  }

 public:
  void insert(const_iterator it, const value_type &val) { emplace(it, val); }

  void insert(const_iterator it, value_type &&val) { emplace(it, std::move(val)); }

  iterator erase(const_iterator it) noexcept {
    value_alloc_traits::destroy(value_alloc, iterator(it.get_ptr()).operator->());
    value_alloc.deallocate(iterator(it.get_ptr()).operator->(), 1);

    return destroy_node(it);
  }

  void push_back(const value_type &val) {
    emplace(cend(), val);
  }

  void push_back(value_type &&val) {
    emplace(cend(), std::move(val));
  }

  void push_front(const value_type &val) {
    emplace(cbegin(), val);
  }

  void push_front(value_type &&val) {
    emplace(cbegin(), std::move(val));
  }

  void pop_back() {
    erase(--end());
  }

  void pop_front() {
    erase(begin());
  }

  size_t size() const noexcept {
    return list_size;
  }

  Alloc get_allocator() const noexcept {
    return value_alloc;
  }

  ~List() noexcept {
    clear_list();
  }
};

template<typename T, typename Alloc>
template<typename Value>
class List<T, Alloc>::base_iterator {
 public:
  using value_type = Value;
  using pointer = value_type *;
  using reference = value_type &;
  using iterator_category = std::bidirectional_iterator_tag;
  using difference_type = std::ptrdiff_t;
  BaseNode *ptr;

 public:
  explicit base_iterator(BaseNode *ptr) : ptr(ptr) {}

  operator base_iterator<const value_type>() const {
    return const_iterator(ptr);
  }

  base_iterator &operator++() noexcept {
    ptr = ptr->next;
    return *this;
  }

  base_iterator &operator--() {
    ptr = ptr->prev;
    return *this;
  }

  base_iterator operator++(int) {
    base_iterator cp = *this;
    ptr = ptr->next;
    return cp;
  }

  base_iterator operator--(int) {
    base_iterator cp = *this;
    ptr = ptr->prev;
    return cp;
  }

  base_iterator operator+=(difference_type val) noexcept {
    while (val-- > 0) {
      ++(*this);
    }
    return *this;
  }

  base_iterator operator+(difference_type val) const noexcept {
    base_iterator tmp(*this);
    tmp += val;
    return tmp;
  }

  BaseNode *get_ptr() const { return ptr; }

  bool operator==(const base_iterator<value_type> &other) const {
    return ptr == other.ptr;
  };

  bool operator!=(const base_iterator<value_type> &other) const {
    return ptr != other.ptr;
  };

  value_type &operator*() {
    return *static_cast<Node *>(ptr)->value_ptr;
  }

  value_type *operator->() const { return (static_cast<Node *>(ptr)->value_ptr); }
};


template<
    typename Key,
    typename Value,
    typename Hash,
    typename Equal,
    typename Alloc>
class UnorderedMap {
 public:
  using NodeType = std::pair<const Key, Value>;

  using iterator = typename List<NodeType, Alloc>::iterator;
  using const_iterator = typename List<NodeType, Alloc>::const_iterator;

 private:
  static const int default_size = 16;
  double mx_load_factor = 0.9;

  Equal equalizer = Equal();
  Hash hasher = Hash();
  Alloc allocator = Alloc();

  using board = std::pair<iterator, iterator>;
  using hash_table_type = std::vector<board, typename std::allocator_traits<Alloc>::template rebind_alloc<board>>;

  List<NodeType, Alloc> list = List<NodeType, Alloc>(allocator);
  hash_table_type hash_table = hash_table_type(default_size, {list.end(), list.end()}, allocator);

 private:
  void clear_map() noexcept {
    while (size() > 0) {
      erase(begin());
    }
  }

  void copy(const UnorderedMap &other) {
    for (auto it = other.begin(); it != other.end(); ++it) {
      try {
        insert(*it);
      } catch (...) {
        clear_map();
        throw;
      }
    }
  }

  UnorderedMap(const UnorderedMap &other, const Alloc &alloc) : allocator(alloc) {
    copy(other);
  }

  void swap_data(UnorderedMap &other) noexcept {
    std::swap(mx_load_factor, other.mx_load_factor);
    std::swap(equalizer, other.equalizer);
    std::swap(hasher, other.hasher);
    list.swap_data(other.list);
    std::swap(hash_table, other.hash_table);
  }

  void swap(UnorderedMap &other) noexcept {
    swap_data(other);
    std::swap(allocator, other.allocator);
  }

 public:
  iterator begin() noexcept {
    return list.begin();
  }

  const_iterator begin() const noexcept {
    return list.begin();
  }

  const_iterator cbegin() const noexcept {
    return list.cbegin();
  }

  iterator end() noexcept {
    return list.end();
  }

  const_iterator end() const noexcept {
    return list.cend();
  }

  const_iterator cend() const noexcept {
    return list.cend();
  }

 public:
  double max_load_factor() const noexcept {
    return mx_load_factor;
  }

  void max_load_factor(double new_load_factor) {
    mx_load_factor = new_load_factor;
  }

  double load_factor() const noexcept {
    return static_cast<double>(size()) / static_cast<double>(hash_table.size());
  }

 public:
  UnorderedMap() = default;

  explicit UnorderedMap(size_t size) : hash_table(size, {list.end(), list.end()}, allocator) {}

  UnorderedMap(size_t size, const Alloc &alloc) : allocator(alloc), hash_table(size, {list.end(), list.end()}, alloc) {}

  UnorderedMap(size_t size, const Hash &hash, const Alloc &alloc) : hasher(hash), allocator(alloc),
                                                                    hash_table(size, {list.end(), list.end()}, alloc) {}

  UnorderedMap(size_t size, const Equal &equal, Hash &hash, const Alloc &alloc) :
      equalizer(equal), hasher(hash), allocator(alloc), hash_table(size, {list.end(), list.end()}, alloc) {}

  UnorderedMap(const UnorderedMap &other) :
      UnorderedMap(default_size, std::allocator_traits<Alloc>::select_on_container_copy_construction(other.allocator)) {
    insert(other.begin(), other.end());
  }

  UnorderedMap(UnorderedMap &&other) noexcept:
      mx_load_factor(other.mx_load_factor),
      equalizer(std::move(other.equalizer)),
      hasher(std::move(other.hasher)),
      allocator(std::move(std::allocator_traits<Alloc>::select_on_container_copy_construction(other.allocator))),
      list(std::move(other.list)),
      hash_table(std::move(other.hash_table)) {
    other.clear_map();
  }

  UnorderedMap &operator=(const UnorderedMap &other) {
    if (std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value) {
      UnorderedMap tmp = UnorderedMap(other, other.allocator);
      swap(tmp);
    } else {
      UnorderedMap tmp = UnorderedMap(other, allocator);
      swap(tmp);
    }
    return *this;
  }

  UnorderedMap &operator=(UnorderedMap &&other) noexcept {
    if (this != &other) {
      UnorderedMap tmp(std::move(other));
      swap(tmp);
    }
    return *this;
  }

  ~UnorderedMap() noexcept = default;

 private:
  size_t get_hash(const Key &key) const noexcept {
    return hasher(key) % hash_table.size();
  }

  void rehash(size_t new_sz) {
    List<NodeType, Alloc> new_list(allocator);
    hash_table = hash_table_type(new_sz, {new_list.end(), new_list.end()}, allocator);
    while (size() > 0) {
      auto it = begin();
      size_t hash = get_hash(it->first);
      auto left = hash_table[hash].first, right = hash_table[hash].second;
      if (left == right) {
        new_list.emplace(new_list.begin(), &(*it));
        hash_table[hash] = {new_list.begin(), new_list.begin() + 1};
      } else {
        new_list.emplace(right, &(*it));
        ++hash_table[hash].second;
      }
      list.destroy_node(it);
    }
    list = std::move(new_list);
  }

 public:
  iterator find(const Key &key) noexcept {
    size_t hash = get_hash(key);
    auto left = hash_table[hash].first, right = hash_table[hash].second;
    for (auto it = left; it != end() && it != right; ++it) {
      if (equalizer(key, it->first)) {
        return it;
      }
    }
    return end();
  }

  const_iterator find(const Key &key) const noexcept {
    return static_cast<const_iterator>(find(key));
  }

 public:
  Value &operator[](const Key &key) noexcept {
    auto it = find(key);
    if (it == end()) {
      it = insert({key, Value()}).first;
    }
    return it->second;
  }

  Value &operator[](Key &&key) noexcept {
    auto it = find(key);
    if (it == end()) {
      it = insert({std::move(key), Value()}).first;
    }
    return it->second;
  }

  Value &at(const Key &key) {
    auto it = find(key);
    if (it == end()) {
      throw std::range_error("no key");
    }
    return it->second;
  }

  const Value &at(const Key &key) const {
    return at(key);
  }

 public:
  size_t size() const noexcept {
    return list.size();
  }

  void reserve(size_t sz) {
    size_t new_sz = hash_table.size();
    while (max_load_factor() <= static_cast<double>(sz) / static_cast<double>(new_sz)) {
      new_sz *= 2;
    }
    rehash(sz);
  }

 public:
  template<typename... Args>
  std::pair<iterator, bool> emplace(Args &&... args) {
    NodeType *new_node = allocator.allocate(1);
    try {
      std::allocator_traits<Alloc>::construct(allocator, new_node, std::forward<Args>(args)...);
    } catch (...) {
      allocator.deallocate(new_node, 1);
      throw;
    }

    size_t hash = get_hash(new_node->first);
    iterator it = find(new_node->first);

    if (it != end()) {
      std::allocator_traits<Alloc>::destroy(allocator, new_node);
      allocator.deallocate(new_node, 1);
      return {it, false};
    }

    if (static_cast<double>(size() + 1) / static_cast<double>(hash_table.size()) > mx_load_factor) {
      try {
        rehash(hash_table.size() * 2);
      } catch (...) {
        std::allocator_traits<Alloc>::destroy(allocator, new_node);
        allocator.deallocate(new_node, 1);
      }
    }
    auto &left = hash_table[hash].first;
    auto &right = hash_table[hash].second;
    try {
      if (left == right) {
        list.emplace(begin(), new_node);
        left = begin();
        right = left + 1;
        return {left, true};
      }
      list.emplace(right, new_node);
      return {right++, true};
    } catch (...) {
      std::allocator_traits<Alloc>::destroy(allocator, new_node);
      allocator.deallocate(new_node, 1);
    }
    return {it, false};
  }

  std::pair<iterator, bool> insert(const NodeType &node) {
    return emplace(node);
  }

  std::pair<iterator, bool> insert(NodeType &&node) {
    return emplace(std::move(node));
  }

  template<typename U>
  std::pair<iterator, bool> insert(U &&value) {
    return emplace(std::forward<U>(value));
  }

  template<typename InputIterator>
  void insert(const InputIterator &left, const InputIterator &right) {
    reserve(size() + std::distance(left, right));
    for (auto it = left; it != right; ++it) {
      insert(*it);
    }
  }

  void erase(const iterator &it) noexcept {
    size_t hash = get_hash(it->first);
    auto &left = hash_table[hash].first;
    auto &right = hash_table[hash].second;
    if (left + 1 == right) {
      hash_table[hash] = {end(), end()};
    } else if (it == left) {
      ++left;
    } else if (it + 1 == right) {
      --right;
    }
    list.erase(it);
  }

  template<typename InputIterator>
  void erase(InputIterator left, InputIterator right) noexcept {
    auto cur = left;
    while (left != right) {
      ++left;
      erase(cur);
      cur = left;
    }
  }
};
