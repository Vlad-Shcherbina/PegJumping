//#define NDEBUG

#include <unistd.h>
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


class SimpleOptimizer {
public:
  vector<Move> allowed_moves;
  map<int, int> reward;

  int n;
  Board board;

  vector<int> moves;
  vector<Move> best_moves;

  int best_score;
  int cnt;
  int iteration_limit;
  int max_depth;

  SimpleOptimizer(Board board, int max_depth=10, int iteration_limit=1000000000)
      : board(board), iteration_limit(iteration_limit), max_depth(max_depth) {
    n = board_size(board);
    best_score = -1;
    cnt = 0;
  }

  int get_score() {
    int result = 0;
    for (const auto &kv : reward)
      if (board[kv.first] == EMPTY)
        result += kv.second;
    return result - moves.size();
  }

  void rec() {
    if (cnt > iteration_limit) {
      return;
    }
    cnt++;

    int score = get_score();
    if (score > best_score) {
      //cerr << "improvement: " << score << " at " << cnt << endl;
      best_score = score;
      best_moves.clear();
      for (int i : moves) best_moves.push_back(allowed_moves[i]);
    }

    if (moves.size() >= max_depth)
      return;

    for (int i = 0; i < allowed_moves.size(); i++) {
      int start = allowed_moves[i].start;
      int delta = allowed_moves[i].delta;

      if (board[start] == EMPTY ||
          board[start + delta] == EMPTY ||
          board[start + 2*delta] != EMPTY)
        continue;

      if (!moves.empty() &&
          moves.back() > i &&
          moves_commute(allowed_moves[moves.back()], allowed_moves[i]))
        continue;

      Cell consumed = board[start + delta];
      board[start + delta] = EMPTY;
      board[start + 2*delta] = board[start];
      board[start] = EMPTY;

      moves.push_back(i);
      rec();
      moves.pop_back();

      board[start] = board[start + 2*delta];
      board[start + 2*delta] = EMPTY;
      board[start + delta] = consumed;
    }
  }
};


class BlobPreprocessor {
public:
  Board board;
  Board board_mask;
  int n;
  int preferred_parity;
  vector<tuple<int, int, int>> goals;

  BlobPreprocessor(const Board &board, int preferred_parity)
      : board(board), preferred_parity(preferred_parity) {
    n = board_size(board);
    board_mask = Board(n*n);

    vector<pair<int, int>> long_runs = find_long_runs(board, 5);

    // (left_goal, right_goal, weight)
    for (const auto &run : long_runs) {

      int start = run.first;
      int length = run.second;
      assert(length >= 4);
      int i = start / n;
      int j = start % n;

      int left_parity = (i + j + preferred_parity) % 2;
      int right_parity = (left_parity + length - 1) % 2;

      for (int q = start; q < start + length; q++)
        board_mask[q] = 9;

      int left_goal;
      int right_goal;

      if (j == 0) {
        if (left_parity == 0)
          board_mask[left_goal = start] = 1;
        else {
          board_mask[start] = 0;
          board_mask[left_goal = start + 1] = 1;
        }
      } else {
        if (left_parity == 0) {
          board_mask[left_goal = start] = 1;
        } else {
          board_mask[left_goal = start - 1] = 1;
        }
      }

      if (j + length == n) {
        if (right_parity == 0)
          board_mask[right_goal = start + length - 1] = 1;
        else {
          board_mask[start + length - 1] = 0;
          board_mask[right_goal = start + length - 2] = 1;
        }
      } else {
        if (right_parity == 0) {
          board_mask[right_goal = start + length - 1] = 1;
        } else {
          board_mask[right_goal = start + length] = 1;
        }
      }

      int weight = (i + preferred_parity) % 2 == 0 ? 10 : 7;
      goals.emplace_back(left_goal, right_goal, weight * length);
    }
    //cerr << board_to_string(board_mask) << endl;
  }

  int goal_deficit() const {
    int result = 0;
    for (const auto &goal : goals) {
      if (board[get<0>(goal)] != EMPTY and board[get<1>(goal)] != EMPTY)
        result += get<2>(goal);
    }
    return result;
  }

