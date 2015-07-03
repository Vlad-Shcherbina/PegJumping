//#define NDEBUG

#include <unistd.h>
#include "common.h"
#include "sparsify.h"

class PegJumping {
public:
  int n;

  vector<string> getMoves(vector<int> peg_values, vector<string> board_) {
    srand(42);
    vector<Move> final_moves;
    { TimeIt time_it("total");

    n = board_.size();
    cerr << "# n = " << n << endl;

    Board board;
    int non_empty = 0;
    int min_peg_value = 10;
    int max_peg_value = 0;
    for (auto row : board_) {
      assert(row.size() == n);
      for (auto c : row) {
        if (c == '.')
          board.push_back(EMPTY);
        else {
          board.push_back(peg_values.at(c - '0'));
          assert(board.back() != EMPTY);
          min_peg_value = min(min_peg_value, board.back());
          max_peg_value = max(max_peg_value, board.back());
          non_empty++;
        }
      }
    }
    cerr << "# density = " << 1.0 * non_empty / board.size() << endl;

    cerr << board_to_string(board) << endl;

    int even_pegs = 0;
    int odd_pegs = 0;
    int even_value = 0;
    int odd_value = 0;
    for (int pos = 0; pos < n*n; pos++)
      if (board[pos] != EMPTY) {
        if ((pos / n + pos % n) % 2 == 0) {
          even_pegs++;
          even_value += board[pos];
        } else {
          odd_pegs++;
          odd_value += board[pos];
        }
      }

    cerr << "even value: " << even_pegs*even_value << endl;
    cerr << "odd value: " << odd_pegs*odd_value << endl;

    int preferred_parity = 0;
    if (n % 2 == 0 and odd_pegs*odd_value > even_pegs*even_value)
      preferred_parity = 1;

    cerr << "# preferred_parity = " << preferred_parity << endl;

    for (int i = 0; i < 2; i++) {
      TimeIt t("sparsify");
      // horizontally
      for (auto move : full_sparsify(board, preferred_parity)) {
        final_moves.push_back(move);
        move.apply(board);
      }

      // vertically
      for (auto move : transpose_moves(n, full_sparsify(transpose_board(board), preferred_parity))) {
        final_moves.push_back(move);
        move.apply(board);
      }
    }

    cerr << board_to_string(board) << endl;

    cerr << show_edges(board, 0, preferred_parity) << endl;

    }  // TimeIt
    print_timers(cerr);

    #ifndef SUBMISSION
    // Just in case, because there were some mysterious problems.
    cerr.flush();
    usleep(200000);
    #endif

    return moves_to_strings(n, final_moves);
  }
};
