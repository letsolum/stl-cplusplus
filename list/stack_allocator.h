#include <cstddef>
#include <iostream>
#include <type_traits>

template <size_t N>
class StackStorage {
  private:
    char data[N];
    size_t cur_pos = 0;

  public:
    StackStorage() = default;
    StackStorage(const StackStorage&) = delete;
    StackStorage& operator=(const StackStorage&) = delete;

    void* equalizing(size_t num, size_t size_of) {
        size_t ost = reinterpret_cast<size_t>(data + cur_pos) % size_of;

        cur_pos += (size_of - ost) % size_of;

        cur_pos += num;
        void* rez = data + (cur_pos - num);

        return rez;
    }
};

template <typename T, size_t N>
class StackAllocator {
  private:
    StackStorage<N>* pool;

  public:
    using value_type = T;

    StackAllocator(){};
    ~StackAllocator(){};

    StackAllocator(StackStorage<N>& strg)
        : pool(&strg) {}

    template <typename U>
    StackAllocator(const StackAllocator<U, N>& other)
        : pool(other.get_storage()) {}

    template <typename U>
    StackAllocator& operator=(const StackAllocator<U, N>& other) {
        pool = other.get_storage();

        return *this;
    }

    T* allocate(size_t count) {
        void* ptr = pool->equalizing(sizeof(T) * count, alignof(T));

        return reinterpret_cast<T*>(ptr);
    }

    StackStorage<N>* get_storage() const {
        return pool;
    }

    void deallocate(T* /*unused*/, size_t /*unused*/) {}

    template <typename U>
    struct rebind {
        using other = StackAllocator<U, N>;
    };
};

template <typename T, size_t N, typename U, size_t M>
bool operator==(const StackAllocator<T, N>& first,
                const StackAllocator<U, M>& second) {
    return first.get_storage() == second.get_storage();
}