#include "deque.hpp"

template <class T, typename Allocator>
Deque<T, Allocator>::Deque(const Deque& src)
    : alloc_(alloc_traits::select_on_container_copy_construction(src.alloc_)) {
  for (const auto& elem : src) {
    std::exception_ptr eptr;
    try {
      push_back(elem);
    } catch (...) {
      eptr = std::current_exception();
      clear();
      for (auto& iter : data_) {
        alloc_traits::deallocate(alloc_, iter, kBacketSize);
        iter = nullptr;
      }
      std::rethrow_exception(eptr);
    }
  }
}
template <class T, typename Allocator>
Deque<T, Allocator>::Deque(Deque&& other)
    : begin_(other.begin_),
      end_(other.end_),
      base_(other.base_),
      data_(other.data_),
      alloc_(other.alloc_) {
  other.begin_ = {0, 0};
  other.end_ = {0, 0};
  other.base_ = 0;
  other.data_ = {nullptr};
}
template <class T, typename Allocator>
Deque<T, Allocator>::Deque(std::initializer_list<T> init,
                           const Allocator& alloc)
    : alloc_(alloc) {
  *this = Deque();

  for (const auto& iter : init) {
    push_back(iter);
  }
}
template <class T, typename Allocator>
Deque<T, Allocator>::Deque(size_t amount, const alloc_type& allocator)
    : alloc_(allocator) {
  for (size_t couter = 0; couter < amount; ++couter) {
    std::exception_ptr eptr;
    try {
      ensure_backet(end_.base);
      alloc_traits::construct(alloc_, &*end());
      end_ += 1;
    } catch (...) {
      eptr = std::current_exception();
      clear();
      for (auto& iter : data_) {
        alloc_traits::deallocate(alloc_, iter, kBacketSize);
        iter = nullptr;
      }
      std::rethrow_exception(eptr);
    }
  }
}
template <class T, typename Allocator>
Deque<T, Allocator>::Deque(size_t amount, const T& value,
                           const Allocator& allocator)
    : alloc_(allocator) {
  for (size_t couter = 0; couter < amount; ++couter) {
    std::exception_ptr eptr;
    try {
      push_back(value);
    } catch (...) {
      eptr = std::current_exception();
      clear();
      for (auto& iter : data_) {
        alloc_traits::deallocate(alloc_, iter, kBacketSize);
        iter = nullptr;
      }
      std::rethrow_exception(eptr);
    }
  }
}
template <class T, typename Allocator>
Deque<T, Allocator>& Deque<T, Allocator>::operator=(
    const Deque<T, Allocator>& other) {
  if (alloc_traits::propagate_on_container_copy_assignment::value) {
    alloc_ = other.alloc_;
  }
  Deque<T, Allocator> copy = other;
  std::swap(base_, copy.base_);
  std::swap(begin_, copy.begin_);
  std::swap(data_, copy.data_);
  std::swap(end_, copy.end_);
  return *this;
}
template <class T, typename Allocator>
Deque<T, Allocator>& Deque<T, Allocator>::operator=(
    Deque<T, Allocator>&& other) {
  Deque<T, Allocator> copy = std::move(other);
  std::swap(base_, copy.base_);
  std::swap(begin_, copy.begin_);
  std::swap(data_, copy.data_);
  std::swap(end_, copy.end_);
  return *this;
}
template <class T, typename Allocator>
Deque<T, Allocator>::~Deque() {
  clear();
  for (auto& very_important_iter : data_) {
    alloc_traits::deallocate(alloc_, very_important_iter, kBacketSize);
    very_important_iter = nullptr;
  }
}
template <class T, typename Allocator>
void Deque<T, Allocator>::clear() {
  while (!empty()) {
    pop_back();
  }
}
template <class T, typename Allocator>
T& Deque<T, Allocator>::at(size_t ind) {
  if (!(begin_ + ind < end_)) {
    throw std::out_of_range("out of range");
  }
  return at(begin_ + ind);
}
template <class T, typename Allocator>
const T& Deque<T, Allocator>::at(size_t ind) const {
  if (!(begin_ + ind < end_)) {
    throw std::out_of_range("out of range");
  }
  return at(begin_ + ind);
}
template <class T, typename Allocator>
template <typename... Args>
void Deque<T, Allocator>::emplace_back(Args&&... args) {
  ensure_backet(end_.base);
  alloc_traits::construct(alloc_, &*end(), std::forward<Args>(args)...);
  end_ += 1;
}
template <class T, typename Allocator>
void Deque<T, Allocator>::push_back(const T& value) {
  ensure_backet(end_.base);
  alloc_traits::construct(alloc_, &*end(), value);
  end_ += 1;
}
template <class T, typename Allocator>
void Deque<T, Allocator>::push_back(T&& value) {
  ensure_backet(end_.base);
  alloc_traits::construct(alloc_, &*end(), std::move(value));
  end_ += 1;
}
template <class T, typename Allocator>
template <typename... Args>
void Deque<T, Allocator>::emplace_front(Args&&... args) {
  auto begin = begin_;
  --begin;
  ensure_backet(begin.base);
  alloc_traits::construct(alloc_, &*begin(), std::forward<Args>(args)...);
  --begin_;
}
template <class T, typename Allocator>
void Deque<T, Allocator>::push_front(const T& value) {
  auto begin = begin_;
  --begin;
  ensure_backet(begin.base);
  at(begin) = value;
  --begin_;
}
template <class T, typename Allocator>
void Deque<T, Allocator>::push_front(T&& value) {
  --begin_;
  ensure_backet(begin_.base);
  alloc_traits::construct(alloc_, &*begin(), std::move(value));
}
template <class T, typename Allocator>
void Deque<T, Allocator>::pop_front() {
  alloc_traits::destroy(alloc_, &(at(begin_)));
  begin_ += 1;
}
template <class T, typename Allocator>
typename Deque<T, Allocator>::IterBase&
Deque<T, Allocator>::IterBase::operator+=(ssize_t delta) {
  offset += delta;
  base += offset / kBacketSize;
  offset %= kBacketSize;
  if (offset < 0) {
    offset += kBacketSize;
    base -= 1;
  }
  return *this;
}
template <class T, typename Allocator>
typename Deque<T, Allocator>::IterBase Deque<T, Allocator>::IterBase::operator+(
    ssize_t delta) const {
  auto result = *this;
  return result += delta;
}
template <class T, typename Allocator>
template <bool Const>
typename Deque<T, Allocator>::template Iterator<Const>&
Deque<T, Allocator>::Iterator<Const>::operator=(
    const typename Deque<T, Allocator>::template Iterator<Const>& source) {
  root_ = source.root_;
  base_ = source.base_;
  return *this;
}
template <class T, typename Allocator>
template <bool Const>

