#pragma once
#include <iostream>
#include <vector>
#include <functional>

template<typename T, typename Alloc = std::allocator<T>>
class List {
 private:
  struct Node;
  using AllocNode = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
  using AllocTraits = typename std::allocator_traits<AllocNode>;
 public:

  List() {
    head_->prev = head_;
    head_->next = head_;
  }

  List(int size) {
    head_->prev = head_;
    head_->next = head_;
    create_list(size);
  }

  List(int size, const Alloc &alloc) : alloc_(alloc) {
    head_->prev = head_;
    head_->next = head_;
    create_list(size);
  }

  List(int size, const T &value) {
    head_->prev = head_;
    head_->next = head_;
    create_list(size, value);
  }

  List(const Alloc &alloc) : alloc_(alloc), head_() {
    head_->prev = head_;
    head_->next = head_;
  }

  List(int size, const T &value, const Alloc &alloc) : alloc_(alloc) {
    head_->prev = head_;
    head_->next = head_;
    create_list(size, value);
  }

  List(const List &other) : alloc_(AllocTraits::select_on_container_copy_construction(other.alloc_)) {
    head_->prev = head_;
    head_->next = head_;
    for (const auto &it:other) {
      push_back(it);
    }
  }

  List(const List &other, const Alloc &alloc) : alloc_(alloc) {
    head_->prev = head_;
    head_->next = head_;
    for (auto it:other) {
      push_back(it);
    }
  }

  List(List &&other) : alloc_(other.alloc_),
                       size_(other.size_),
                       head_(other.head_) {
    other.head_ = AllocTraits::allocate(other.alloc_, 1);
    other.size_ = 0;
    other.head_->prev = other.head_;
    other.head_->next = other.head_;
  }

  List<T, Alloc> &operator=(const List<T, Alloc> &other) {
    List copy = other;
    if (AllocTraits::propagate_on_container_move_assignment::value) {
      alloc_ = other.alloc_;
    }
    swap(copy);
    return *this;
  }
  List<T, Alloc> &operator=(List &&other) {
    List copy = std::move(other);
    alloc_ = other.alloc_;
    swap(copy);
    return *this;
  }

  ~List() {
    erase_list();
  }

  template<bool IsConst>
  struct common_iterator {
   private:
    using T_c = std::conditional_t<IsConst, const T, T>;
   public:
    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    friend class List;

    common_iterator() : node_(nullptr) {}

    common_iterator<IsConst>(const common_iterator &other) : node_(other.node_) {}

    common_iterator<IsConst>(Node *node)
        : node_(node) {}

    common_iterator<IsConst> &operator=(const common_iterator<IsConst> &other) {
      node_ = other.node_;
      return *this;
    }
    T_c &operator*() const {
      return (node_->value);
    }
    T_c *operator->() const {
      return &(node_->value);
    }
    common_iterator<IsConst> &operator++() {
      node_ = node_->next;
      return *this;
    }
    common_iterator<IsConst> &operator--() {
      node_ = node_->prev;
      return *this;
    }
    common_iterator<IsConst> operator++(int) {
      common_iterator<IsConst> copy = *this;
      node_ = node_->next;
      return copy;
    }
    common_iterator<IsConst> operator--(int) {
      common_iterator<IsConst> copy = *this;
      node_ = node_->prev;
      return copy;
    }
    bool operator==(const const_iterator &other) const {
      return node_ == other.node_;
    }
    bool operator!=(const const_iterator &other) const {
      return node_ != other.node_;
    }
    operator common_iterator<true>() const {
      return common_iterator<true>(node_);
    }
    bool is_null() const {
      if (node_ == nullptr) {
        return true;
      }
      return false;
    }
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = T_c;
    using difference_type = std::ptrdiff_t;
    using pointer = T_c *;
    using reference = T_c &;

