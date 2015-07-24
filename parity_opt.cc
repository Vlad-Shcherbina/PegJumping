//#define NDEBUG

#include "common.h"


class Hz {
public:
  int n;
  Board board;
  vector<Move> moves, best_moves;
  //vector<int> degrees;
  int score;
  int best_score;
  int cnt;
  int iteration_limit;

  Hz(Board board, int iteration_limit=1000000000)
      : board(board), iteration_limit(iteration_limit) {
    n = board_size(board);
    best_score = -1;
    cnt = 0;
    //score = 0;
    //degrees = vector<int>(n * n, 0);
  }

  int naive_compute_score() const {
    vector<int> degrees(n*n, 0);
    int edges = 0;
    for (int i = 0; i < n; i += 2)
      for (int j = 0; j < n; j += 2) {
        int pos = i * n + j;
        if (board[pos] != EMPTY) continue;
        if (j + 2 < n && board[pos + 1] != EMPTY && board[pos + 2] == EMPTY) {
          degrees[pos]++;
          degrees[pos + 2]++;
          edges++;
        }
        if (i + 2 < n && board[pos + n] != EMPTY && board[pos + 2*n] == EMPTY) {
          degrees[pos]++;
          degrees[pos + 2*n]++;
          edges++;
        }
      }
    //cerr << board_to_string(degrees) << endl;
    //cerr << edges << " edges" << endl;
    int odd = 0;
    for (int d : degrees)
      if (d % 2)
        odd++;
    //cerr << odd << " odd vertices" << endl;
    return 1000 * edges - 501 * odd;
  }

  void rec() {
    if (cnt > iteration_limit) {
      return;
    }
    cnt++;
    // if (score_upper_bound() <= best_score) {
    //   return;
    // }

    int score = naive_compute_score();
    if (score > best_score) {
      cerr << "improvement: " << score << " at " << cnt << endl;
      best_score = score;
      best_moves = moves;
    }
    for (int pos = 0; pos < n*n; pos++) {
      if (board[pos] == EMPTY)
        continue;
      int i = pos / n;
      int j = pos % n;

      if (j >= 2) try_move(pos, -1);
      if (j < n - 2) try_move(pos, 1);
      if (i >= 2) try_move(pos, -n);
      if (i < n - 2) try_move(pos, n);
    }
  }

  void try_move(int pos, int delta) {
    if (board.at(pos + delta) == EMPTY)
      return;
    if (board.at(pos + 2*delta) != EMPTY)
      return;

    assert(board[pos] != EMPTY);
    assert(board[pos + delta] != EMPTY);
    assert(board[pos + 2*delta] == EMPTY);

    if (!moves.empty()) {
      int last_pos = moves.back().start;
      int last_delta = moves.back().delta;
      if (make_pair(pos, delta) < make_pair(last_pos, last_delta) &&
          moves_commute(pos, delta, last_pos, last_delta))
        return;
    }

    Cell consumed = board[pos + delta];
    board[pos + delta] = EMPTY;
    board[pos + 2*delta] = board[pos];
    board[pos] = EMPTY;

    moves.emplace_back(pos, delta);
    rec();
    moves.pop_back();

    board[pos] = board[pos + 2*delta];
    board[pos + 2*delta] = EMPTY;
    board[pos + delta] = consumed;
  }

};


int main() {
  int n = 11;
  Board board;
  for (int i = 0; i < n * n; i++) {
    board.push_back(EMPTY);
    int p = (i % n + i / n) % 2 == 1 ? 350 : 450;
    if (rand() % 1000 < p)
      board.back() = 1;
  }

  cerr << board_to_string(board) << endl;
  cerr << show_edges(board, 0, 0) << endl;

  Hz hz(board, 100000000);
  //cerr << board_to_string(hz.naive_compute_degrees()) << endl;
  cerr << "initial score: " << hz.naive_compute_score() << endl;

  hz.rec();
  cerr << hz.best_moves << endl;
  for (auto move : hz.best_moves)
    move.apply(board);

  cerr << show_edges(board, 0, 0) << endl;

}
