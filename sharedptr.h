#include <iostream>
#include <memory>

template<typename T>
class SharedPtr;

template<typename T>
class WeakPtr;

struct BaseControlBlock {
 private:
  int shared_cnt;
  int weak_cnt;
 public:
  BaseControlBlock(int shared_cnt, int weak_cnt) : shared_cnt(shared_cnt), weak_cnt(weak_cnt) {}

  virtual void useDeleter() noexcept = 0;

  virtual void dealloc_this() noexcept {
    delete this;
  }

  void increment_shared_cnt() {
    ++shared_cnt;
  }

  void decrement_shared_cnt() {
    --shared_cnt;
    if (shared_cnt == 0) {
      useDeleter();
      if (weak_cnt == 0) {
        dealloc_this();
      }
    }
  }

  void increment_weak_cnt() {
    ++weak_cnt;
  }

  void decrement_weak_cnt() {
    --weak_cnt;
    if (shared_cnt == 0 && weak_cnt == 0) {
      dealloc_this();
    }
  }

  int get_shared_cnt() const { return shared_cnt; }

  virtual ~BaseControlBlock() = default;
};

template<typename T, typename Deleter, typename Alloc>
struct ControlBlockRegular : BaseControlBlock {
 private:
  Deleter deleter;
  Alloc alloc;
  T *ptr;

 public:
  ControlBlockRegular(int shared_cnt, int weak_cnt, T *ptr, const Deleter &deleter = Deleter(),
                      const Alloc &alloc = Alloc()) :
      BaseControlBlock(shared_cnt, weak_cnt),
      deleter(deleter),
      alloc(alloc),
      ptr(ptr) {}

  void dealloc_this() noexcept override {
    auto tmp = typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockRegular>(alloc);
    tmp.deallocate(this, 1);
  }

  void useDeleter() noexcept override {
    deleter(ptr);
  }
};

template<typename T, typename Alloc>
struct ControlBlockMakeShared : BaseControlBlock {
  Alloc alloc;
  T object;

  template<typename... Args>
  ControlBlockMakeShared(int shared_cnt, int weak_cnt, Alloc alloc, Args &&... args) :
      BaseControlBlock(shared_cnt, weak_cnt), alloc(alloc), object(std::forward<Args>(args)...) {};


  void useDeleter() noexcept override {
    std::allocator_traits<Alloc>::destroy(alloc, &object);
  }

  T *get_ptr() { return &object; }

  void dealloc_this() noexcept override {
    auto tmp = typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockMakeShared>(alloc);
    tmp.deallocate(this, 1);
  }
};

template<typename T, typename Alloc, typename... Args>
SharedPtr<T> allocateShared(const Alloc &alloc, Args &&... args) {
  using AllocControlBlock = typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockMakeShared<T, Alloc>>;
  AllocControlBlock AllocCB = alloc;
  auto new_block = AllocCB.allocate(1);
  std::allocator_traits<AllocControlBlock>::construct(AllocCB, new_block, 1, 0, alloc, std::forward<Args>(args)...);
  return SharedPtr<T>(new_block);
}

template<typename T, typename Deleter, typename Alloc>
auto allocateRegular(T *ptr, const Deleter &deleter, const Alloc &alloc) {
  using AllocControlBlock = typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockRegular<T, Deleter, Alloc>>;
  AllocControlBlock AllocCB = alloc;
  auto new_block = AllocCB.allocate(1);
  new(new_block) ControlBlockRegular(1, 0, ptr, deleter, alloc);
  return new_block;
}

template<typename U, typename... Args>
SharedPtr<U> makeShared(Args &&... args) {
  return allocateShared<U, std::allocator<U>>(std::allocator<U>(), std::forward<Args>(args)...);
}

template<typename T>
class EnableSharedFromThis {
  template<typename U>
  friend
  class SharedPtr;

 private:
  WeakPtr<T> wptr;
 public:
  SharedPtr<T> shared_from_this() const noexcept {
    return wptr.lock();
  }
};

template<typename T>
class SharedPtr {
  template<typename U>
  friend
  class WeakPtr;

  template<typename U>
  friend
  class SharedPtr;

 private:
  T *ptr = nullptr;
  BaseControlBlock *cb = nullptr;
 public:
  SharedPtr() = default;

  template<typename U, typename Deleter, typename Alloc>
  SharedPtr(U *ptr, const Deleter &deleter, const Alloc &alloc) : ptr(ptr), cb(allocateRegular(ptr, deleter, alloc)) {
    if constexpr (std::is_base_of_v<EnableSharedFromThis<U>, U>) {
      ptr->wptr = *this;
    }
  }

  template<typename U>
  SharedPtr(U *ptr) :
      SharedPtr(ptr, std::default_delete<U>(), std::allocator<U>()) {}

  template<typename U, typename Deleter>
  SharedPtr(U *ptr, const Deleter &deleter) : ptr(ptr), cb(allocateRegular<U>(ptr, deleter, std::allocator<U>())) {
    if constexpr (std::is_base_of_v<EnableSharedFromThis<U>, U>) {
      ptr->wptr = *this;
    }
  }

