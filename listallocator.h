template<size_t size>
class StackStorage {
private:
    void *array;
    size_t shift;
public:
    StackStorage() {
        static std::byte temp[size];
        array = reinterpret_cast<void *>(temp);
        shift = 0;
    }

    StackStorage(const StackStorage &other) = delete;

    template<typename T>
    void *allocate(size_t amount) {
        void *aligned = reinterpret_cast<std::byte *>(array) + shift;
        size_t left = size - shift;
        void *res = std::align(alignof(T[amount]), sizeof(T[amount]), aligned, left);
        if (res) {
            size_t delta = reinterpret_cast<std::byte *>(aligned) - reinterpret_cast<std::byte *>(array);
            shift = delta + sizeof(T[amount]);
            return aligned;
        }
        throw std::bad_alloc();
    }

    void deallocate(void *ptr, size_t amount) {
        std::ignore = ptr;
        std::ignore = amount;
    }
};


template<typename T, size_t size>
class StackAllocator {
public:
    using storage_type = StackStorage<size>;
    using value_type = T;

private:
    storage_type *storage;
public:
    StackAllocator() = default;

    StackAllocator(storage_type &storage) : storage(&storage) {};

    template<typename U>
    StackAllocator(const StackAllocator<U, size> &other_alloc) {
        storage = other_alloc.get_storage();
    }

    template<typename U>
    struct rebind {
        using other = StackAllocator<U, size>;
    };

    StackAllocator &operator=(const StackAllocator &other) {
        storage = other.get_storage();
        return *this;
    }

    value_type *allocate(size_t amount) {
        void *bytes = storage->template allocate<value_type>(amount);
        return reinterpret_cast<value_type *> (bytes);
    }

    void deallocate(value_type *ptr, size_t amount) {
        storage->deallocate(ptr, amount);
    }

    storage_type *get_storage() const { return storage; }

    ~StackAllocator() = default;
};

template<typename T, size_t size>
bool operator==(const StackAllocator<T, size> &al1, const StackAllocator<T, size> &al2) {
    return al1.get_storage() == al2.get_storage();
}

template<typename T, size_t size>
bool operator!=(const StackAllocator<T, size> &al1, const StackAllocator<T, size> &al2) {
    return !(al1 == al2);
}

template<typename T, typename Alloc = std::allocator<T>>
class List {
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
    };

    struct Node : public BaseNode {
        value_type value;

        Node() : value() {}

        Node(const value_type &value) : value(value) {}
    };


    using base_node_alloc_type = typename std::allocator_traits<Alloc>::template rebind_alloc<BaseNode>;
    using node_alloc_type = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;

    using node_alloc_traits = typename std::allocator_traits<node_alloc_type>;
    using value_alloc_traits = typename std::allocator_traits<Alloc>;

    size_t list_size;
    Alloc value_alloc;
    base_node_alloc_type base_node_alloc;
    node_alloc_type node_alloc;
    BaseNode *fake_node;


public:
    iterator begin() { return ++iterator(fake_node); }

    const_iterator begin() const { return ++const_iterator(fake_node); }

    const_iterator cbegin() const { return ++const_iterator(fake_node); }

    iterator end() {
        return iterator(fake_node);
    }

    const_iterator end() const {
        return const_iterator(fake_node);
    }

    const_iterator cend() const {
        return const_iterator(fake_node);
    }

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    auto rbegin() { return std::reverse_iterator<iterator>(end()); }

    auto rend() { return std::reverse_iterator<iterator>(begin()); }

    auto rbegin() const { return std::reverse_iterator<const_iterator>(cend()); }

    auto rend() const { return std::reverse_iterator<const_iterator>(cbegin()); }

    auto crbegin() const { return std::reverse_iterator<iterator>(cend()); }

    auto crend() const { return std::reverse_iterator<iterator>(cbegin()); }

private:
    template<typename... Args>
    void insert_pos(const_iterator it, Args &&... args) {
        Node *new_node = node_alloc.allocate(1);
        try {
            node_alloc_traits::construct(node_alloc, new_node, std::forward<Args>(args)...);
        } catch (...) {
            node_alloc.deallocate(new_node, 1);
            throw;
        }
        new_node->next = it.get_ptr();
        new_node->prev = it.get_ptr()->prev;
        it.get_ptr()->prev->next = new_node;
        it.get_ptr()->prev = new_node;

        ++list_size;
    }

    template<typename... Args>
    void construct_list(size_t sz, Args &&... args) {
        for (int i = 0; i < int(sz); ++i) {
            try {
                insert_pos(end(), std::forward<Args>(args)...);
            } catch (...) {
                clear_list();
                throw;
            }
        }
    }

    void clear_list() {
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

    List(const List &other, const Alloc &alloc) :
        value_alloc(alloc),
        base_node_alloc(base_node_alloc_type(alloc)),
        node_alloc(node_alloc_type(alloc)) {
        copy(other);
    }

public:
    List(const Alloc &alloc = Alloc()) : list_size(0), value_alloc(alloc), base_node_alloc(alloc), node_alloc(alloc) {
        fake_node = base_node_alloc.allocate(1);
        fake_node->next = fake_node;
        fake_node->prev = fake_node;
    };

    List(size_t size, const Alloc &alloc = Alloc()) : List(alloc) {
        construct_list(size);
    }

    List(size_t size, const value_type &val, const Alloc &alloc = Alloc()) : List(alloc) {
        construct_list(size, val);
    }

    List(const List &other) :
            value_alloc(value_alloc_traits::select_on_container_copy_construction(other.value_alloc)),
            base_node_alloc(std::allocator_traits<base_node_alloc_type>::select_on_container_copy_construction(
                    other.base_node_alloc)),
            node_alloc(
                    node_alloc_traits::select_on_container_copy_construction(other.node_alloc)) {
        copy(other);
    }

    void swap(List &other) {
        std::swap(fake_node, other.fake_node);
        std::swap(value_alloc, other.value_alloc);
        std::swap(base_node_alloc, other.base_node_alloc);
        std::swap(node_alloc, other.node_alloc);
        std::swap(list_size, other.list_size);
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

    void insert(const_iterator it, const value_type &val) { insert_pos(it, val); }

    void erase(const_iterator it) {
        auto prev = it.get_ptr()->prev;
        auto next = it.get_ptr()->next;
        node_alloc_traits::destroy(node_alloc, static_cast<Node *>(it.get_ptr()));
        node_alloc.deallocate(static_cast<Node *>(it.get_ptr()), 1);
        next->prev = prev;
        prev->next = next;
        --list_size;
    }

    void push_back(const value_type &val) {
        insert(cend(), val);
    }

    void push_front(const value_type &val) {
        insert(cbegin(), val);
    }

    void pop_back() {
        erase(--cend());
    }

    void pop_front() {
        erase(cbegin());
    }

    size_t size() const {
        return list_size;
    }

    Alloc get_allocator() const {
        return value_alloc;
    }

    ~List() {
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

    base_iterator &operator++() {
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

    BaseNode *get_ptr() { return ptr; }

    bool operator==(const base_iterator<value_type> &other) const {
        return ptr == other.ptr;
    };

    bool operator!=(const base_iterator<value_type> &other) const {
        return !(*this == other);
    }

    value_type &operator*() {
        return static_cast<Node *>(ptr)->value;
    }

    value_type *operator->() { return &(static_cast<Node *>(ptr)->value); }
};