   private:
    friend common_iterator<false>;
    Node *node_;

  };
  using iterator = common_iterator<false>;
  using const_iterator = common_iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin() {
    return iterator(head_->next);
  }
  const_iterator begin() const {
    return const_iterator(head_->next);
  }
  iterator end() {
    return iterator(head_);
  }
  const_iterator end() const {
    return const_iterator(head_);
  }
  const_iterator cbegin() const {
    return const_iterator(head_->next);
  }
  const_iterator cend() const {
    return const_iterator(head_);
  }
  reverse_iterator rbegin() {
    return reverse_iterator(end());
  }
  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }
  reverse_iterator rend() {
    return reverse_iterator(begin());
  }
  const_reverse_iterator rend() const {
    return const_reverse_iterator(begin());
  }
  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(cend());
  }
  const_reverse_iterator crend() const {
    return const_reverse_iterator(cbegin());
  }

  int size() const {
    return size_;
  }

  AllocNode get_allocator() const {
    return alloc_;
  }

  void insert(const const_iterator &other, const T &value) {
    Node *new_node = AllocTraits::allocate(alloc_, 1);
    AllocTraits::construct(alloc_, new_node, value);
    other.node_->prev->next = new_node;
    new_node->prev = other.node_->prev;
    other.node_->prev = new_node;
    new_node->next = other.node_;
    ++size_;
  }

  void insert(const const_iterator &other) {
    Node *new_node = AllocTraits::allocate(alloc_, 1);
    AllocTraits::construct(alloc_, new_node);
    other.node_->prev->next = new_node;
    new_node->prev = other.node_->prev;
    other.node_->prev = new_node;
    new_node->next = other.node_;
    ++size_;
  }

  void erase(const const_iterator &other) {
    other.node_->prev->next = other.node_->next;
    other.node_->next->prev = other.node_->prev;
    AllocTraits::destroy(alloc_, other.node_);
    AllocTraits::deallocate(alloc_, other.node_, 1);
    --size_;
  }

  template<typename ...Args>
  void emplace(const const_iterator &other, Args &&...args) {
    Node *new_node = AllocTraits::allocate(alloc_, 1);
    AllocTraits::construct(alloc_, new_node, std::forward<Args>(args)...);
    other.node_->prev->next = new_node;
    new_node->prev = other.node_->prev;
    other.node_->prev = new_node;
    new_node->next = other.node_;
    ++size_;
  }

  void push_back(const T &value) {
    insert(end(), value);
  }
  void push_back(T &&value) {
    insert(end(), std::move(value));
  }
  template<typename ... Args>
  void emplace_back(Args &&... args) {
    emplace(end(), std::forward<Args>(args)...);
  }
  void pop_back() {
    erase(--end());
  }

  void push_front(const T &value) {
    insert(begin(), value);
  }

  void pop_front() {
    erase(begin());
  }

  void clear() {
    erase_list();
  }

 private:
  void swap(List &L) {
    std::swap(size_, L.size_);
    std::swap(head_, L.head_);
  }

  void create_list(int size, const T &value) {
    for (int i = 0; i < size; ++i) {
      push_back(value);
    }
  }

  void create_list(int size) {
    for (int i = 0; i < size; ++i) {
      insert(end());
    }
  }

  void erase_list() {
    while (size_) {
      pop_back();
    }
  }

  struct Node {
    template<typename ...Args>
    Node(Args &&...args) : value(std::forward<Args>(args)...) {}
    Node(const Node &other) : value(other.value), prev(other.prev), next(other.next) {}
    Node() : value() {}
    T value;
    Node *prev = nullptr;
    Node *next = nullptr;
  };

 private:
  AllocNode alloc_;
  int size_ = 0;
  Node *head_ = AllocTraits::allocate(alloc_, 1);
};

template<typename T, typename Alloc = std::allocator<T>>
void swap_lists(List<T, Alloc> &l1, List<T, Alloc> &l2) {
  List<T, Alloc> copy = std::move(l1);
  l1 = std::move(l2);
  l2 = std::move(copy);
}

template<typename Key, typename Value, typename Hash = std::hash<Key>, typename Equal = std::equal_to<Key>, typename Alloc  = std::allocator<
    std::pair<const Key, Value>>>
class UnorderedMap {

 private:
  struct Node;
  using AllocNode = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
  template<typename T>
  using MainAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
  using AllocTraits = typename std::allocator_traits<AllocNode>;
  using AllocTraitsPair = typename std::allocator_traits<Alloc>;
  using Iterator = typename List<Node *, MainAlloc<Node *>>::iterator;
 public:
  using NodeType = std::pair<const Key, Value>;

  UnorderedMap() = default;

  UnorderedMap(const UnorderedMap &other)
      : alloc_(AllocTraits::select_on_container_copy_construction(other.alloc_)), alloc_pair_(alloc_) {
    for (auto it:other) {
      insert(it);
    }
  }

  UnorderedMap(const UnorderedMap &other, const Alloc &alloc)
      : alloc_(alloc), alloc_pair_(alloc_), table_(other.table_), lst_(other.lst_, alloc_) {}

  UnorderedMap(UnorderedMap &&other)
      : alloc_(AllocTraits::select_on_container_copy_construction(other.alloc_)), alloc_pair_(alloc_),
        table_(other.table_),
        lst_(other.lst_, alloc_) { other.erase_map(); }

  UnorderedMap(UnorderedMap &&other, const Alloc &alloc) : alloc_(alloc), alloc_pair_(alloc_), table_(other.table_),
                                                           lst_(other.lst_) { other.erase_map(); }

  ~UnorderedMap() {
    erase_map();
  }

