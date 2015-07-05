//#define NDEBUG

#define USE_TIME_IT

#include "common.h"
#include "bridges.h"


typedef vector<pair<int, int>> Frontier;


void build_frontier(Frontier &f) {
  sort(f.begin(), f.end());
  auto p = unique(f.begin(), f.end());
  f.erase(p, f.end());
  // TODO: remove dominated stuff
}


class Expander {
public:
  map<int, int> prev;
  map<int, vector<int>> children;
  map<int, vector<int>> index_in_path;
  map<int, Frontier> frontiers;

  int best_improvement;
  int best_ancestor;
  pair<int, int> best_left, best_right;

  bool expand(int root, vector<int> &path, Graph &extra) {
    assert(!path.empty());
    for (int i = 0; i < path.size(); i++)
      index_in_path[path[i]].push_back(i);

    //cerr << "index in path " << index_in_path << endl;

    //int root = 3530;

    children[root] = {};

    prev[root] = root;

    queue<int> work;
    work.push(root);

    while (!work.empty()) {
      int v = work.front();
      work.pop();
      for (int w : extra.at(v)) {
        if (prev.count(w) == 0) {
          prev[w] = v;
          work.push(w);

          if (index_in_path.count(w) > 0) {
            //cerr << "hz" << endl;
            int u = w;
            while (true) {
              auto pq = children.insert({prev[u], vector<int>()});
              //assert(pq.)
              auto &cs = pq.first->second;
              // cerr << " ----------------" << endl;
              // cerr << children << endl;
              // cerr << u << " " << prev[u] << " " << cs << endl;
              if (find(cs.begin(), cs.end(), u) != cs.end())
                break;
              cs.push_back(u);
              if (!pq.second)
                break;
              u = prev[u];
            }
          }
        }
      }
    }

    // TODO: remove
    /*Graph tree;
    for (auto kv : children)
      for (int child : kv.second)
        add_edge(tree, {kv.first, child});
    cerr << "tree: " << endl;
    cerr << graph_to_string(60, tree) << endl;*/

    //cerr << children << endl;

    best_improvement = 0;
    //cerr << "children " << children << endl;
    rec(root);
    //cerr << frontiers << endl;

    if (best_improvement > 0) {
      if (best_left.first > best_right.first) {
        swap(best_left, best_right);
      }

      auto left_path = reconstruct(best_ancestor, best_left.first, best_left.second);
      auto right_path = reconstruct(best_ancestor, best_right.first, best_right.second);

      assert(best_left.first < best_right.first);

      /*cerr << "path before: " << path << endl;
      cerr << "best_improvement " << best_improvement << endl;
      cerr << best_ancestor << ", " << best_left << ", " << best_right << endl;
      cerr << left_path << endl;
      cerr << right_path << endl;*/

      auto new_slice = left_path;
      reverse(new_slice.begin(), new_slice.end());
      assert(new_slice.back() == right_path.front());
      copy(right_path.begin() + 1, right_path.end(), back_inserter(new_slice));

      //cerr << new_slice << endl;
      int left_index = best_left.first;
      int right_index = best_right.first;


      for (int i = left_index; i < right_index; i++) {
        Edge e(path[i], path[i + 1]);
        assert(!has_edge(extra, e));
        add_edge(extra, e);
      }
      for (int i = 1; i < new_slice.size(); i++) {
        Edge e(new_slice[i - 1], new_slice[i]);
        assert(has_edge(extra, e));
        remove_edge(extra, e);
      }

      assert(path[left_index] == new_slice.front());
      assert(path[right_index] == new_slice.back());

      new_slice.pop_back();
      assert(new_slice.size() > right_index - left_index);
      slice_assign(path, left_index, right_index, new_slice);

      //cerr << path << endl;
      return true;
    }
    return false;
  }

  void rec(int v) {
    //cerr << v << endl;
    assert(frontiers.count(v) == 0);
    Frontier f;
    if (index_in_path.count(v) > 0) {
      for (int i : index_in_path.at(v))
        f.emplace_back(i, 0);
    }

    if (children.count(v) > 0) {
      for (int child : children.at(v)) {
        rec(child);
        const auto &cf = frontiers.at(child);

        // TODO: faster
        for (auto kv : cf) {
          kv = {kv.first, kv.second + 1};

          for (const auto &kv2 : f) {
            assert(kv.first != kv2.first);
            int improvement = kv.second + kv2.second - abs(kv.first - kv2.first);
            if (improvement > best_improvement) {
              best_improvement = improvement;
              best_ancestor = v;
              best_left = kv2;
              best_right = kv;
            }
          }
        }

        for (const auto &kv : cf) {
          f.emplace_back(kv.first, kv.second + 1);
        }
      }
    }

    frontiers[v] = f;
  }

