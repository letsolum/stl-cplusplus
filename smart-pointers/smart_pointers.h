#include <iostream>
#include <memory>
#include <type_traits>

struct BaseControlBlock {
    size_t cnt_shared;
    size_t cnt_weak;

    BaseControlBlock(size_t cnt_s, size_t cnt_w)
        : cnt_shared(cnt_s), cnt_weak(cnt_w) {}

    virtual void deallocate_cb() = 0;
    virtual void destroy_object() = 0;
    virtual ~BaseControlBlock() = default;
};

template <class T, class Deleter = std::default_delete<T>,
          class Alloc = std::allocator<T>>
struct ControlBlockRegular : BaseControlBlock {
    T* object;
    Deleter del;
    Alloc alloc;

    ControlBlockRegular(size_t sc, size_t wc, T* ptr, const Deleter& d, Alloc a)
        : BaseControlBlock(sc, wc), object(ptr), del(d), alloc(a) {}

    ~ControlBlockRegular() override = default;

    void deallocate_cb() override {
        using Obj = ControlBlockRegular<T, Deleter, Alloc>;
        using ObjAlloc =
            typename std::allocator_traits<Alloc>::template rebind_alloc<Obj>;
        ObjAlloc cb_alloc = alloc;
        auto casted = dynamic_cast<Obj*>(this);
        std::allocator_traits<ObjAlloc>::deallocate(cb_alloc, casted, 1);
    }

    void destroy_object() override {
        del(object);
    }
};

template <typename T, typename Alloc = std::allocator<T>>
struct MakeSharedControlBlock : BaseControlBlock {
    Alloc alloc;
    T object;

    template <typename... Args>
    MakeSharedControlBlock(size_t sc, size_t wc, Alloc a, Args&&... args)
        : BaseControlBlock(sc, wc),
          alloc(a),
          object(std::forward<Args>(args)...) {}

    void deallocate_cb() override {
        using Obj = MakeSharedControlBlock<T, Alloc>;
        using ObjAlloc =
            typename std::allocator_traits<Alloc>::template rebind_alloc<Obj>;
        ObjAlloc cb_alloc = alloc;
        auto casted = dynamic_cast<Obj*>(this);
        std::allocator_traits<ObjAlloc>::deallocate(cb_alloc, casted, 1);
    }

    ~MakeSharedControlBlock() override = default;

    void destroy_object() override {
        std::allocator_traits<Alloc>::destroy(alloc, &object);
    }
};

template <typename T>
class WeakPtr;

template <typename T>
class SharedPtr {
  private:
    template <typename Alloc>
    SharedPtr(MakeSharedControlBlock<T, Alloc>* make_shared_cb)
        : object(nullptr), cb(make_shared_cb) {}

    SharedPtr(WeakPtr<T> wptr);

    void clear();

    T* object = nullptr;
    BaseControlBlock* cb = nullptr;

    template <typename U>
    friend class SharedPtr;

    template <typename U>
    friend class WeakPtr;

    template <typename U, typename Alloc, typename... Args>
    friend SharedPtr<U> allocateShared(Alloc, Args&&...);

    template <typename U, typename... Args>
    friend SharedPtr<U> makeShared(Args&&...);

  public:
    template <typename Deleter = std::default_delete<T>,
              typename Alloc = std::allocator<T>>
    SharedPtr(T* ptr, const Deleter& del = Deleter(), Alloc alloc = Alloc())
        : object(ptr) {
        using AllocTr = typename std::allocator_traits<Alloc>;
        using RegularConBT = ControlBlockRegular<T, Deleter, Alloc>;
        using RegularConBloAllTr =
            typename AllocTr::template rebind_traits<RegularConBT>;
        using RegularConBA =
            typename AllocTr::template rebind_alloc<RegularConBT>;

        RegularConBA cb_alloc = alloc;
        auto regular_cb = RegularConBloAllTr::allocate(cb_alloc, 1);
        new (regular_cb) RegularConBT(1, 0, ptr, del, cb_alloc);
        cb = regular_cb;
    }

