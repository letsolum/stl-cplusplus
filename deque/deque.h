#include <stdlib.h>

#include <iterator>
#include <type_traits>
#include <vector>

template <typename T>
class Deque {
  private:
    struct Pos {
        int row, col;

        Pos() = default;
        Pos(const std::pair<int, int>& x);
        Pos(int row_, int col_);
        Pos(int index);

        Pos& operator++();
        Pos operator++(int);
        Pos& operator--();
        Pos operator--(int);
        Pos& operator=(const std::pair<int, int>& x);
        Pos& operator+=(int add);

        bool operator==(const Pos& second);
        bool operator==(const Pos& second) const;

        int get() {
            return row * block_size + col;
        }
        int get() const {
            return row * block_size + col;
        }
    };

    std::vector<T*> data;
    size_t size_;
    size_t cap;
    Pos start_pos, end_pos;
    static const int block_size = 64;

    void Delete() {
        for (int i = start_pos.get(); i < end_pos.get(); ++i) {
            (data[i / block_size] + (i % block_size))->~T();
        }
        for (size_t i = 0; i < cap; ++i) {
            delete[] reinterpret_cast<char*>(data[i]);
        }
        data.clear();
    }

    void reallocate() {
        cap *= 3;
        std::vector<T*> new_data(cap);
        if (cap == 0) {
            cap = 1;
            new_data.resize(cap);
            new_data[0] =
                reinterpret_cast<T*>(new char[block_size * sizeof(T)]);
            start_pos = Pos(0, 31);
            end_pos = start_pos;
            data = std::move(new_data);
            return;
        }
        for (size_t i = 0; i < cap; ++i) {
            if (i < cap / 3 || i >= cap / 3 * 2) {
                new_data[i] =
                    reinterpret_cast<T*>(new char[block_size * sizeof(T)]);
            } else {
                new_data[i] = std::move(data[i - (cap / 3)]);
            }
        }
        end_pos.row += cap / 3;
        start_pos.row += cap / 3;
        data = std::move(new_data);
    }

  public:
    using value_type = T;
    template <typename Value>
    class base_iterator {
      private:
        friend Deque;
        const std::vector<T*>* deque;
        T* bucket;
        Pos pos;
        base_iterator(const std::vector<T*>& second, Pos cur)
            : deque(&second), bucket(nullptr), pos(cur) {
            if (cur.row < static_cast<int>(second.size())) {
                bucket = second[cur.row];
            }
        }

      public:
        using value_type = Value;
        using reference = value_type&;
        using pointer = value_type*;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;
        base_iterator() = default;
        base_iterator(const base_iterator&) = default;

        base_iterator& operator=(const base_iterator& second) = default;

        reference operator*() {
            return bucket[pos.col];
        }
        pointer operator->() const {
            return &bucket[pos.col];
        }
        operator base_iterator<const Value>() const {
            base_iterator<const Value> cp(deque, pos, bucket);
            return base_iterator<const Value>(cp);
        }

        void bucket_update() {
            if (deque != nullptr && !deque->empty()) {
                bucket = (*deque)[pos.row];
            }
        }

        base_iterator& operator+=(int dst) {
            pos += dst;
            bucket_update();
            return *this;
        }

        base_iterator& operator-=(int dst) {
            pos += (-dst);
            bucket_update();
            return *this;
        }

        base_iterator& operator++() {
            pos++;
            bucket_update();
            return *this;
        }

        base_iterator& operator--() {
            pos--;
            bucket_update();
            return *this;
        }

        base_iterator operator++(int) {
            auto copy = *this;
            ++pos;
            bucket_update();
            return copy;
        }

        base_iterator operator--(int) {
            auto copy = *this;
            --pos;
            bucket_update();
            return copy;
        }

        base_iterator operator+(const int dst) const {
            auto copy = *this;
            copy += dst;
            return copy;
        }

        base_iterator operator-(const int dst) const {
            auto copy = *this;
            copy -= dst;
            return copy;
        }

        difference_type operator-(const base_iterator& second) const {
            return pos.get() - second.pos.get();
        }

        bool operator==(const base_iterator& second) const {
            bool fir = (deque == second.deque), sec = (pos == second.pos);
            return fir && sec;
        }