  vector<int> reconstruct(int v, int index, int depth) {
    vector<int> result;
    while (depth > 0) {
      bool found = false;
      for (int child : children.at(v)) {
        const auto &cf = frontiers.at(child);
        if (find(cf.begin(), cf.end(), make_pair(index, depth - 1)) != cf.end()) {
          result.push_back(v);
          v = child;
          depth--;
          found = true;
          break;
        }
      }
      assert(found);
    }
    const auto &is = index_in_path.at(v);
    assert(find(is.begin(), is.end(), index) != is.end());
    result.push_back(v);
    return result;
  }

};


vector<int> longest_path_by_expansion(const Graph &g, int from, int to) {
  TimeIt t("longest_path_by_expansion");
  auto path = ShortestPaths(g, from).get_path(to);


  Graph extra(g);
  assert(!path.empty());
  for (int i = 1; i < path.size(); i++)
    remove_edge(extra, {path[i - 1], path[i]});

  // TODO: support randomized changes even if new slice has the same length
  // as the old one.

  int seed = 42;
  while (true) {
    vector<int> roots;
    for (const auto &kv : extra) {
      if (!kv.second.empty()) {
        roots.push_back(kv.first);
      }
    }
    shuffle(roots.begin(), roots.end(), std::default_random_engine(seed++));

    bool had_improvement = false;
    for (int root : roots) {
      if (Expander().expand(root, path, extra)) {
        // cerr << "improved" << endl;
        // cerr << "path: " << path << endl;
        // cerr << path_to_string(60, path) << endl;
        assert(is_path_in_graph(g, from, to, path));
        had_improvement = true;
      }
    }
    if (!had_improvement)  {
      break;
    }

    add_work(1e-7*pow(roots.size(), 1.5));
    if (check_deadline())
      break;
  }

  // cerr << "Remains:" << endl;
  // cerr << graph_to_string(60, extra) << endl;

  assert(is_path_in_graph(g, from, to , path));
  return path;
}


int main() {
  int n = 100;
  Board board;
  for (int i = 0; i < n * n; i++) {
    board.push_back(EMPTY);
    int p = (i % n + i / n) % 2 == 1 ? 750 : 150;
    if (rand() % 1000 < p)
      board.back() = 1;
  }

  cerr << board_to_string(board) << endl;
  cerr << show_edges(board, 0, 0) << endl;

  Graph g;
  vector<Edge> all_edges;
  for (int pos = 0; pos < n * n; pos++) {
    if (pos % n % 2 == 0 && pos / n % 2 == 0) {
      if (board[pos] == EMPTY) {
        for (auto e : collect_edges(n, board, pos)) {
          all_edges.push_back(e);
          add_edge(g, e);
        }
      }
    }
  }

  BridgeForest bf(g, 4);
  bf.show(cerr);

  Graph largest;
  for (const Graph &block : bf.bridge_blocks)
    if (num_edges(block) > num_edges(largest))
      largest = block;

  cerr << graph_to_string(n, largest) << endl;

  int v = 100000;
  for (const auto &kv : largest)
    v = min(kv.first, v);

  int w = -1;
  for (const auto &kv : largest)
    w = max(kv.first, w);

  cerr << "v = " << v << endl;
  cerr << "w = " << w << endl;

  // auto path = longest_path_in_2_edge_connected(largest, v, v);
/*  auto path = ShortestPaths(largest, v).get_path(v + 2 * n);
  cerr << path << endl;
  Graph eg;
  for (int i = 1; i < path.size(); i++)
    add_edge(eg, {path[i - 1], path[i]});

  cerr << graph_to_string(n, eg) << endl;
*/

  vector<int> path;
  { TimeIt t("longest_expansion_raw");
  path = longest_path_by_expansion_raw(largest, v, v + 2*n);
  }

  { TimeIt t("longest_expansion");
  path = longest_path_by_expansion(largest, v, v + 2*n);
  }

  cerr << path.size() << " " << path << endl;
  cerr << path_to_string(n, path) << endl;



  // { TimeIt t("old_ways");
  // cerr << "compared to old " << longest_path_in_2_edge_connected(largest, v, v).size() << endl;
  // }

  /*auto path = longest_path_by_expansion(largest, v, v);

  cerr << path.size() << endl;
  cerr << path_to_string(n, path) << endl;
  */

  print_timers(cerr);
}
