#define NDEBUG

#include "common.h"
#include "bridges.h"

int main() {
  int n = 60;
  Board board;
  for (int i = 0; i < n * n; i++) {
    board.push_back(EMPTY);
    int p = (i % n + i / n) % 2 == 1 ? 750 : 150;
    if (rand() % 1000 < p)
      board.back() = 1;
  }

  cout << board_to_string(board) << endl;
  cout << show_edges(board, 0, 0) << endl;

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

  //BridgeForest bf(g, 4);
  //bf.show(cout);

  //cout << longest_path(g, 0, 4) << endl;

  /*vector<int> odd = odd_vertices(g);
  //set<int> odd_set(odd.begin(), odd.end());
  Graph odd_subg = build_subgraph(g, set<int>(odd.begin(), odd.end()));
  cout << graph_to_string(n, odd_subg) << endl;

  cerr << maximal_matching(odd_subg) << endl;

  Graph q;
  for (auto e : maximal_matching(odd_subg)) add_edge(q, e);
  cout << graph_to_string(n, q) << endl;

  return 0;*/

  BridgeForest(g, 0).show(cout);

  vector<int> path;
  {
    TimeIt t("total");
    path = longest_path_from(g, (n / 4 * 2) * (n + 1));
  }


  cout << path.size() << endl;
  cout << path << endl;

  Graph path_graph;
  for (int i = 1; i < path.size(); i++) {
    add_edge(path_graph, {path[i - 1], path[i]});
  }
  cout << graph_to_string(n, path_graph);
  cout << path.size() << endl;

  #ifdef LP_CACHE
  cout << cache.size() << " cached items" << endl;
  #endif


  print_timers(cout);

  // ShortestPaths sp(bf.bridge_blocks[0], 0);
  //cout << graph_to_string(n, bf.bridge_blocks[3]);
  // cout << sp.get_path(82) << endl;
}
