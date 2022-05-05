#ifndef PIET_ROLLABLE_STACK_RBTREE_HPP
#define PIET_ROLLABLE_STACK_RBTREE_HPP
/**
 * @brief Red-Black-Tree(赤黒木)
 * @docs https://github.com/ei1333/library/blob/master/docs/red-black-tree.md
 */
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

template <typename Monoid>
struct RedBlackTree {
 public:
  enum COLOR { BLACK, RED };

  struct Node {
    Monoid key;
    std::unique_ptr<Node> l, r;
    COLOR color;
    int level, cnt;

    Node() {}

    Node(const Monoid &k)
        : key(k), l(nullptr), r(nullptr), color(BLACK), level(0), cnt(1) {}

    Node(std::unique_ptr<Node> l, std::unique_ptr<Node> r, const Monoid &k)
        : key(k), l(std::move(l)), r(std::move(r)), color(RED) {}

    bool is_leaf() const { return l == nullptr; }
  };

 private:
  std::unique_ptr<Node> alloc(std::unique_ptr<Node> l,
                              std::unique_ptr<Node> r) {
    auto t = std::make_unique<Node>(std::move(l), std::move(r), M1);
    update(t);
    return t;
  }

  std::unique_ptr<Node> rotate(std::unique_ptr<Node> t, bool b) {
    std::unique_ptr<Node> s;
    if (b) {
      s = std::move(t->l);
      t->l = std::move(s->r);
      s->r = std::move(t);
      update(s->r);
    } else {
      s = std::move(t->r);
      t->r = std::move(s->l);
      s->l = std::move(t);
      update(s->l);
    }
    update(s);
    return s;
  }

  std::unique_ptr<Node> submerge(std::unique_ptr<Node> l,
                                 std::unique_ptr<Node> r) {
    if (l->level < r->level) {
      std::unique_ptr<Node> &c =
          (r->l = submerge(std::move(l), std::move(r->l)));
      if (r->color == BLACK && c->color == RED && c->l && c->l->color == RED) {
        r->color = RED;
        c->color = BLACK;
        if (r->r->color == BLACK) return rotate(std::move(r), true);
        r->r->color = BLACK;
      }
      update(r);
      return r;
    }
    if (l->level > r->level) {
      std::unique_ptr<Node> &c =
          (l->r = submerge(std::move(l->r), std::move(r)));
      if (l->color == BLACK && c->color == RED && c->r && c->r->color == RED) {
        l->color = RED;
        c->color = BLACK;
        if (l->l->color == BLACK) return rotate(std::move(l), false);
        l->l->color = BLACK;
      }
      update(l);
      return l;
    }
    return alloc(std::move(l), std::move(r));
  }

  std::unique_ptr<Node> build(int l, int r, const std::vector<Monoid> &v) {
    if (l + 1 >= r) return alloc(v[l]);
    return merge(build(l, (l + r) >> 1, v), build((l + r) >> 1, r, v));
  }

  void update(std::unique_ptr<Node> &t) {
    t->cnt = count(t->l) + count(t->r) + (!t->l || !t->r);
    t->level = t->l ? t->l->level + (t->l->color == BLACK) : 0;
  }

  void dump(const std::unique_ptr<Node> &r,
            typename std::vector<Monoid>::iterator &it) const {
    if (r->is_leaf()) {
      *it++ = r->key;
      return;
    }
    dump(r->l, it);
    dump(r->r, it);
  }

  std::unique_ptr<Node> merge(std::unique_ptr<Node> l) { return l; }

 public:
  const Monoid M1;

  RedBlackTree(const Monoid &M1) : M1(M1) {}

  std::unique_ptr<Node> alloc(const Monoid &key) {
    return std::make_unique<Node>(key);
  }

  int count(const std::unique_ptr<Node> &t) const { return t ? t->cnt : 0; }

  std::pair<std::unique_ptr<Node>, std::unique_ptr<Node> > split(
      std::unique_ptr<Node> t, int k) {
    if (!t) return {nullptr, nullptr};
    if (k == 0) return {nullptr, std::move(t)};
    if (k >= count(t)) return {std::move(t), nullptr};
    std::unique_ptr<Node> &l = t->l;
    std::unique_ptr<Node> &r = t->r;
    if (k < count(l)) {
      auto pp = split(std::move(l), k);
      return {std::move(pp.first), merge(std::move(pp.second), std::move(r))};
    }
    if (k > count(l)) {
      auto pp = split(std::move(r), k - count(l));
      return {merge(std::move(l), std::move(pp.first)), std::move(pp.second)};
    }
    return {std::move(l), std::move(r)};
  }

  std::tuple<std::unique_ptr<Node>, std::unique_ptr<Node>,
             std::unique_ptr<Node> >
  split3(std::unique_ptr<Node> t, int a, int b) {
    auto x = split(std::move(t), a);
    auto y = split(std::move(x.second), b - a);
    return {std::move(x.first), std::move(y.first), std::move(y.second)};
  }

  template <typename... Args>
  std::unique_ptr<Node> merge(std::unique_ptr<Node> l, Args... rest) {
    auto r = merge(std::forward<Args>(rest)...);
    if (!l || !r) return std::move(l ? l : r);
    auto c = submerge(std::move(l), std::move(r));
    c->color = BLACK;
    return c;
  }

  std::unique_ptr<Node> build(const std::vector<Monoid> &v) {
    return build(0, (int)v.size(), v);
  }

  std::vector<Monoid> dump(const std::unique_ptr<Node> &r) const {
    std::vector<Monoid> v((size_t)count(r));
    auto it = begin(v);
    if (r != nullptr) dump(r, it);
    return v;
  }

  std::string to_string(const std::unique_ptr<Node> &r) const {
    const auto s = dump(r);
    std::string ret;
    for (auto &&elem : s) {
      ret += std::to_string(elem);
      ret += ", ";
    }
    return ret;
  }

  void insert(std::unique_ptr<Node> &t, int k, const Monoid &v) {
    auto x = split(t, k);
    t = merge(merge(x.first, alloc(v)), x.second);
  }

  Monoid erase(std::unique_ptr<Node> &t, int k) {
    auto x = split(t, k);
    auto y = split(x.second, 1);
    auto v = y.first->key;
    t = merge(x.first, y.second);
    return v;
  }

  Monoid query(std::unique_ptr<Node> t, int a, int b) {
    return query(t, a, b, 0, count(t));
  }

  void set_element(std::unique_ptr<Node> &t, int k, const Monoid &x) {
    if (t->is_leaf()) {
      t->key = x;
      return;
    }
    if (k < count(t->l))
      set_element(t->l, k, x);
    else
      set_element(t->r, k - count(t->l), x);
    t = update(t);
  }

  void push_front(std::unique_ptr<Node> &t, const Monoid &v) {
    t = merge(std::make_unique<Node>(v), std::move(t));
  }

  void push_back(std::unique_ptr<Node> &t, const Monoid &v) {
    t = merge(std::move(t), std::make_unique<Node>(v));
  }

  Monoid pop_front(std::unique_ptr<Node> &t) {
    auto ret = split(t, 1);
    t = std::move(ret.second);
    return ret.first->key;
  }

  Monoid pop_back(std::unique_ptr<Node> &t) {
    const auto pos = count(t) - 1;
    auto ret = split(std::move(t), pos);
    t = std::move(ret.first);
    return ret.second->key;
  }
};

#endif  // PIET_ROLLABLE_STACK_RBTREE_HPP