  vector<Move> optimize_block(int i1, int j1, int i2, int j2) {
    vector<Move> allowed_moves;
    for (const auto &move : moves_in_a_box(n, i1, j1, i2, j2)) {
      if (board_mask[move.start] != 9 &&
          board_mask[move.start + move.delta] != 9 &&
          board_mask[move.start + 2 * move.delta] != 9)
        allowed_moves.push_back(move);
    }
    //cerr << allowed_moves << endl;

    auto is_inside = [=](int pos){
      int i = pos / n;
      int j = pos % n;
      return (i >= i1) && (i < i2) && (j >= j1) && (j < j2);
    };

    map<int, int> reward;
    for (const auto &goal : goals) {
      if (board[get<0>(goal)] != EMPTY and is_inside(get<1>(goal)))
        reward[get<1>(goal)] += get<2>(goal);

      if (board[get<1>(goal)] != EMPTY and is_inside(get<0>(goal)))
        reward[get<0>(goal)] += get<2>(goal);
    }
    //cerr << reward << endl;

    bool all_sat = true;
    for (const auto &kv : reward) {
      if (board[kv.first] != EMPTY)
        all_sat = false;

    }
    if (all_sat) return {};

    SimpleOptimizer so(
        board,
        reward.size() * 2 /* max_depth */, 10000 /* max_iterations */);
    so.reward = reward;
    so.allowed_moves = allowed_moves;

    so.rec();

    assert(so.board == board);
    //cerr << so.best_moves << endl;
    return so.best_moves;
  }

  int try_optimize(int w, int h, vector<Move> &result) {
    auto backup = board;
    for (int base = 0; base < 20; base++) {
      bool improvement = false;
      for (int i = rand() % h; i + h <= n; i += h) {
        for (int j = rand() % w; j + w <= n; j += w) {
          auto moves = optimize_block(i, j, i + h, j + w);
          for (const auto &move : moves) {
            improvement = true;
            move.apply(board);
            result.push_back(move);
          }
        }
      }
      if (!improvement) {
        //cerr << "done" << endl;
        break;
      }
    }
    int gd = goal_deficit();
    board = backup;

    return gd;
  }

  vector<Move> optimize() {
    cerr << "# goal_deficit_before = " << goal_deficit() << endl;

    vector<Move> best_moves;
    int best_score = 1000000;

    for (int i = 0; i < 20; i++) {
      vector<Move> moves;
      int score = try_optimize(4 + rand() % 5, 4 + rand() % 5, moves);
      if (score < best_score) {
        cerr << score << " at " << i << endl;
        best_moves = moves;
        best_score = score;
      }
    }

    for (const auto &move : best_moves) {
      //improvement = true;
      move.apply(board);
      //result.push_back(move);
    }

    cerr << "# goal_deficit_after = " << goal_deficit() << endl;
    cerr << best_moves.size() << endl;
    //cerr << board_to_string(board) << endl;
    return best_moves;
  }
};


vector<Move> sparsify_blobs(Board board, int preferred_parity) {
  //cerr << board_to_string(board) << endl;
  vector<Move> result;
  int n = board_size(board);

  for (const auto &run : find_long_runs(board, 4)) {
    int start = run.first;
    int length = run.second;

    int i = start / n;
    int j = start % n;

    int left_parity = (i + j + preferred_parity) % 2;
    int right_parity = (left_parity + length - 1) % 2;

    if (j > 0) {
      if (left_parity == 1 && board[start - 1] == EMPTY) {
        for (int k = -1; k + 2 < length; k += 2) {
          result.emplace_back(start + k + 2, -1);
          result.back().apply(board);
        }
        continue;
      }
    }
    if (j + length < n) {
      if (right_parity == 1 && board[start + length] == EMPTY) {
        for (int k = length; k - 2 >= 0; k -= 2) {
          result.emplace_back(start + k - 2, 1);
          result.back().apply(board);
        }
        continue;
      }
    }

  }
  //cerr << result << endl;
  return result;
}


vector<Move> full_sparsify(Board board, int preferred_parity) {
  vector<Move> result;

  BlobPreprocessor bp(board, 1 - preferred_parity);
  // cerr << "# goal_deficit_before = " << bp.goal_deficit() << endl;
  // bp.optimize_block(0, 0, 10, 10);

  for (auto move : bp.optimize()) {
    result.push_back(move);
    move.apply(board);
  }

  for (auto move : sparsify_blobs(board, 1 - preferred_parity)) {
    result.push_back(move);
    move.apply(board);
  }
  return result;
}


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

    auto long_runs = find_long_runs(board, 4);
    cerr << long_runs << endl;
    int full_runs_cnt = 0;
    for (const auto &run : long_runs) if (run.second == n) full_runs_cnt++;
    cerr << "# full_runs_cnt = " << full_runs_cnt << endl;

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
