#include "common.h"
#include <math.h>


const double phi = 0.5 * (sqrt(5) - 1);


bool can_reach(int n, const Board &board, int pos) {
  if (board[pos] != EMPTY)
    return true;
  int x = pos % n;
  int y = pos / n;

  double sum = 0.0;
  for (int i = 0; i < n*n; i++) {
    if (board[i] == EMPTY)
      continue;
    int x1 = i % n;
    int y1 = i / n;
    int d = abs(x1 - x) + abs(y1 - y);
    //int d = abs(x1 - x - y1 + y);
    sum += pow(phi, d);
  }
  return sum >= 1.0 + phi - 1e-7;
}


bool can_reach(int n, Board &board, vector<int> poss) {
  assert(board.size() == n*n);
  double sum = 0.0;
  Board backup = board;
  for (int pos : poss)
    if (board[pos] != EMPTY) {
      sum += 1.0;
      board[pos] = EMPTY;
      // for (int pos2 : poss)
      //   if (pos2 != pos)
      //     sum -= pow(phi, abs(pos % n - pos2 % n) + abs(pos / n - pos2 / n));
    }
  //cout << "tmp sum" << sum << endl;
  for (int i = 0; i < n*n; i++) {
    if (board[i] == EMPTY)
      continue;
    int x = i % n;
    int y = i / n;
    int d = 200;
    for (int pos : poss) {
      if (board[pos] != EMPTY) continue;
      d = min(d, abs(pos % n - x) + abs(pos / n - y));
    }
    assert(d > 0);
    if (d < 10)
      //cout << "d " << d << endl;
    //int d = abs(x1 - x) + abs(y1 - y);
    //int d = abs(x1 - x - y1 + y);
    sum += pow(phi, d);
  }
  //cout << sum << endl;

  board = backup;
  return sum >= poss.size() - 1e-7;
}


int main() {
  int n = 20;
  Board board;
  for (int i = 0; i < n * n; i++) {
    board.push_back(EMPTY);
    if (rand() % 1000 < 150)
      board.back() = rand() % 9 + 1;
  }

  // for (int i = 4; i < 6; i++)
  //   for (int j = 4; j < 6; j++)
  //     board[i * n + j ] = 5;

  cout << board_to_string(board) << endl;

  {
  Board reachable(n * n);
  for (int i = 0; i < n*n; i++)
    if (can_reach(n, board, i))
      reachable[i] = 1;
  cout << board_to_string(reachable) << endl;
  }

  cout << can_reach(n, board, {n * (n - 1), n * (n - 1) + 1}) << endl;

  //return 0;

  {
  Board reachable(n * n);
  for (int i = 0; i < n*n; i++)
    if (i % n < n - 1 &&
        i / n < n - 1 &&
        can_reach(n, board, {i, i + 1, i + 2}))
      reachable[i] = 1;
  cout << board_to_string(reachable) << endl;
  }



/*
  ChainEnumerator ce(n, board);
  ce.find();
  cout << ce.paths.size() << endl;
  // for (auto path : ce.paths)
  //   cout << path << endl;

  Brute b(n, board);
  b.rec();
  cout << b.best_score << endl;
  cout << b.best_paths << endl;
  cout << b.cnt << endl;

  for (auto path: b.best_paths) {
    cout << moves_to_strings(n, path_to_moves(path)) << endl;
  }
  */

  //cout << paths_commute({1, 7, 11}, {2, 2}) << endl;
}
