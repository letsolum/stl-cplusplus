#include <cstddef>
#include <iostream>
#include <type_traits>

template <typename T, typename Allocator = std::allocator<T>>
class List {
  private:
    struct BaseNode {
        BaseNode* prev;
        BaseNode* next;

        BaseNode()
            : prev(this), next(this) {}
    };

    BaseNode root;

    struct Node : BaseNode {
        T value;
        Node() = default;

        Node(const T& value)
            : value(value) {}
    };
    using value_type = T;
    using AllocatorTraits = std::allocator_traits<Allocator>;
    using NodeAllocator = typename AllocatorTraits::template rebind_alloc<Node>;
    using NodeAllocatorTraits =
        typename AllocatorTraits::template rebind_traits<Node>;

    Allocator allocator_;
    NodeAllocator node_alloc_ = allocator_;

    size_t size_;

    template <bool is_const>
    class CommonIterator;

    void add_elements(const List& /*arg_list*/);
    void swap(List<T, Allocator>& /*arg_list*/) noexcept;

  public:
    List() noexcept;
    List(size_t /*size*/);
    List(size_t /*size*/, const T& /*value*/);
    List(const Allocator& /*allocator*/) noexcept;
    List(size_t /*size*/, const Allocator& /*allocator*/);
    List(size_t /*size*/, const T& /*value*/, const Allocator& /*allocator*/);

    List(const List& /*other_list*/);
    ~List() noexcept;
    List& operator=(const List& /*arg_list*/);

    const Allocator& get_allocator() const {
        return allocator_;
    };

    size_t size() const noexcept;

    using iterator = CommonIterator<false>;
    using const_iterator = CommonIterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    template <typename... Args>
    iterator emplace(const_iterator /*iter*/, const Args&... /*args*/);

    template <typename... Args>
    iterator insert(const_iterator /*iter*/, const Args&... /*args*/);
    iterator erase(const_iterator /*iter*/) noexcept;

    void push_back(const T& /*value*/);
    void push_back();
    void push_front(const T& /*value*/);
    void pop_back() noexcept;
    void pop_front() noexcept;

    iterator begin() noexcept;
    const_iterator begin() const noexcept;
    const_iterator cbegin() const noexcept;
    iterator end() noexcept;
    const_iterator end() const noexcept;
    const_iterator cend() const noexcept;

    reverse_iterator rbegin() noexcept;
    const_reverse_iterator rbegin() const noexcept;
    const_reverse_iterator crbegin() const noexcept;
    reverse_iterator rend() noexcept;
    const_reverse_iterator rend() const noexcept;
    const_reverse_iterator crend() const noexcept;

  private:
    iterator begin_, end_;
};

template <typename T, typename Allocator>
List<T, Allocator>::List() noexcept
    : size_(0), begin_(&root), end_(&root) {}

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t size)
    : List<T, Allocator>() {
    try {
        while (size-- > 0) {
            push_back();
        }
    } catch (...) {
        while (size_ > 0) {
            pop_back();
        }
        throw;
    }
}

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t size, const T& value)
    : List<T, Allocator>() {
    try {
        while (size-- > 0) {
            push_back(value);
        }
    } catch (...) {
        while (size_ > 0) {
            pop_back();
        }
        throw;
    }
}

template <typename T, typename Allocator>
List<T, Allocator>::List(const Allocator& allocator) noexcept
    : allocator_(allocator), size_(0), begin_(&root), end_(&root) {}

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t size, const Allocator& allocator)
    : List<T, Allocator>(allocator) {
    node_alloc_ = allocator_;
    try {
        while (size-- > 0) {
            push_back();
        }
    } catch (...) {
        while (size_ > 0) {
            pop_back();
        }
        throw;
    }
}

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t size, const T& value,
                         const Allocator& allocator)
    : List<T, Allocator>(size, allocator) {
    node_alloc_ = allocator_;
    for (auto it = begin_; it != end_; ++it) {
        try {
            *it = value;
        } catch (...) {
            while (size_ > 0) {
                pop_back();
            }
            throw;
        }
    }
}

template <typename T, typename Allocator>
void List<T, Allocator>::add_elements(const List<T, Allocator>& arg_list) {
    for (auto it = arg_list.begin(); it != arg_list.end(); ++it) {
        try {
            push_back(*it);
        } catch (...) {
            while (size_ > 0) {
                pop_back();
            }
            throw;
        }
    }
}