  template<typename U>
  SharedPtr(const SharedPtr<U> &other) :
      ptr(other.ptr), cb(reinterpret_cast<BaseControlBlock *>(other.cb)) {
    cb->increment_shared_cnt();
  }

  SharedPtr(const SharedPtr &other) : ptr(other.ptr), cb(other.cb) {
    if (cb) {
      cb->increment_shared_cnt();
    }
  }

  template<typename U>
  SharedPtr(SharedPtr<U> &&other) noexcept : ptr(other.ptr), cb(reinterpret_cast<BaseControlBlock *>(other.cb)) {
    cb->increment_shared_cnt();
    ptr = other.get();
    other.reset();
  }

  SharedPtr(SharedPtr &&other) noexcept: ptr(other.ptr), cb(other.cb) {
    other.ptr = nullptr;
    other.cb = nullptr;
  }

  template<typename U>
  SharedPtr &operator=(const SharedPtr &other) {
    auto tmp_ptr = SharedPtr(other);
    swap(tmp_ptr);
    return *this;
  }

  SharedPtr &operator=(const SharedPtr &other) {
    auto tmp_ptr = SharedPtr(other);
    swap(tmp_ptr);
    return *this;
  }

  template<typename U>
  SharedPtr &operator=(SharedPtr<U> &&other) noexcept {
    auto tmp_ptr = SharedPtr(std::move(other));
    swap(tmp_ptr);
    return *this;
  }

  SharedPtr &operator=(SharedPtr &&other) noexcept {
    auto tmp_ptr = SharedPtr(std::move(other));
    swap(tmp_ptr);
    return *this;
  }

  ~SharedPtr() {
    if (cb) {
      cb->decrement_shared_cnt();
    }
  }

 public:
  template<typename U, typename... Args>
  friend SharedPtr<U> makeShared(Args &&... args);

  template<typename U, typename Alloc, typename... Args>
  friend auto allocateShared(const Alloc &alloc, Args &&... args);

  template<typename Alloc>
  SharedPtr(ControlBlockMakeShared<T, Alloc> *cb) : ptr(cb->get_ptr()), cb(cb) {}

  SharedPtr(const WeakPtr<T> &weak_ptr) : ptr(weak_ptr.ptr), cb(weak_ptr.cb) {
    cb->increment_shared_cnt();
  }

 public:
  template<typename U>
  void reset(U *new_ptr) { SharedPtr<U>(new_ptr).swap(*this); }

  void reset() noexcept { SharedPtr().swap(*this); }

  int use_count() const { return cb->get_shared_cnt(); }

  T *operator->() noexcept { return ptr; }

  T *operator->() const noexcept { return ptr; }

  T &operator*() noexcept { return *ptr; }

  T &operator*() const noexcept { return *ptr; }

  T *get() noexcept { return ptr; }

  T *get() const noexcept { return ptr; }

  template<typename U>
  void swap(SharedPtr<U> &other) {
    std::swap(ptr, other.ptr);
    std::swap(cb, other.cb);
  }
};

template<typename T>
class WeakPtr {
  template<typename U>
  friend
  class SharedPtr;

  template<typename U>
  friend
  class WeakPtr;

 private:
  BaseControlBlock *cb = nullptr;
  T *ptr = nullptr;

 public:
  WeakPtr() = default;

  template<typename U>
  WeakPtr(const SharedPtr<U> &shared_ptr): cb(reinterpret_cast<BaseControlBlock *>(shared_ptr.cb)),
                                           ptr(shared_ptr.ptr) {
    cb->increment_weak_cnt();
  }

  template<typename U>
  WeakPtr(const WeakPtr<U> &other): cb(reinterpret_cast<BaseControlBlock *>(other.cb)), ptr(other.ptr) {
    cb->increment_weak_cnt();
  }

  WeakPtr(const WeakPtr &other) : cb(other.cb), ptr(other.ptr) {
    cb->increment_weak_cnt();
  }

  template<typename U>
  WeakPtr(WeakPtr<U> &&other): cb(other.cb), ptr(other.ptr) {
    other.cb = nullptr;
    other.ptr = nullptr;
  }

  template<typename U>
  WeakPtr &operator=(const WeakPtr<U> &other) {
    auto tmp_ptr = WeakPtr<T>(other);
    std::swap(tmp_ptr, *this);
    return *this;
  }

  template<typename U>
  WeakPtr &operator=(WeakPtr<U> &&other) {
    std::swap(WeakPtr<T>(std::move(other)), *this);
    return *this;
  }

  template<typename U>
  WeakPtr &operator=(const SharedPtr<U> &shared_ptr) {
    auto tmp_ptr = WeakPtr<T>(shared_ptr);
    tmp_ptr.swap(*this);
    return *this;
  }

  ~WeakPtr() {
    if (cb) {
      cb->decrement_weak_cnt();
    }
  }

 public:
  bool expired() const noexcept {
    return cb && cb->get_shared_cnt() == 0;
  }

  SharedPtr<T> lock() const noexcept {
    if (!expired()) {
      return SharedPtr<T>(*this);
    }
    return SharedPtr<T>();
  }

  int use_count() const noexcept { return cb->get_shared_cnt(); }

  template<typename U>
  void swap(WeakPtr<U> &other) {
    std::swap(cb, other.cb);
    std::swap(ptr, other.ptr);
  }
};