        bool operator!=(const base_iterator& second) {
            return !(*this == second);
        }
        bool operator<(const base_iterator& second) {
            return (*this - second) < 0;
        }
        bool operator>(const base_iterator& second) {
            return (*this - second) > 0;
        }
        bool operator<=(const base_iterator& second) {
            return !(*this > second);
        }
        bool operator>=(const base_iterator& second) {
            return !(*this < second);
        }
    };

    void swap(Deque<T>& second);
    Deque();
    Deque(int size_);
    explicit Deque(int count, const T& value);
    Deque(const Deque<T>& second);
    Deque<T>& operator=(const Deque<T>& second);
    T& operator[](int index);
    const T& operator[](int index) const;
    T& at(size_t index);
    const T& at(size_t index) const;

    void push_back(const T& value);
    void pop_back();
    void push_front(const T& value);
    void pop_front();

    size_t size() const;
    ~Deque() {
        Delete();
    }

    using iterator = base_iterator<T>;
    using const_iterator = base_iterator<const T>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

    const_iterator cbegin() const;
    const_iterator cend() const;

    reverse_iterator rbegin();
    reverse_iterator rend();

    const_reverse_iterator crbegin();
    const_reverse_iterator crend();

    void insert(iterator it, const T& value);
    void erase(iterator it);
};

template <typename T>
Deque<T>::Pos::Pos(const std::pair<int, int>& x)
    : row(x.first), col(x.second) {}

template <typename T>
Deque<T>::Pos::Pos(int row_, int col_) : row(row_), col(col_) {}

template <typename T>
Deque<T>::Pos::Pos(int index)
    : row(index / block_size), col(index % block_size) {}

template <typename T>
typename Deque<T>::Pos& Deque<T>::Pos::operator++() {
    if (++col == block_size) {
        ++row;
        col = 0;
    }
    return *this;
}

template <typename T>
typename Deque<T>::Pos Deque<T>::Pos::operator++(int) {
    auto copy = *this;
    if (++col == block_size) {
        ++row;
        col = 0;
    }
    return copy;
}

template <typename T>
typename Deque<T>::Pos& Deque<T>::Pos::operator--() {
    if (--col < 0) {
        --row;
        col = block_size - 1;
    }
    return *this;
}

template <typename T>
typename Deque<T>::Pos Deque<T>::Pos::operator--(int) {
    auto copy = *this;
    if (--col < 0) {
        --row;
        col = block_size - 1;
    }
    return copy;
}

template <typename T>
typename Deque<T>::Pos& Deque<T>::Pos::operator=(const std::pair<int, int>& x) {
    col = x.first, row = x.second;
    return *this;
}

template <typename T>
typename Deque<T>::Pos& Deque<T>::Pos::operator+=(int add) {
    row += (col + add) / block_size;
    col = (col + add) % block_size;
    if (col < 0) {
        row--;
    }
    col = (col + block_size) % block_size;
    return *this;
}

template <typename T>
bool Deque<T>::Pos::operator==(const Pos& second) {
    return row == second.row && col == second.col;
}

template <typename T>
bool Deque<T>::Pos::operator==(const Pos& second) const {
    return row == second.row && col == second.col;
}

template <typename T>
Deque<T>::Deque()
    : size_(0), cap(0), start_pos(Pos(0, 0)), end_pos(Pos(0, 0)) {}

template <typename T>
Deque<T>::Deque(int size_) : size_(size_) {
    cap = (size_ / block_size + 2) << 1;
    data.resize(cap);
    start_pos.row = (cap >> 1) - ((size_ >> 1) / block_size + 1);
    start_pos.col = block_size - (size_ >> 1) % block_size - 1;
    end_pos = start_pos;
    for (size_t i = 0; i < cap; ++i) {
        data[i] = reinterpret_cast<T*>(new char[block_size * sizeof(T)]);
    }
    if constexpr (std::is_default_constructible<T>::value) {
        try {
            for (; end_pos.get() - start_pos.get() < size_; ++end_pos) {
                new (data[end_pos.row] + end_pos.col) T();
            }
        } catch (...) {
            Delete();
            size_ = 0;
            cap = 0;
            start_pos = Pos(0, 0);
            throw;
        }
    } else {
        start_pos += size_;
        end_pos = start_pos;
        start_pos += (-size_);
    }
}