template <typename T, typename Allocator>
List<T, Allocator>::List(const List& other_list)
    : List(AllocatorTraits::select_on_container_copy_construction(
          other_list.allocator_)) {
    add_elements(other_list);
}

template <typename T, typename Allocator>
List<T, Allocator>::~List() noexcept {
    while (size_ > 0) {
        pop_back();
    }
}

template <typename T, typename Allocator>
void List<T, Allocator>::swap(List<T, Allocator>& arg_list) noexcept {
    std::swap(root, arg_list.root);
    std::swap(root.prev->next, arg_list.root.prev->next);
    std::swap(root.next->prev, arg_list.root.next->prev);

    std::swap(size_, arg_list.size_);
    std::swap(begin_, arg_list.begin_);
    std::swap(allocator_, arg_list.allocator_);
    std::swap(node_alloc_, arg_list.node_alloc_);
}

template <typename T, typename Allocator>
List<T, Allocator>& List<T, Allocator>::operator=(const List& arg_list) {
    List<T, Allocator> tmp_list(
        AllocatorTraits::propagate_on_container_copy_assignment::value
            ? arg_list.allocator_
            : allocator_);
    tmp_list.add_elements(arg_list);
    swap(tmp_list);
    return *this;
}

template <typename T, typename Allocator>
size_t List<T, Allocator>::size() const noexcept {
    return size_;
}

template <typename T, typename Allocator>
template <typename... Args>
typename List<T, Allocator>::iterator List<T, Allocator>::emplace(
    List::const_iterator iter, const Args&... args) {
    auto ptr =
        static_cast<BaseNode*>(NodeAllocatorTraits::allocate(node_alloc_, 1));
    try {
        NodeAllocatorTraits::construct(node_alloc_, static_cast<Node*>(ptr),
                                       args...);
    } catch (...) {
        NodeAllocatorTraits::deallocate(node_alloc_, static_cast<Node*>(ptr), 1);
        throw;
    }
    ++size_;
    auto cur = ptr;
    auto prev = (iter.get_node())->prev,
         next = static_cast<BaseNode*>(iter.get_node());
    cur->prev = prev;
    cur->next = next;
    prev->next = cur;
    next->prev = cur;
    if (iter == begin_) {
        begin_ = iterator(cur);
    }
    return iterator(cur);
}

template <typename T, typename Allocator>
void List<T, Allocator>::push_back(const T& value) {
    emplace(end_, value);
}

template <typename T, typename Allocator>
void List<T, Allocator>::push_back() {
    emplace(end_);
}

template <typename T, typename Allocator>
void List<T, Allocator>::push_front(const T& value) {
    emplace(begin_, value);
}

template <typename T, typename Allocator>
void List<T, Allocator>::pop_back() noexcept {
    erase(std::prev(end_));
}

template <typename T, typename Allocator>
void List<T, Allocator>::pop_front() noexcept {
    erase(begin_);
}

