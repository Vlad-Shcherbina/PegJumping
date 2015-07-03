#define NDEBUG

#include "common.h"



// Return pairs (pos, length)
vector<pair<int, int>> find_long_runs(const Board &board, int min_length) {
  vector<pair<int, int>> result;
  int n = board_size(board);
  for (int i = 0; i < n; i++) {
    int k = 0;
    for (int j = 0; j < n; j++) {
      int pos = i * n + j;
      if (board[pos] == EMPTY) {
        if (k >= min_length)
          result.emplace_back(pos - k, k);
        k = 0;
      } else {
        k += 1;
      }
    }
    if (k >= min_length)
      result.emplace_back(i * n + n - k, k);
  }

  for (auto e : result) {
    assert(e.first / n == (e.first + e.second - 1) / n);
    assert(e.second >= min_length);
    for (int i = 0; i < e.second; i++)
      assert(board[e.first + i] != EMPTY);
  }

  return result;
}


class PegJumping {
public:
  int n;

  vector<string> getMoves(vector<int> peg_values, vector<string> board_) {
    vector<Move> moves;
    { TimeIt time_it("total");

    n = board_.size();
    cerr << "# n = " << n << endl;

    Board board;
    int non_empty = 0;
    for (auto row : board_) {
      assert(row.size() == n);
      for (auto c : row) {
        if (c == '.')
          board.push_back(EMPTY);
        else {
          board.push_back(peg_values.at(c - '0'));
          assert(board.back() != EMPTY);
          non_empty++;
        }
      }
    }
    cerr << "# density = " << 1.0 * non_empty / board.size() << endl;

    cerr << board_to_string(board) << endl;
    cerr << find_long_runs(board, 4) << endl;

    }  // TimeIt
    print_timers(cerr);

    return moves_to_strings(n, moves);
  }
};