    template <typename U, typename Deleter = std::default_delete<T>,
              typename Alloc = std::allocator<T>>
    SharedPtr(T* ptr, const Deleter& del = Deleter(), Alloc alloc = Alloc())
        : object(ptr) {
        using AllocTr = typename std::allocator_traits<Alloc>;
        using RegularConBT = ControlBlockRegular<U, Deleter, Alloc>;
        using RegularConBloAllTr =
            typename AllocTr::template rebind_traits<RegularConBT>;
        using RegularConBA =
            typename AllocTr::template rebind_alloc<RegularConBT>;
        RegularConBA cb_alloc = alloc;
        auto regular_cb = RegularConBloAllTr::allocate(cb_alloc, 1);
        new (regular_cb) RegularConBT(1, 0, ptr, del, cb_alloc);
        cb = regular_cb;
    }

    SharedPtr() {}

    SharedPtr(const SharedPtr& shptr) noexcept;

    SharedPtr& operator=(SharedPtr&& shptr) noexcept;

    SharedPtr& operator=(const SharedPtr& shptr) noexcept;

    T& operator*() const noexcept;

    T* operator->() const noexcept;

    size_t use_count() const;

    T* get() const;

    void reset();

    void swap(SharedPtr& other);

    template <typename U>
    SharedPtr(const SharedPtr<U>& shptr) noexcept
        : object(shptr.object), cb(shptr.cb) {
        if (cb == nullptr) {
            return;
        }
        ++cb->cnt_shared;
    }

    template <typename U>

    SharedPtr& operator=(const SharedPtr<U>& shptr) noexcept {
        SharedPtr copy(shptr);
        swap(copy);
        return *this;
    }

    template <typename U>

    SharedPtr& operator=(SharedPtr<U>&& shptr) noexcept {
        SharedPtr copy(std::move(shptr));
        swap(copy);
        return *this;
    }

    template <typename U>

    SharedPtr(SharedPtr<U>&& shptr) noexcept
        : object(shptr.object), cb(shptr.cb) {
        shptr.cb = nullptr;
        shptr.object = nullptr;
    }

    template <typename U, typename Deleter = std::default_delete<T>,
              typename Alloc = std::allocator<T>>

    void reset(U* ptr, Deleter del = Deleter(), Alloc alloc = Alloc()) {
        SharedPtr<T>(ptr, del, alloc).swap(*this);
    }

    ~SharedPtr() {
        clear();
    }
};

template <typename T>
SharedPtr<T>::SharedPtr(WeakPtr<T> wptr) : object(wptr.object), cb(wptr.cb) {
    if (cb != nullptr) {
        ++cb->cnt_shared;
    }
}

template <typename T>
void SharedPtr<T>::clear() {
    if (cb != nullptr) {
        --cb->cnt_shared;
        if (cb->cnt_shared == 0) {
            cb->destroy_object();
            if (cb->cnt_weak == 0) {
                cb->deallocate_cb();
            }
        }
    }
}

template <typename T>
SharedPtr<T>::SharedPtr(const SharedPtr& shptr) noexcept
    : object(shptr.object), cb(shptr.cb) {
    if (cb != nullptr) {
        ++cb->cnt_shared;
    }
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr&& shptr) noexcept {
    SharedPtr copy(std::move(shptr));
    swap(copy);
    return *this;
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr& shptr) noexcept {
    if (this != &shptr) {
        SharedPtr copy(shptr);
        swap(copy);
    }
    return *this;
}

template <typename T>
T& SharedPtr<T>::operator*() const noexcept {
    if (!object) {
        return static_cast<MakeSharedControlBlock<T>*>(cb)->object;
    }
    return *object;
}

template <typename T>
T* SharedPtr<T>::operator->() const noexcept {
    if (!object) {
        return &(static_cast<MakeSharedControlBlock<T>*>(cb)->object);
    }
    return object;
}

template <typename T>
size_t SharedPtr<T>::use_count() const {
    return cb->cnt_shared;
}

template <typename T>
T* SharedPtr<T>::get() const {
    return object;
}

template <typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {
    auto cb = new MakeSharedControlBlock<T, std::allocator<T>>(
        1, 0, std::allocator<T>(), std::forward<Args>(args)...);
    return SharedPtr<T>(cb);
}

template <typename T>
void SharedPtr<T>::reset() {
    SharedPtr<T>().swap(*this);
}

template <typename T>
void SharedPtr<T>::swap(SharedPtr& other) {
    std::swap(cb, other.cb);
    std::swap(object, other.object);
}