  UnorderedMap &operator=(const UnorderedMap &other) {
    if (AllocTraits::propagate_on_container_copy_assignment::value) {
      alloc_ = other.alloc_;
      alloc_pair_ = alloc_;
    }
    for (auto it:other) {
      insert(*it);
    }
    return *this;

  }
  UnorderedMap &operator=(UnorderedMap &&other) {
    table_ = std::move(other.table_);
    other.table_.resize(16, nullptr);
    lst_ = std::move(other.lst_);
    alloc_ = other.alloc_;
    alloc_pair_ = alloc_pair_;
    return *this;
  }

  template<bool IsConst>
  struct common_iterator {
   private:
    using T_c = std::conditional_t<IsConst, const NodeType, NodeType>;
   public:
    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;
    friend class UnorderedMap;

    common_iterator() : iter_() {}

    common_iterator<IsConst>(const common_iterator &other) : iter_(other.iter_) {}

    common_iterator<IsConst>(std::conditional_t<IsConst,
                                                typename List<Node *,
                                                              MainAlloc<Node *>>::const_iterator,
                                                typename List<Node *, MainAlloc<Node *>>::iterator> iter)
        : iter_(iter) {}

    common_iterator<IsConst> &operator=(const common_iterator<IsConst> &other) {
      iter_ = other.iter_;
      return *this;
    }

    T_c &operator*() {
      return (*iter_)->data;
    }

    T_c *operator->() const {
      return &((*iter_)->data);
    }

    common_iterator<IsConst> &operator++() {
      ++iter_;
      return *this;
    }

    common_iterator<IsConst> operator++(int) const {
      common_iterator<IsConst> copy = *this;
      return ++copy;
    }

    bool operator==(const const_iterator &other) const {
      return iter_ == other.iter_;
    }

    bool operator!=(const const_iterator &other) const {
      return iter_ != other.iter_;
    }

    operator common_iterator<true>() const {
      return common_iterator<true>(iter_);
    }

    using iterator_category = std::forward_iterator_tag;
    using value_type = T_c;
    using difference_type = std::ptrdiff_t;
    using pointer = T_c *;
    using reference = T_c &;

   private:
   public:
    size_t hash() const {
      return (*iter_)->cached;
    }

    friend common_iterator<false>;
    std::conditional_t<IsConst,
                       typename List<Node *, MainAlloc<Node *>>::const_iterator,
                       typename List<Node *, MainAlloc<Node *>>::iterator> iter_;
  };
  using iterator = common_iterator<false>;
  using const_iterator = common_iterator<true>;

  iterator begin() {
    return iterator(lst_.begin());
  }
  const_iterator begin() const {
    return const_iterator(lst_.begin());
  }
  iterator end() {
    return iterator(lst_.end());
  }
  const_iterator end() const {
    return const_iterator(lst_.end());
  }
  const_iterator cbegin() const {
    return const_iterator(lst_.cbegin());
  }
  const_iterator cend() const {
    return const_iterator(lst_.cend());
  }

  template<typename ...Args>
  std::pair<iterator, bool> emplace(Args &&...args) {
    rehash();
    Node *node = AllocTraits::allocate(alloc_, 1);
    AllocTraitsPair::construct(alloc_pair_, &node->data, std::forward<Args>(args)...);
    node->cached = Hash()(node->data.first);
    size_t hash = node->cached;
    auto bucket_end = iterator(find(node->data.first, hash));
    if (bucket_end != end()) {
      AllocTraitsPair::destroy(alloc_pair_, &node->data);
      AllocTraits::deallocate(alloc_, node, 1);
      return {bucket_end, false};
    }
    auto bucket_begin = table_[hash % table_.size()];
    if (bucket_begin.is_null()) {
      bucket_begin = lst_.end();
    }
    lst_.insert(bucket_begin, node);
    table_[hash % table_.size()] = --bucket_begin;
    return {iterator(bucket_begin), true};
  }

  using node_key = std::pair<Key, Value>;
  std::pair<iterator, bool> insert(node_key &&node) {
    rehash();
    size_t hash = Hash()(node.first);
    auto bucket_end = iterator(find(node.first, hash));

    if (bucket_end != end()) {
      return {bucket_end, false};
    }
    auto bucket_begin = table_[hash % table_.size()];
    if (table_[hash % table_.size()].is_null()) {
      bucket_begin = lst_.end();
    }
    Node *new_node = AllocTraits::allocate(alloc_, 1);
    AllocTraitsPair::construct(alloc_pair_, &new_node->data, std::move(node));
    new_node->cached = hash;
    lst_.insert(bucket_begin, new_node);
    table_[hash % table_.size()] = --bucket_begin;
    return {iterator(bucket_begin), true};
  }

