#ifndef PRETTY_PRINTING
#define PRETTY_PRINTING

#include <ostream>
#include <vector>
#include <map>
#include <set>
#include <utility>
#include <tuple>


template<typename T>
std::ostream& operator<<(std::ostream &out, const std::vector<T> &v) {
  out << "[";
  bool first = true;
  for (const auto &e : v) {
    if (!first)
      out << ", ";
    first = false;
    out << e;
  }
  out << "]";
  return out;
}

template<typename T>
std::ostream& operator<<(std::ostream &out, const std::set<T> &s) {
  out << "{";
  bool first = true;
  for (const auto &e : s) {
    if (!first)
      out << ", ";
    first = false;
    out << e;
  }
  out << "}";
  return out;
}

template<typename K, typename V>
std::ostream& operator<<(std::ostream &out, const std::map<K, V> &m) {
  out << "{";
  bool first = true;
  for (const auto &kv : m) {
    if (!first)
      out << ", ";
    first = false;
    out << kv.first << ": " << kv.second;
  }
  out << "}";
  return out;
}

template<typename T1, typename T2>
std::ostream& operator<<(std::ostream &out, const std::pair<T1, T2> &p) {
  out << "(" << p.first << ", " << p.second << ")";
  return out;
}


// Tuple operations are messy.

template<int prefix_size, typename... ElemTypes>
struct _print_tuple_elems {
  static void call(std::ostream &out, const std::tuple<ElemTypes...> &t) {
    _print_tuple_elems<prefix_size - 1, ElemTypes...>::call(out, t);
    out << ", " << std::get<prefix_size - 1>(t);
  }
};

template<typename... ElemTypes>
struct _print_tuple_elems<1, ElemTypes...> {
  static void call(std::ostream &out, const std::tuple<ElemTypes...> &t) {
    out << std::get<0>(t);
  }
};

template<typename... ElemTypes>
struct _print_tuple_elems<0, ElemTypes...> {
  static void call(std::ostream &out, const std::tuple<ElemTypes...> &t) {
  }
};

template<typename... ElemTypes>
std::ostream& operator<<(std::ostream &out, const std::tuple<ElemTypes...> &t) {
  out << "(";
   _print_tuple_elems<std::tuple_size<std::tuple<ElemTypes...>>::value, ElemTypes...>::call(out, t);
  out << ")";
  return out;
}


#endif
