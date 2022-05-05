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
    std::shared_ptr<Node> l, r;
    COLOR color;
    int level, cnt;

    Node() {}

    Node(const Monoid &k)
        : key(k), l(nullptr), r(nullptr), color(BLACK), level(0), cnt(1) {}

    Node(std::shared_ptr<Node> l, std::shared_ptr<Node> r, const Monoid &k)
        : key(k), l(l), r(r), color(RED) {}

    bool is_leaf() const { return l == nullptr; }
  };

 private:
  std::shared_ptr<Node> alloc(std::shared_ptr<Node> l,
                              std::shared_ptr<Node> r) {
    auto t = std::make_shared<Node>(l, r, M1);
    return update(t);
  }

  virtual std::shared_ptr<Node> clone(std::shared_ptr<Node> t) { return t; }

  std::shared_ptr<Node> rotate(std::shared_ptr<Node> t, bool b) {
    t = clone(t);
    std::shared_ptr<Node> s;
    if (b) {
      s = clone(t->l);
      t->l = s->r;
      s->r = t;
    } else {
      s = clone(t->r);
      t->r = s->l;
      s->l = t;
    }
    update(t);
    return update(s);
  }

  std::shared_ptr<Node> submerge(std::shared_ptr<Node> l,
                                 std::shared_ptr<Node> r) {
    if (l->level < r->level) {
      r = clone(r);
      std::shared_ptr<Node> c = (r->l = submerge(l, r->l));
      if (r->color == BLACK && c->color == RED && c->l && c->l->color == RED) {
        r->color = RED;
        c->color = BLACK;
        if (r->r->color == BLACK) return rotate(r, true);
        r->r->color = BLACK;
      }
      return update(r);
    }
    if (l->level > r->level) {
      l = clone(l);
      std::shared_ptr<Node> c = (l->r = submerge(l->r, r));
      if (l->color == BLACK && c->color == RED && c->r && c->r->color == RED) {
        l->color = RED;
        c->color = BLACK;
        if (l->l->color == BLACK) return rotate(l, false);
        l->l->color = BLACK;
      }
      return update(l);
    }
    return alloc(l, r);
  }

  std::shared_ptr<Node> build(int l, int r, const std::vector<Monoid> &v) {
    if (l + 1 >= r) return alloc(v[l]);
    return merge(build(l, (l + r) >> 1, v), build((l + r) >> 1, r, v));
  }

  std::shared_ptr<Node> update(std::shared_ptr<Node> t) {
    t->cnt = count(t->l) + count(t->r) + (!t->l || !t->r);
    t->level = t->l ? t->l->level + (t->l->color == BLACK) : 0;
    return t;
  }

  void dump(std::shared_ptr<Node> r,
            typename std::vector<Monoid>::iterator &it) {
    if (r->is_leaf()) {
      *it++ = r->key;
      return;
    }
    dump(r->l, it);
    dump(r->r, it);
  }

  std::shared_ptr<Node> merge(std::shared_ptr<Node> l) { return l; }

 public:
  const Monoid M1;

  RedBlackTree(const Monoid &M1) : M1(M1) {}

  std::shared_ptr<Node> alloc(const Monoid &key) {
    return std::make_shared<Node>(key);
  }

  int count(const std::shared_ptr<Node> t) const { return t ? t->cnt : 0; }

  std::pair<std::shared_ptr<Node>, std::shared_ptr<Node> > split(
      std::shared_ptr<Node> t, int k) {
    if (!t) return {nullptr, nullptr};
    if (k == 0) return {nullptr, t};
    if (k >= count(t)) return {t, nullptr};
    t = clone(t);
    std::shared_ptr<Node> l = t->l, r = t->r;
    t.reset();
    if (k < count(l)) {
      auto pp = split(l, k);
      return {pp.first, merge(pp.second, r)};
    }
    if (k > count(l)) {
      auto pp = split(r, k - count(l));
      return {merge(l, pp.first), pp.second};
    }
    return {l, r};
  }

  std::tuple<std::shared_ptr<Node>, std::shared_ptr<Node>,
             std::shared_ptr<Node> >
  split3(std::shared_ptr<Node> t, int a, int b) {
    auto x = split(t, a);
    auto y = split(x.second, b - a);
    return std::make_tuple(x.first, y.first, y.second);
  }

  template <typename... Args>
  std::shared_ptr<Node> merge(std::shared_ptr<Node> l, Args... rest) {
    auto r = merge(rest...);
    if (!l || !r) return l ? l : r;
    auto c = submerge(l, r);
    c->color = BLACK;
    return c;
  }

  std::shared_ptr<Node> build(const std::vector<Monoid> &v) {
    return build(0, (int)v.size(), v);
  }

  std::vector<Monoid> dump(std::shared_ptr<Node> r) {
    std::vector<Monoid> v((size_t)count(r));
    auto it = begin(v);
    if (r != nullptr) dump(r, it);
    return v;
  }

  std::string to_string(std::shared_ptr<Node> r) {
    const auto s = dump(r);
    std::string ret;
    for (auto &&elem : s) {
      ret += std::to_string(elem);
      ret += ", ";
    }
    return ret;
  }

  void insert(std::shared_ptr<Node> &t, int k, const Monoid &v) {
    auto x = split(t, k);
    t = merge(merge(x.first, alloc(v)), x.second);
  }

  Monoid erase(std::shared_ptr<Node> &t, int k) {
    auto x = split(t, k);
    auto y = split(x.second, 1);
    auto v = y.first->key;
    y.first.reset();
    t = merge(x.first, y.second);
    return v;
  }

  Monoid query(std::shared_ptr<Node> t, int a, int b) {
    return query(t, a, b, 0, count(t));
  }

  void set_element(std::shared_ptr<Node> &t, int k, const Monoid &x) {
    t = clone(t);
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

  void push_front(std::shared_ptr<Node> &t, const Monoid &v) {
    t = merge(std::make_shared<Node>(v), t);
  }

  void push_back(std::shared_ptr<Node> &t, const Monoid &v) {
    t = merge(t, std::make_shared<Node>(v));
  }

  Monoid pop_front(std::shared_ptr<Node> &t) {
    auto ret = split(t, 1);
    t = ret.second;
    return ret.first->key;
  }

  Monoid pop_back(std::shared_ptr<Node> &t) {
    auto ret = split(t, count(t) - 1);
    t = ret.first;
    return ret.second->key;
  }
};

#endif  // PIET_ROLLABLE_STACK_RBTREE_HPP
