#include <exception>
#include <iostream>
#include <iterator>
#include <type_traits>
#include <vector>

template <class T, typename Allocator = std::allocator<T>>
class Deque {
  using alloc_type = Allocator;
  using alloc_traits = std::allocator_traits<alloc_type>;
  static const ssize_t kBacketSize = 8;
  alloc_type alloc_;

 public:
  Deque() = default;
  Deque(const Allocator& allocator) : alloc_(allocator) {}
  Deque(const Deque& src);
  const Allocator& get_allocator() const { return alloc_; }
  Allocator& get_allocator() { return alloc_; }
  Deque(Deque&& other);
  Deque(std::initializer_list<T> init, const Allocator& alloc = Allocator());
  Deque(size_t amount, const alloc_type& allocator = Allocator());
  Deque(size_t amount, const T& value,
        const Allocator& allocator = Allocator());
  Deque& operator=(const Deque& other);
  Deque& operator=(Deque&& other);

  ~Deque();
  void clear();

  size_t size() const { return end_ - begin_; }
  bool empty() const { return size() == 0; }

  T& operator[](size_t ind) { return at(begin_ + ind); }
  const T& operator[](size_t ind) const { return at(begin_ + ind); }
  T& at(size_t ind);
  const T& at(size_t ind) const;

  template <typename... Args>
  void emplace_back(Args&&... args);

  void push_back(const T& value);
  void push_back(T&& value);

  template <typename... Args>
  void emplace_front(Args&&... args);

  void push_front(const T& value);
  void push_front(T&& value);
  void pop_back() { alloc_traits::destroy(alloc_, &(at(--end_))); }
  void pop_front();

 private:
  struct IterBase {
    ssize_t base;
    ssize_t offset;

    bool operator==(const IterBase& other) const {
      return base == other.base && offset == other.offset;
    }
    bool operator<(const IterBase& other) const {
      return base < other.base || base == other.base && offset < other.offset;
    }
    IterBase& operator+=(ssize_t delta);
    IterBase& operator++() { return *this += 1; }
    IterBase& operator--() { return *this += -1; }

    ssize_t operator-(IterBase begin) const {
      return (base - begin.base) * kBacketSize + (offset - begin.offset);
    }

    IterBase operator+(ssize_t delta) const;
  };

  template <bool Const>
  class Iterator : public std::iterator<std::random_access_iterator_tag,
                                        std::conditional_t<Const, const T, T>> {
    using root = std::conditional_t<Const, const Deque*, Deque*>;
    using value = std::conditional_t<Const, const T, T>;

   public:
    Iterator(root root, IterBase base) : root_(root), base_(base) {}
    Iterator(const Iterator& source)
        : root_(source.root_), base_(source.base_) {}
    Iterator(Iterator&& source) : root_(source.root_), base_(source.base_) {}
    Iterator& operator=(const Iterator& source);

    bool operator==(const Iterator& other) const {
      return base_ == other.base_ && root_ == other.root_;
    }
    bool operator!=(const Iterator& other) { return !(*this == other); }

    bool operator<(const Iterator& other) { return base_ < other.base_; }
    bool operator>(const Iterator& other) { return other.base_ < base_; }
    bool operator<=(const Iterator& other) { return !(other.base_ < base_); }
    bool operator>=(const Iterator& other) { return !(base_ < other.base_); }

    Iterator& operator+=(ssize_t amount);
    Iterator& operator++() { return (*this += 1); }
    Iterator operator+(ssize_t amount) const;
    Iterator operator++(int);

    Iterator& operator-=(ssize_t amount);
    Iterator& operator--() { return (*this -= 1); }
    Iterator operator-(ssize_t amount) const;
    ssize_t operator-(Iterator other) const { return base_ - other.base_; }
    value& operator*() { return root_->at(base_); }
    const value& operator*() const { return root_->at(base_); }
    value* operator->() { return &root_->at(base_); }
    const value* operator->() const { return &root_->at(base_); }

   private:
    IterBase base_;
    root root_;
  };

 public:
  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  auto begin() { return iterator(this, begin_); }
  auto end() { return iterator(this, end_); }
  auto begin() const { return const_iterator(this, begin_); }
  auto end() const { return const_iterator(this, end_); }
  auto cbegin() const { return begin(); }
  auto cend() const { return end(); }
  auto rbegin() { return reverse_iterator(end()); }
  auto rend() { return reverse_iterator(begin()); }
  auto rbegin() const { return const_reverse_iterator(end()); }
  auto rend() const { return const_reverse_iterator(begin()); }
  auto crbegin() const { return rbegin(); }
  auto crend() const { return rend(); }

  template <typename... Args>
  void emplace(iterator pos, Args&&... args);

  void insert(iterator pos, const T& value);
  void erase(iterator pos);

 private:
  T& at(IterBase pos) { return data_[pos.base + base_][pos.offset]; }
  const T& at(IterBase pos) const {
    return data_[pos.base + base_][pos.offset];
  }

  void ensure_backet(ssize_t pos) {
    ensure_data(pos);
    if (data_[pos + base_] == nullptr) {
      data_[pos + base_] = alloc_traits::allocate(alloc_, kBacketSize);
    }
  }
  void ensure_data(ssize_t pos);
  IterBase begin_ = {0, 0}, end_ = {0, 0};
  ssize_t base_ = 0;
  std::vector<T*> data_ = {nullptr};
};