typename Deque<T, Allocator>::template Iterator<Const>&
Deque<T, Allocator>::Iterator<Const>::operator+=(ssize_t amount) {
  base_ += amount;
  return *this;
}
template <class T, typename Allocator>
template <bool Const>

typename Deque<T, Allocator>::template Iterator<Const>
Deque<T, Allocator>::Iterator<Const>::operator+(ssize_t amount) const {
  auto copy = *this;
  copy += amount;
  return copy;
}
template <class T, typename Allocator>
template <bool Const>
typename Deque<T, Allocator>::template Iterator<Const>
Deque<T, Allocator>::Iterator<Const>::operator++(int) {
  auto result = *this;
  ++*this;
  return result;
}
template <class T, typename Allocator>
template <bool Const>
typename Deque<T, Allocator>::template Iterator<Const>&
Deque<T, Allocator>::Iterator<Const>::operator-=(ssize_t amount) {
  base_ += -amount;
  return *this;
}
template <class T, typename Allocator>
template <bool Const>

typename Deque<T, Allocator>::template Iterator<Const>
Deque<T, Allocator>::Iterator<Const>::operator-(ssize_t amount) const {
  auto copy = *this;

  copy -= amount;
  return copy;
}
template <class T, typename Allocator>
template <typename... Args>
void Deque<T, Allocator>::emplace(iterator pos, Args&&... args) {
  auto copy = Deque();
  for (auto iter = begin(); iter < pos; ++iter) {
    copy.push_back(*iter);
  }
  copy.emplace_back(std::forward<Args>(args)...);
  for (auto iter = pos; iter < end(); ++iter) {
    copy.push_back(*iter);
  }
  *this = copy;
}
template <class T, typename Allocator>
void Deque<T, Allocator>::insert(iterator pos, const T& value) {
  auto copy = Deque();
  for (auto iter = begin(); iter < pos; ++iter) {
    copy.push_back(*iter);
  }
  copy.push_back(value);
  for (auto iter = pos; iter < end(); ++iter) {
    copy.push_back(*iter);
  }
  *this = copy;
}
template <class T, typename Allocator>
void Deque<T, Allocator>::erase(iterator pos) {
  auto copy = Deque();
  for (auto iter = begin(); iter < pos; ++iter) {
    copy.push_back(*iter);
  }
  for (auto iter = pos + 1; iter < end(); ++iter) {
    copy.push_back(*iter);
  }
  *this = copy;
}
template <class T, typename Allocator>
void Deque<T, Allocator>::ensure_data(ssize_t pos) {
  if (pos + base_ < 0) {
    auto data = std::vector<T*>(data_.size() * 2);
    for (size_t couter = 0; couter < data_.size(); ++couter) {
      data[couter + data_.size()] = data_[couter];
    }
    base_ += data_.size();
    data_ = data;
  }
  if (pos + base_ >= static_cast<ssize_t>(data_.size())) {
    data_.resize(data_.size() * 2);
  }
}