//#define NDEBUG

#define USE_TIME_IT

#include "common.h"
#include "bridges.h"


int main() {
  int n = 40;
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

  vector<int> path;

  path = longest_path_by_expansion(largest, v, v + 0*n);

  cerr << path.size() << " " << path << endl;
  cerr << path_to_string(n, path) << endl;



  Graph extra(largest);
  assert(!path.empty());
  for (int i = 1; i < path.size(); i++)
    remove_edge(extra, {path[i - 1], path[i]});

  cerr << "remainds: " << endl;
  cerr << graph_to_string(n, extra) << endl;


  cerr << longest_path_stats << endl;

  print_timers(cerr);
}
