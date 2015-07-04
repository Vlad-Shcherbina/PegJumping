//#define NDEBUG

#include "common.h"
#include "bridges.h"



void expand_euler_graph(Graph &eg, Graph &extra) {
  // make copy because we are mutating eg
  for (const auto &kv : Graph(eg)) {
    int start = kv.first;

    auto p = extra.find(start);
    if (p == extra.end() || p->second.empty())
      continue;

    ShortestPaths sp_orig(eg, start);

    for (int w : p->second) {
      cerr << "Considering departure " << start << " -> " << w << endl;

      remove_edge(extra, {start, w});
      ShortestPaths sp_extra(extra, w);

      bool improved = false;
      for (const auto &kv : sp_extra.distance) {
        int end = kv.first;
        int extra_distance = kv.second;

        int orig_distance = sp_orig.get_distance(end);
        if (orig_distance == -1 or orig_distance >= extra_distance + 1)
          continue;

        vector<int> extra_path = sp_extra.get_path(end);
        extra_path.insert(extra_path.begin(), start);

        vector<int> orig_path = sp_orig.get_path(end);

        assert(!orig_path.empty());
        assert(!extra_path.empty());
        assert(orig_path.front() == extra_path.front());
        assert(orig_path.back() == extra_path.back());
        cerr << "found improvement" << extra_path << orig_path << endl;
        assert(extra_path.size() > orig_path.size());

        add_edge(extra, {start, w});  // we will immediately remove it

        for (int i = 1; i < orig_path.size(); i++) {
          Edge e(orig_path[i - 1], orig_path[i]);
          remove_edge(eg, e);
          add_edge(extra, e);
        }
        for (int i = 1; i < extra_path.size(); i++) {
          Edge e(extra_path[i - 1], extra_path[i]);
          add_edge(eg, e);
          remove_edge(extra, e);
        }

        improved = true;
        break;
      }

      // sp_orig was ivalidated,
      // p->second we are iterating on was mutated
      if (improved)
        break;

      add_edge(extra, {start, w});
    }
  }
}


// DOES NOT WORK BECAUSE EULER REPRESENTATION DOES NOT MAINTAIN CONNECTIVITY
vector<int> longest_path_by_expansion(const Graph &g, int from, int to) {
  auto path = ShortestPaths(g, from).get_path(to);
  assert(!path.empty());

  Graph eg;

  // needed in expand_euler_graph
  eg[from] = {};
  eg[to] = {};

  Graph remainder = g;
  for (int i = 1; i < path.size(); i++) {
    Edge e(path[i - 1], path[i]);
    add_edge(eg, e);
    remove_edge(remainder, e);
  }

  while (true) {
    auto old_eg = eg;
    auto old_remainder = remainder;

    expand_euler_graph(eg, remainder);
    if (eg == old_eg) {
      assert(remainder == old_remainder);
      break;
    }

    cerr << "eg:" << endl;
    cerr << graph_to_string(20, eg) << endl;
    cerr << "remainder:" << endl;
    cerr << graph_to_string(20, remainder) << endl;

    //break;
  }


  return euler_path(eg, from, to);
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

  cerr << "v = " << v << endl;

  // auto path = longest_path_in_2_edge_connected(largest, v, v);
/*  auto path = ShortestPaths(largest, v).get_path(v + 2 * n);
  cerr << path << endl;
  Graph eg;
  for (int i = 1; i < path.size(); i++)
    add_edge(eg, {path[i - 1], path[i]});

  cerr << graph_to_string(n, eg) << endl;
*/

  auto path = longest_path_by_expansion_raw(largest, v, v + 2*n);
  cerr << path.size() << endl;
  cerr << path_to_string(n, path) << endl;



  { TimeIt t("old_ways");
  cerr << "compared to old " << longest_path_in_2_edge_connected(largest, v, v).size() << endl;
  }
  /*auto path = longest_path_by_expansion(largest, v, v);

  cerr << path.size() << endl;
  cerr << path_to_string(n, path) << endl;
  */

  print_timers(cerr);
}
