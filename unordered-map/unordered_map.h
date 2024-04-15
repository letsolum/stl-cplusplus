#pragma once

#include <cmath>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <vector>

template <class T, typename Allocator = std::allocator<T>>
class List {
  private:
    struct BaseNode {
        BaseNode* next = nullptr;
        BaseNode* prev = nullptr;

        BaseNode() = default;

        BaseNode(BaseNode* prv, BaseNode* nxt)
            : next(nxt), prev(prv) {}
    };

    struct ListNode : BaseNode {
        T val;

        ListNode() = default;

        template <typename... Args>
        ListNode(Args&&... args)
            : val(std::forward<Args>(args)...) {}
    };

    void swap(List& other) {
        std::swap(size_, other.size_);
        std::swap(base_, other.base_);
    }

  public:
    using AllocTraits = std::allocator_traits<Allocator>;
    using NodeTraits = typename AllocTraits::template rebind_traits<ListNode>;
    using NodeAlloc = typename AllocTraits::template rebind_alloc<ListNode>;
    using value_type = T;
    using BaseNodeTraits = typename AllocTraits::template rebind_traits<BaseNode>;
    using BaseNodeAlloc = typename AllocTraits::template rebind_alloc<BaseNode>;

    BaseNodeAlloc base_node_alloc_;
    NodeAlloc alloc_;
    BaseNode* base_;
    size_t size_;

    template <typename U>
    List(size_t n, U&& val)
        : List(n, std::forward<U>(val), Allocator()) {}