template <typename T>
Deque<T>::Deque(const int count, const T& value)
    : size_(0), cap(0), start_pos(0, 0), end_pos(0, 0) {
    *this = Deque<T>(count);
    Pos now = start_pos;
    for (size_t i = 0; i < size_; ++i) {
        data[now.row][now.col] = value;
        ++now;
    }
}

template <typename T>
Deque<T>::Deque(const Deque<T>& second) {
    data.resize(second.cap);
    size_ = second.size_;
    cap = second.cap;
    start_pos = second.start_pos;
    end_pos = second.end_pos;
    for (size_t i = 0; i < cap; i++) {
        data[i] = reinterpret_cast<T*>(new char[block_size * sizeof(T)]);
        std::uninitialized_copy(second.data[i], second.data[i] + block_size,
                                data[i]);
    }
}

template <typename T>
void Deque<T>::swap(Deque<T>& second) {
    std::swap(second.data, data);
    std::swap(second.cap, cap);
    std::swap(second.size_, size_);
    std::swap(second.start_pos, start_pos);
    std::swap(second.end_pos, end_pos);
}

template <typename T>
Deque<T>& Deque<T>::operator=(const Deque<T>& second) {
    Deque<T> copy(second);
    swap(copy);
    return *this;
}

template <typename T>
T& Deque<T>::operator[](int index) {
    return data[start_pos.row + (start_pos.col + index) / block_size]
               [(start_pos.col + index) % block_size];
}

template <typename T>
const T& Deque<T>::operator[](int index) const {
    return data[start_pos.row + (start_pos.col + index) / block_size]
               [(start_pos.col + index) % block_size];
}

template <typename T>
T& Deque<T>::at(size_t index) {
    if (index >= size_ || index < 0) {
        throw std::out_of_range("");
    } else {
        return (*this)[index];
    }
}

template <typename T>
const T& Deque<T>::at(size_t index) const {
    if (index >= size_ || index < 0) {
        throw std::out_of_range("");
    }
    return (*this)[index];
}

template <typename T>
void Deque<T>::pop_back() {
    if (size_ != 0) {
        --end_pos;
        data[end_pos.row][end_pos.col].~T();
        --size_;
    }
}

template <typename T>
void Deque<T>::pop_front() {
    if (size_ != 0) {
        data[start_pos.row][start_pos.col].~T();
        ++start_pos;
        --size_;
    }
}

template <typename T>
void Deque<T>::push_back(const T& value) {
    ++size_;
    if (cap == 0 || (static_cast<size_t>(end_pos.row) == cap - 1 &&
                     static_cast<size_t>(end_pos.col) == block_size - 1)) {
        reallocate();
    }
    new (data[end_pos.row] + end_pos.col) T(value);
    ++end_pos;
}

template <typename T>
void Deque<T>::push_front(const T& value) {
    ++size_;
    if (start_pos.row == 0 && start_pos.col == 0) {
        reallocate();
    }
    --start_pos;
    data[start_pos.row][start_pos.col] = value;
}

template <typename T>
size_t Deque<T>::size() const {
    return size_;
}

template <typename T>
typename Deque<T>::iterator Deque<T>::begin() {
    return iterator(data, start_pos);
}

template <typename T>
typename Deque<T>::iterator Deque<T>::end() {
    return iterator(data, end_pos);
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::begin() const {
    return const_iterator(data, start_pos);
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::end() const {
    return const_iterator(data, end_pos);
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::cbegin() const {
    return const_iterator(data, start_pos);
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::cend() const {
    return const_iterator(data, end_pos);
}

template <typename T>
typename Deque<T>::reverse_iterator Deque<T>::rbegin() {
    return std::reverse_iterator(end());
}

template <typename T>
typename Deque<T>::reverse_iterator Deque<T>::rend() {
    return std::reverse_iterator(begin());
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::crbegin() {
    return std::make_reverse_iterator(begin());
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::crend() {
    return std::make_reverse_iterator(end());
}

template <typename T>
void Deque<T>::insert(iterator it, const T& value) {
    T now = value;
    while (it < end()) {
        std::swap(now, *it);
        ++it;
    }
    (*this).push_back(now);
}

template <typename T>
void Deque<T>::erase(iterator it) {
    while (it + 1 < end()) {
        *it = *(it + 1);
        ++it;
    }
    (*this).pop_back();
}