template <typename T, typename Allocator>
template <typename... Args>
typename List<T, Allocator>::iterator List<T, Allocator>::insert(
    List::const_iterator iter, const Args&... args) {
    return emplace(iter, args...);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::iterator List<T, Allocator>::erase(
    List::const_iterator iter) noexcept {
    BaseNode* node_to_delete = iter.get_node();
    BaseNode* prev = node_to_delete->prev;
    BaseNode* next = node_to_delete->next;
    prev->next = next;
    next->prev = prev;
    NodeAllocatorTraits::destroy(node_alloc_, static_cast<Node*>(node_to_delete));
    NodeAllocatorTraits::deallocate(node_alloc_,
                                    static_cast<Node*>(node_to_delete), 1);
    --size_;
    if (iter == begin_) {
        begin_ = iterator(next);
    }
    return iterator(next);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::iterator List<T, Allocator>::begin() noexcept {
    return begin_;
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::begin()
    const noexcept {
    return cbegin();
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::cbegin()
    const noexcept {
    return static_cast<const_iterator>(begin_);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::iterator List<T, Allocator>::end() noexcept {
    return end_;
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::end()
    const noexcept {
    return cend();
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::cend()
    const noexcept {
    return static_cast<const_iterator>(end_);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::reverse_iterator
List<T, Allocator>::rbegin() noexcept {
    return reverse_iterator(end_);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator List<T, Allocator>::rbegin()
    const noexcept {
    return crbegin();
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator
List<T, Allocator>::crbegin() const noexcept {
    return const_reverse_iterator(end_);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::reverse_iterator
List<T, Allocator>::rend() noexcept {
    return reverse_iterator(begin_);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator List<T, Allocator>::rend()
    const noexcept {
    return crend();
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator List<T, Allocator>::crend()
    const noexcept {
    return const_reverse_iterator(begin_);
}

template <typename T, typename Allocator>
template <bool is_const>
class List<T, Allocator>::CommonIterator {
  private:
    BaseNode* node_;

  public:
    CommonIterator() noexcept;
    CommonIterator(BaseNode* /*node*/) noexcept;

    using value_type = T;
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = ssize_t;
    using reference = typename std::conditional<is_const, const T&, T&>::type;
    using pointer = typename std::conditional<is_const, const T*, T*>::type;

    operator CommonIterator<true>() const noexcept;

    CommonIterator<is_const> operator--(int) noexcept;
    CommonIterator<is_const> operator++(int) noexcept;
    CommonIterator<is_const>& operator--() noexcept;
    CommonIterator<is_const>& operator++() noexcept;

    reference operator*() const noexcept;
    pointer operator->() const noexcept;

    bool operator==(
        const CommonIterator<is_const>& /*other_iter*/) const noexcept;
    bool operator!=(
        const CommonIterator<is_const>& /*other_iter*/) const noexcept;

    BaseNode* get_node() const noexcept;
};

template <typename T, typename Allocator>
template <bool is_const>
List<T, Allocator>::CommonIterator<is_const>::CommonIterator() noexcept =
    default;

template <typename T, typename Allocator>
template <bool is_const>
List<T, Allocator>::CommonIterator<is_const>::CommonIterator(
    BaseNode* node) noexcept
    : node_(node) {}

template <typename T, typename Allocator>
template <bool is_const>
List<T, Allocator>::CommonIterator<is_const>::operator CommonIterator<true>()
    const noexcept {
    return CommonIterator<true>(node_);
}

template <typename T, typename Allocator>
template <bool is_const>
typename List<T, Allocator>::template CommonIterator<is_const>&
List<T, Allocator>::CommonIterator<is_const>::operator--() noexcept {
    node_ = node_->prev;
    return *this;
}

template <typename T, typename Allocator>
template <bool is_const>
typename List<T, Allocator>::template CommonIterator<is_const>&
List<T, Allocator>::CommonIterator<is_const>::operator++() noexcept {
    node_ = node_->next;
    return *this;
}

template <typename T, typename Allocator>
template <bool is_const>
typename List<T, Allocator>::template CommonIterator<is_const>
List<T, Allocator>::CommonIterator<is_const>::operator--(int) noexcept {
    CommonIterator<is_const> tmp(*this);
    --(*this);
    return tmp;
}

template <typename T, typename Allocator>
template <bool is_const>
typename List<T, Allocator>::template CommonIterator<is_const>
List<T, Allocator>::CommonIterator<is_const>::operator++(int) noexcept {
    CommonIterator<is_const> tmp(*this);
    ++(*this);
    return tmp;
}

template <typename T, typename Allocator>
template <bool is_const>
typename List<T, Allocator>::template CommonIterator<is_const>::reference
List<T, Allocator>::CommonIterator<is_const>::operator*() const noexcept {
    return static_cast<Node*>(node_)->value;
}

template <typename T, typename Allocator>
template <bool is_const>
typename List<T, Allocator>::template CommonIterator<is_const>::pointer
List<T, Allocator>::CommonIterator<is_const>::operator->() const noexcept {
    return &(operator*());
};

template <typename T, typename Allocator>
template <bool is_const>
bool List<T, Allocator>::CommonIterator<is_const>::operator==(
    const List<T, Allocator>::CommonIterator<is_const>& other_iter)
    const noexcept {
    return (node_ == other_iter.node_);
}

template <typename T, typename Allocator>
template <bool is_const>
bool List<T, Allocator>::CommonIterator<is_const>::operator!=(
    const List<T, Allocator>::CommonIterator<is_const>& other_iter)
    const noexcept {
    return !(*this == other_iter);
}

template <typename T, typename Allocator>
template <bool is_const>
typename List<T, Allocator>::BaseNode*
List<T, Allocator>::CommonIterator<is_const>::get_node() const noexcept {
    return static_cast<BaseNode*>(node_);
}