template <typename T, typename Alloc, typename... Args>
SharedPtr<T> allocateShared(Alloc alloc, Args&&... args) {
    using AllocTr = typename std::allocator_traits<Alloc>;
    using MakeSharedBlockAllocTraits = typename AllocTr::template rebind_traits<
        MakeSharedControlBlock<T, Alloc>>;
    using MakeSharedBlockAlloc = typename AllocTr::template rebind_alloc<
        MakeSharedControlBlock<T, Alloc>>;
    MakeSharedBlockAlloc cb_alloc = alloc;
    auto cb = MakeSharedBlockAllocTraits::allocate(cb_alloc, 1);
    MakeSharedBlockAllocTraits::construct(cb_alloc, cb, 1, 0, cb_alloc,
                                          std::forward<Args>(args)...);
    return SharedPtr<T>(cb);
}

template <typename T>
class WeakPtr {
  private:
    void swap(WeakPtr& other);

    void clear();

    T* object = nullptr;
    BaseControlBlock* cb = nullptr;

    template <typename U>
    friend class SharedPtr;

    template <typename U>
    friend class WeakPtr;

  public:
    WeakPtr() {}

    WeakPtr(const SharedPtr<T>& sp);

    WeakPtr(const WeakPtr& wptr);

    WeakPtr(WeakPtr&& wptr);

    WeakPtr& operator=(const WeakPtr& wptr);

    WeakPtr& operator=(WeakPtr&& wptr);

    bool expired() const noexcept;

    size_t use_count() const noexcept;

    SharedPtr<T> lock() const noexcept;

    template <typename U>
    WeakPtr(const SharedPtr<U>& sp) : object(sp.object), cb((sp.cb)) {
        if (cb != nullptr) {
            ++cb->cnt_weak;
        }
    }

    template <typename U>
    WeakPtr(const WeakPtr<U>& wptr) : object(wptr.object), cb((wptr.cb)) {
        if (cb != nullptr) {
            ++cb->cnt_weak;
        }
    }

    template <typename U>
    WeakPtr(WeakPtr<U>&& wptr) : object(wptr.object), cb(wptr.cb) {
        wptr.object = nullptr;
        wptr.cb = nullptr;
    }

    template <typename U>
    WeakPtr& operator=(const WeakPtr<U>& wptr) {
        WeakPtr copy(wptr);
        swap(copy);
        return *this;
    }

    template <typename U>
    WeakPtr& operator=(WeakPtr<U>&& wptr) {
        WeakPtr copy(std::move(wptr));
        swap(copy);
        return *this;
    }

    ~WeakPtr() {
        clear();
    }
};

template <typename T>
void WeakPtr<T>::swap(WeakPtr& other) {
    std::swap(cb, other.cb);
    std::swap(object, other.object);
}

template <typename T>
void WeakPtr<T>::clear() {
    if (cb != nullptr) {
        if (cb->cnt_shared == 0 && --cb->cnt_weak == 0) {
            cb->deallocate_cb();
        }
    }
}

template <typename T>
WeakPtr<T>::WeakPtr(const SharedPtr<T>& sp) : object(sp.object), cb(sp.cb) {
    if (cb != nullptr) {
        ++cb->cnt_weak;
    }
}

template <typename T>
WeakPtr<T>::WeakPtr(const WeakPtr& wptr) : object(wptr.object), cb(wptr.cb) {
    if (cb != nullptr) {
        ++cb->cnt_weak;
    }
}

template <typename T>
WeakPtr<T>::WeakPtr(WeakPtr&& wptr) : object(wptr.object), cb(wptr.cb) {
    wptr.object = nullptr;
    wptr.cb = nullptr;
}

template <typename T>
WeakPtr<T>& WeakPtr<T>::operator=(const WeakPtr& wptr) {
    if (this == &wptr) {
        return *this;
    }
    WeakPtr copy(wptr);
    swap(copy);
    return *this;
}

template <typename T>
WeakPtr<T>& WeakPtr<T>::operator=(WeakPtr&& wptr) {
    WeakPtr copy(std::move(wptr));
    swap(copy);
    return *this;
}

template <typename T>
SharedPtr<T> WeakPtr<T>::lock() const noexcept {
    if (!expired()) {
        return SharedPtr<T>(*this);
    }
    return SharedPtr<T>();
}

template <typename T>
size_t WeakPtr<T>::use_count() const noexcept {
    return (cb == nullptr ? 0 : cb->cnt_shared);
}

template <typename T>
bool WeakPtr<T>::expired() const noexcept {
    return use_count() == 0;
}