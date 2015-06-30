//#define NDEBUG

#include "common.h"
#include "patch.h"




int main() {
  int n = 24;
  Board board;
  for (int i = 0; i < n * n; i++) {
    board.push_back(EMPTY);
    int p = (i % n + i / n) % 2 == 1 ? 550 : 550;
    if (rand() % 1000 < p)
      board.back() = 1;
  }

  cout << board_to_string(board) << endl;

  // Patcher patcher(board, 2, 2, 8);
  // cout << board_to_string(patcher.get()) << endl;
  // cout << show_edges(patcher.get(), 0, 0) << endl;

  divide_and_optimize(board, 8);

  /*
  PatchOptimizer po(patcher.get(), 100000);
  cout << po.score_upper_bound() << endl;

  po.rec();
  cout << "best: " << po.best_score << " " << po.best_moves << endl;
  cout << patcher.translate_moves(po.best_moves) << endl;
  for (auto move : patcher.translate_moves(po.best_moves)) move.apply(board);

  cout << show_edges(board, 0, 0) << endl;
  cout << po.cnt << endl;*/
}