    template <typename U>
    List(size_t n, U&& val, Allocator alloc)
        : alloc_(alloc), size_(0) {
        try {
            size_t i = 0;
            base_ = BaseNodeTraits::allocate(base_node_alloc_, 1);
            base_->next = base_;
            base_->prev = base_;
            BaseNode* tail = base_;
            while (i < n) {
                auto new_node = NodeTraits::allocate(alloc_, 1);
                ++size_;
                NodeTraits::construct(alloc_, new_node, std::forward<U>(val));
                tail->next = new_node;
                ++i;
                new_node->prev = tail;
                base_->prev = new_node;
                new_node->next = base_;
                tail = new_node;
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    template <typename U>
    void push_front(U&& val) {
        insert(begin(), std::forward<U>(val));
    };

    template <bool is_const>
    class common_iterator {
      private:
        using base_node_pointer = std::conditional_t<is_const, const BaseNode*, BaseNode*>;
        using list_node_pointer = std::conditional_t<is_const, const ListNode*, ListNode*>;
        base_node_pointer node_;

        base_node_pointer get_node() const {
            return node_;
        }

        explicit common_iterator(base_node_pointer node)
            : node_(node) {}

        using value_type = T;
        using reference = std::conditional_t<is_const, T const&, T&>;
        friend List;
        using difference_type = ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

      public:
        common_iterator operator++(int) {
            auto copy = *this;
            operator++();
            return copy;
        }

        bool operator==(const common_iterator& other) const {
            return node_ == other.node_;
        }

        reference operator*() const {
            auto list_node = static_cast<list_node_pointer>(node_);
            return list_node->val;
        }

        common_iterator& operator--() {
            node_ = node_->prev;
            return *this;
        }

        common_iterator operator--(int) {
            auto copy = *this;
            operator--();
            return copy;
        }

        common_iterator& operator++() {
            node_ = node_->next;
            return *this;
        }

        bool operator!=(const common_iterator& other) const {
            return !operator==(other);
        }

        operator common_iterator<true>() const {
            return common_iterator<true>(node_);
        }
    };

    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    List()
        : List(Allocator()) {}

    List(Allocator alloc)
        : alloc_(alloc), size_(0) {
        base_ = BaseNodeTraits::allocate(base_node_alloc_, 1);
        base_->next = base_;
        base_->prev = base_;
    }

    List(size_t n)
        : List(n, Allocator()) {}

    List(size_t n, Allocator alloc)
        : alloc_(alloc), size_(0) {
        try {
            base_ = BaseNodeTraits::allocate(base_node_alloc_, 1);
            base_->next = base_;
            base_->prev = base_;
            BaseNode* tail = base_;
            for (size_t i = 0; i < n; ++i) {
                auto new_node = NodeTraits::allocate(alloc_, 1);
                NodeTraits::construct(alloc_, new_node);
                tail->next = new_node;
                new_node->prev = tail;
                base_->prev = new_node;
                new_node->next = base_;
                tail = new_node;
                ++size_;
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    List(const List& other)
        : alloc_(AllocTraits::select_on_container_copy_construction(other.get_allocator())),
          size_(0) {
        try {
            base_ = BaseNodeTraits::allocate(base_node_alloc_, 1);
            base_->next = base_;
            base_->prev = base_;
            BaseNode* tail = base_;
            auto iter = other.cbegin();
            for (size_t i = 0; i < other.size_; ++i) {
                auto new_node = NodeTraits::allocate(alloc_, 1);
                NodeTraits::construct(alloc_, new_node, *iter);
                tail->next = new_node;
                new_node->prev = tail;
                base_->prev = new_node;
                new_node->next = base_;
                tail = new_node;
                ++size_;
                ++iter;
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    List(List&& other) noexcept
        : alloc_(std::move(other.get_allocator())),
          size_(0) {
        base_ = BaseNodeTraits::allocate(base_node_alloc_, 1);
        base_->next = base_;
        base_->prev = base_;
        swap(other);
    }

    ~List() {
        clear();
        BaseNodeTraits::deallocate(base_node_alloc_, base_, 1);
    }

    void clear() {
        BaseNode* head = base_->next;
        for (size_t i = 0; i < size_; ++i) {
            BaseNode* next_head = head->next;
            NodeTraits::destroy(alloc_, static_cast<ListNode*>(head));
            NodeTraits::deallocate(alloc_, static_cast<ListNode*>(head), 1);
            head = next_head;
        }
        base_->next = base_;
        base_->prev = base_;
        size_ = 0;
    }

    List& operator=(const List& other) {
        if (AllocTraits::propagate_on_container_copy_assignment::value) {
            alloc_ = other.get_allocator();
        }
        List copy(alloc_);
        auto other_iter = other.cbegin();
        for (size_t i = 0; i < other.size_; ++i) {
            copy.push_back(*other_iter);
            ++other_iter;
        }
        swap(copy);
        return *this;
    }

    List& operator=(List&& other) {
        clear();
        if (AllocTraits::propagate_on_container_move_assignment::value) {
            alloc_ = other.get_allocator();
        }
        swap(other);
        return *this;
    }

    Allocator get_allocator() const {
        return Allocator(alloc_);
    }

    template <typename U>
    void push_back(U&& val) {
        insert(end(), std::forward<U>(val));
    }

    void pop_back() {
        erase(--end());
    };

    void pop_front() {
        erase(begin());
    };

    template <typename U>
    void insert(const_iterator iter, U&& val) {
        auto node = const_cast<BaseNode*>(iter.get_node());  // NOLINT
        auto prev_node = node->prev;
        auto new_node = NodeTraits::allocate(alloc_, 1);
        NodeTraits::construct(alloc_, new_node, std::forward<U>(val));
        prev_node->next = static_cast<BaseNode*>(new_node);
        new_node->prev = prev_node;
        node->prev = static_cast<BaseNode*>(new_node);
        new_node->next = node;
        ++size_;
    }

    void erase(const_iterator iter) {
        auto node = const_cast<BaseNode*>(iter.get_node());  // NOLINT
        auto prev_node = node->prev;
        auto next_node = node->next;
        prev_node->next = next_node;
        next_node->prev = prev_node;
        NodeTraits::destroy(alloc_, static_cast<ListNode*>(node));
        NodeTraits::deallocate(alloc_, static_cast<ListNode*>(node), 1);
        --size_;
    }

    iterator begin() {
        return iterator(base_->next);
    }
    const_iterator cbegin() const {
        return const_iterator(base_->next);
    }
    const_iterator begin() const {
        return cbegin();
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }
    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(cend());
    }
    const_reverse_iterator rbegin() const {
        return crbegin();
    }

    iterator end() {
        return iterator(base_);
    }
    const_iterator cend() const {
        return const_iterator(base_);
    };
    const_iterator end() const {
        return cend();
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    };
    const_reverse_iterator crend() const {
        return const_reverse_iterator(cbegin());
    };
    const_reverse_iterator rend() const {
        return crend();
    };

    size_t size() const {
        return size_;
    }
};

template <typename Key, typename Value,
          typename Hash = std::hash<Key>,
          typename Equal = std::equal_to<Key>,
          typename Allocator = std::allocator<std::pair<const Key, Value>>>
class UnorderedMap {
  public:
    using NodeType = std::pair<const Key, Value>;

  private:
    using AllocTraits = std::allocator_traits<Allocator>;
    using NodeTypeTraits = typename AllocTraits::template rebind_traits<NodeType>;
    using NodeTypeAlloc = typename AllocTraits::template rebind_alloc<NodeType>;

    NodeTypeAlloc alloc_;
    Hash hash_;
    Equal key_equal_;

    struct Node {
        NodeTypeAlloc alloc;
        Hash hash;
        NodeType* kv = nullptr;
        size_t hash_val;

        void set_alloc_and_hash(NodeTypeAlloc allocator, Hash hash_func) {
            alloc = allocator;
            hash = hash_func;
        }

        void construct_kv(const Node& other) {
            kv = other.kv;
            hash_val = other.hash_val;
        }

        void construct_kv(Node&& other) {
            kv = std::move(other.kv);
            hash_val = std::move(other.hash_val);
            other.kv = nullptr;
            other.hash_val = 0;
        }

        void construct_kv(const NodeType& pair) {
            kv = NodeTypeTraits::allocate(alloc, 1);
            NodeTypeTraits::construct(alloc, kv, pair);
            hash_val = hash(kv->first);
        }

        void construct_kv(NodeType&& pair) {
            kv = NodeTypeTraits::allocate(alloc, 1);
            NodeTypeTraits::construct(alloc, kv, std::move_if_noexcept(pair.first), std::move(pair.second));
            hash_val = hash(kv->first);
        }

        template <typename... Args>
        void construct_kv(Args&&... args) {
            kv = NodeTypeTraits::allocate(alloc, 1);
            NodeTypeTraits::construct(alloc, kv, std::forward<Args>(args)...);
            hash_val = hash(kv->first);
        }

        void destroy_kv() {
            if (kv != nullptr) {
                NodeTypeTraits::destroy(alloc, kv);
                NodeTypeTraits::deallocate(alloc, kv, 1);
                kv = nullptr;
            }
        }
    };

    List<Node, Allocator> bucket_lst_;
    std::vector<typename List<Node, Allocator>::iterator> arr_;

    size_t size_ = 0;
    size_t bucket_count_ = 1;
    float max_load_factor_ = 1.0;

  public:
    template <bool is_const>
    class common_iterator {
      public:
        using value_type = NodeType;
        using reference = std::conditional_t<is_const, NodeType const&, NodeType&>;
        using pointer = std::conditional_t<is_const, const NodeType*, NodeType*>;
        using difference_type = ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;
        using node_iterator_type = std::conditional_t<is_const,
                                                      typename List<Node, Allocator>::const_iterator,
                                                      typename List<Node, Allocator>::iterator>;

        reference operator*() const {
            return *(*iter_).kv;
        }

        pointer operator->() const {
            return (*iter_).kv;
        }

        common_iterator& operator++() {
            ++iter_;
            return *this;
        }

        common_iterator operator++(int) {
            auto copy = *this;
            operator++();
            return copy;
        }

        bool operator==(const common_iterator& other) const {
            return iter_ == other.iter_;
        }

        bool operator!=(const common_iterator& other) const {
            return !operator==(other);
        }

        operator common_iterator<true>() const {
            return common_iterator<true>(iter_);
        }

      private:
        node_iterator_type iter_;

        node_iterator_type get_node_iterator() {
            return iter_;
        }

        Node& get_node() {
            return *iter_;
        }

        explicit common_iterator(node_iterator_type iter)
            : iter_(iter) {}

        friend UnorderedMap;
    };

    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;

    UnorderedMap()
        : arr_(1, bucket_lst_.end()) {}

    UnorderedMap(const UnorderedMap& other)
        : alloc_(AllocTraits::select_on_container_copy_construction(other.get_allocator())),
          hash_(other.hash_),
          key_equal_(other.key_equal_),
          arr_(1, bucket_lst_.end()) {
        max_load_factor_ = other.max_load_factor_;
        for (auto it = other.cbegin(); it != other.cend(); ++it) {
            emplace(*it);
        }
    }

    UnorderedMap(UnorderedMap&& other) noexcept
        : alloc_(std::move(other.alloc_)),
          hash_(std::move(other.hash_)),
          key_equal_(std::move(other.key_equal_)) {
        std::swap(bucket_lst_, other.bucket_lst_);
        arr_ = std::move(other.arr_);
        size_ = std::move(other.size_);
        bucket_count_ = std::move(other.bucket_count_);
        max_load_factor_ = std::move(other.max_load_factor_);
    }

    ~UnorderedMap() {
        for (auto it = bucket_lst_.begin(); it != bucket_lst_.end(); ++it) {
            (*it).destroy_kv();
        }
    }

    UnorderedMap& operator=(const UnorderedMap& other) {
        if (AllocTraits::propagate_on_container_copy_assignment::value) {
            alloc_ = other.get_allocator();
        }
        hash_ = other.hash_;
        key_equal_ = other.key_equal_;
        max_load_factor_ = other.max_load_factor_;
        reserve(other.size_);
        for (auto it = other.cbegin(); it != other.cend(); ++it) {
            emplace(*it);
        }
        return *this;
    }

    UnorderedMap& operator=(UnorderedMap&& other) {
        for (auto it = bucket_lst_.begin(); it != bucket_lst_.end(); ++it) {
            (*it).destroy_kv();
        }

        if (AllocTraits::propagate_on_container_move_assignment::value) {
            alloc_ = other.get_allocator();
        }
        hash_ = std::move(other.hash_);
        key_equal_ = std::move(other.key_equal_);
        bucket_lst_ = std::move(other.bucket_lst_);
        arr_ = std::move(other.arr_);

        size_ = other.size_;
        bucket_count_ = other.bucket_count_;
        max_load_factor_ = other.max_load_factor_;

        other.size_ = 0;
        other.bucket_count_ = 0;
        other.max_load_factor_ = 0;
        return *this;
    }

    Allocator get_allocator() const {
        return Allocator(alloc_);
    }

    Value& operator[](const Key& key) {
        auto iter = find(key);
        if (iter != end()) {
            return iter->second;
        }
        std::tie(iter, std::ignore) = emplace(key, Value());
        return iter->second;
    }

    Value& operator[](Key&& key) {
        auto iter = find(key);
        if (iter != end()) {
            return iter->second;
        }
        std::tie(iter, std::ignore) = emplace(std::move(key), Value());
        return iter->second;
    }

    Value& at(const Key& key) {
        auto iter = find(key);
        if (iter == end()) {
            throw std::out_of_range("Key not found!");
        }
        return iter->second;
    }

    const Value& at(const Key& key) const {
        auto iter = find(key);
        if (iter == end()) {
            throw std::out_of_range("Key not found!");
        }
        return iter->second;
    }

    iterator find(const Key& key) {
        size_t hash_val = hash_(key);
        auto cur = arr_[hash_val % bucket_count_];
        ++cur;
        while (cur != bucket_lst_.end() && (*cur).hash_val == hash_val) {
            if (key_equal_((*cur).kv->first, key)) {
                return iterator(cur);
            }
            ++cur;
        }
        return end();
    }

    const_iterator find(const Key& key) const {
        size_t hash_val = hash_(key);
        auto cur = arr_[hash_val % bucket_count_];
        ++cur;
        while (cur != bucket_lst_.end() && (*cur).hash_val == hash_val) {
            if (key_equal_((*cur).kv->first, key)) {
                return const_iterator(cur);
            }
            ++cur;
        }
        return end();
    }

    std::pair<iterator, bool> insert(const NodeType& val) {
        return emplace(val);
    }

    std::pair<iterator, bool> insert(NodeType&& val) {
        return emplace(std::move(val));
    }

    template <typename T>
    std::pair<iterator, bool> insert(T&& val) {
        return emplace(std::forward<T>(val));
    }

    template <typename InputIt>
    void insert(InputIt begin, InputIt end) {
        for (auto it = begin; it != end; ++it) {
            emplace(*it);
        }
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        Node new_node;
        new_node.set_alloc_and_hash(alloc_, hash_);
        new_node.construct_kv(std::forward<Args>(args)...);
        bool inserted = true;
        auto found_iter = find(new_node.kv->first);
        if (found_iter != end()) {
            erase(found_iter);
            inserted = false;
        }

        auto node_iter = arr_[new_node.hash_val % bucket_count_];
        ++node_iter;
        bucket_lst_.insert(node_iter, new_node);
        if (node_iter != bucket_lst_.end() && (*node_iter).hash_val != new_node.hash_val) {
            arr_[(*node_iter).hash_val % bucket_count_] = bucket_lst_.begin();
        }
        ++size_;

        if (load_factor() >= max_load_factor_) {
            reserve(2 * size_);
        }

        auto ans_iter = arr_[new_node.hash_val % bucket_count_];
        ++ans_iter;
        return {iterator(ans_iter), inserted};
    }

    void erase(iterator iter) {
        auto node_iter = iter.get_node_iterator();
        size_t hash_val = (*node_iter).hash_val;
        auto next_iter = node_iter;
        ++next_iter;
        auto prev_iter = node_iter;
        --prev_iter;

        (*node_iter).destroy_kv();
        bucket_lst_.erase(node_iter);
        if (next_iter != bucket_lst_.end() && hash_val != (*next_iter).hash_val) {
            arr_[(*next_iter).hash_val % bucket_count_] = prev_iter;
        }
        if (arr_[hash_val % bucket_count_] == prev_iter) {
            arr_[hash_val % bucket_count_] = bucket_lst_.end();
        }
        --size_;
    }

    void erase(iterator begin, iterator end) {
        auto cur_it = begin;
        while (cur_it != end) {
            auto next_iter = cur_it;
            ++next_iter;
            erase(cur_it);
            cur_it = next_iter;
        }
    }

    iterator begin() {
        return iterator(bucket_lst_.begin());
    }

    const_iterator begin() const {
        return cbegin();
    }

    const_iterator cbegin() const {
        return const_iterator(bucket_lst_.cbegin());
    };

    iterator end() {
        return iterator(bucket_lst_.end());
    }

    const_iterator end() const {
        return cend();
    }

    const_iterator cend() const {
        return const_iterator(bucket_lst_.cend());
    };

    void rehash(size_t count) {
        auto bucket_lst_copy(std::move(bucket_lst_));
        bucket_count_ = count;

        arr_.resize(count, bucket_lst_.end());
        std::fill(arr_.begin(), arr_.end(), bucket_lst_.end());

        size_ = 0;
        for (auto& node : bucket_lst_copy) {
            emplace(std::move(node));
        }
    }

    void reserve(size_t count) {
        rehash(std::ceil(static_cast<float>(count) / max_load_factor_));
    }

    float load_factor() const {
        return static_cast<float>(size_) / static_cast<float>(bucket_count_);
    }

    float max_load_factor() const {
        return max_load_factor_;
    }

    void max_load_factor(float ml) {
        max_load_factor_ = ml;
    }

    size_t size() const {
        return size_;
    }

    size_t max_size() const {
        return std::numeric_limits<ptrdiff_t>::max();
    };
};