  std::pair<iterator, bool> insert(const node_key &node) {
    rehash();
    size_t hash = Hash()(node.first);
    auto bucket_end = iterator(find(node.first, hash));
    if (bucket_end != end()) {
      return {bucket_end, false};
    }
    auto bucket_begin = table_[hash % table_.size()];
    if (table_[hash % table_.size()].is_null()) {
      bucket_begin = lst_.end();
    }
    Node *new_node = AllocTraits::allocate(alloc_, 1);
    AllocTraitsPair::construct(alloc_pair_, &new_node->data, node);
    new_node->cached = hash;
    lst_.insert(bucket_begin, new_node);
    table_[hash % table_.size()] = --bucket_begin;
    return {iterator(bucket_begin), true};
  }
  template<typename InputIterator>
  void insert(const InputIterator begin, const InputIterator end) {
    for (auto it = begin; it != end; ++it) {
      insert(*it);
    }
  }

  iterator find(const Key &key) {
    return find(key, Hash()(key));
  }

  auto &operator[](const Key &key) {
    auto bucket = insert({key, Value()}).first;
    return bucket->second;
  }

  Value &at(const Key &key) {
    auto bucket = find(key);
    if (bucket == end()) {
      throw std::out_of_range("...");
    }
    return (bucket->second);
  }

  const Value &at(const Key &key) const {
    auto bucket = find(key);
    if (bucket == end()) {
      throw std::out_of_range("...");
    }
    return (bucket->second);
  }

  void reserve(size_t n) {
    rehash(n);
  }

  size_t size() const {
    return lst_.size();
  }

  size_t max_size() {
    return table_.size();
  }

  double load_factor() {
    return double(lst_.size() / table_.size());
  }

  void erase(const iterator &iterate) {
    iterator it = iterate;
    auto iter = it.iter_;
    size_t hash = (*iter)->cached;
    if (iter != lst_.begin()) {
      auto it_l = iter;
      if (it_l == lst_.begin() || hash != iterator(--it_l).hash()) {
        auto copy_it = iter;
        if (++copy_it != lst_.end() && (hash == iterator(copy_it).hash())) {
          table_[hash % table_.size()] = ++iter;
          --iter;
        } else {
          table_[hash % table_.size()] = nullptr;
        }
      }

    } else {
      auto copy_it = iter;
      if (++copy_it != lst_.end() && hash == iterator(copy_it).hash()) {
        table_[hash % table_.size()] = ++iter;
        --iter;
      } else {
        table_[hash % table_.size()] = nullptr;
      }
    }
    lst_.erase(iter);
  }

  void erase(const iterator &begin, const iterator &end) {
    for (auto it = begin; it != end;) {
      auto copy = it;
      ++it;
      erase(copy);
    }
  }

 private:
  iterator find(const Key &key, size_t hash) {
    if (table_[hash % table_.size()].is_null()) {
      return iterator(lst_.end());
    }
    auto iter = iterator(table_[hash % table_.size()]);
    while (iter != end() && iter.hash() == hash) {
      if (Equal()(iter->first, key)) {
        return iter;
      }
      ++iter;
    }
    return iterator(lst_.end());
  }

  void rehash(int sz = 0) {
    if (double(lst_.size() / table_.size()) > 0.9 || sz != 0) {
      if (sz == 0) {
        sz = 2 * table_.size();
      }
      table_.clear();
      table_.resize(sz, nullptr);
      List<Node *, MainAlloc<Node *>> new_lst;
      swap_lists(lst_, new_lst);
      for (auto &it:new_lst) {
        insertptrnode(it);
      }
    }
  }

  void insertptrnode(Node *node) {
    size_t hash = node->cached;
    auto bucket_begin = table_[hash % table_.size()];
    if (table_[hash % table_.size()].is_null()) {
      bucket_begin = lst_.end();
    }
    lst_.insert(bucket_begin, node);
    table_[hash % table_.size()] = --bucket_begin;
  }

  void erase_map() {
    lst_.clear();
    table_.clear();
    table_.shrink_to_fit();
  }

  void swap(UnorderedMap &other) {
    std::swap(table_, other.table_);
    std::swap(lst_, other.lst_);

  }
 private:
  struct Node {
    template<typename ...Args>
    Node(size_t hash, Args &&...args) : cached(hash), data(std::forward<Args>(args)...) {}
    template<typename ...Args>
    Node(Args &&... args) :data(std::forward<Args>(args)...) { cached = Hash()(data.first); }
    size_t cached;
    NodeType data;
  };

  AllocNode alloc_;
  Alloc alloc_pair_;
  std::vector<Iterator, MainAlloc<Iterator>> table_
      = std::vector<Iterator, MainAlloc<Iterator>>(16, nullptr);
  List<Node *, MainAlloc<Node *>> lst_;
};
