#define NDEBUG

#include "common.h"
#include "bridges.h"
#include "patch.h"


class LongestPathFinder {
public:
  int n;
  Board &board;

  vector<int> path;
  int sum;

  int best_score;
  vector<int> best_path;

  LongestPathFinder(int n, Board &board)
    : n(n), board(board) {
    best_score = -1;
  }

  vector<Move> get_moves() const {
    assert(best_path.size() >= 2);
    vector<Move> result;
    for (int i = 1; i < best_path.size(); i++) {
      int start = best_path[i - 1];
      int delta = best_path[i] - start;
      assert(delta % 2 == 0);
      delta /= 2;
      result.emplace_back(start, delta);
    }
    return result;
  }

  void find() {
    for (int i = 0; i < board.size(); i++) {
      if (board[i] != EMPTY)
        find_starting(i);
    }
  }

  void find_starting(int pos) {
    path.clear();
    sum = 0;

    Cell backup = board[pos];
    assert(backup != EMPTY);
    board[pos] = EMPTY;
    rec(pos);
    board[pos] = backup;

    assert(sum == 0);
    assert(path.empty());
  }

  void rec(int pos) {
    if (path.size() > 12)  // otherwise it's too slow
      return;
    //cerr << "rec " << pos << endl;
    path.push_back(pos);

    vector<int> deltas = get_deltas(n, pos);
    bool has_moves = false;
    for (int delta : deltas) {
      if (board[pos + delta] == EMPTY) continue;
      if (board[pos + 2*delta] != EMPTY) continue;
      has_moves = true;
      Cell backup = board[pos + delta];
      sum += backup;
      board[pos + delta] = EMPTY;

      rec(pos + 2*delta);

      sum -= backup;
      board[pos + delta] = backup;
    }

    if (!has_moves) {
      int score = (path.size() - 1) * sum;
      //cerr << "score " << score << endl;
      if (score > best_score) {
        //cerr << "better " << score << " " << path << endl;
        //cerr << sum << endl;
        best_path = path;
        best_score = score;
      }
    }

    path.pop_back();
  }
};


const int MIN_RUN = 2;


vector<Move> prepare_blobs(Board &board) {
  vector<Move> result;
  int n = board_size(board);
  cerr << n << endl;

  for (int i = 0; i < n; i++) {
    int run_length = 0;
    for (int j = 0; j < n; j++) {
      int pos = i*n + j;

      if (board[pos] == EMPTY && (i + j) % 2 == 1 && run_length >= MIN_RUN) {
        //cerr << pos << " " << run_length << endl;

        for (int q = 0; q < run_length / 2; q++) {
          result.emplace_back(pos - 2 * q - 2, 1);
          //cerr << result.back() << endl;
          result.back().apply(board);
        }

        run_length = 0;
      }

      if (board[pos] != EMPTY)
        run_length++;
      else
        run_length = 0;
    }
  }

  for (int i = 0; i < n; i++) {
    int run_length = 0;
    for (int j = n - 1; j >= 0; j--) {
      int pos = i*n + j;

      if (board[pos] == EMPTY && (i + j) % 2 == 1 && run_length >= MIN_RUN) {
        //cerr << pos << " " << run_length << endl;

        for (int q = 0; q < run_length / 2; q++) {
          result.emplace_back(pos + 2 * q + 2, -1);
          //cerr << result.back() << endl;
          result.back().apply(board);
        }

        run_length = 0;
      }

      if (board[pos] != EMPTY)
        run_length++;
      else
        run_length = 0;
    }
  }

  for (int i = 0; i < n; i++) {
    int run_length = 0;
    for (int j = 0; j < n; j++) {
      int pos = j*n + i;

      if (board[pos] == EMPTY && (i + j) % 2 == 1 && run_length >= MIN_RUN) {
        //cerr << pos << " " << run_length << endl;

        for (int q = 0; q < run_length / 2; q++) {
          result.emplace_back(pos - 2 * n * q - 2 * n, n);
          //cerr << result.back() << endl;
          result.back().apply(board);
        }

        run_length = 0;
      }

      if (board[pos] != EMPTY)
        run_length++;
      else
        run_length = 0;
    }
  }

  for (int i = 0; i < n; i++) {
    int run_length = 0;
    for (int j = n - 1; j >= 0; j--) {
      int pos = j*n + i;

      if (board[pos] == EMPTY && (i + j) % 2 == 1 && run_length >= MIN_RUN) {
        //cerr << pos << " " << run_length << endl;

        for (int q = 0; q < run_length / 2; q++) {
          result.emplace_back(pos + 2 * n * q + 2 * n, -n);
          //cerr << result.back() << endl;
          result.back().apply(board);
        }

        run_length = 0;
      }

      if (board[pos] != EMPTY)
        run_length++;
      else
        run_length = 0;
    }
  }

  cerr << board_to_string(board) << endl;
  cerr << show_edges(board, 0, 0) << endl;

  return result;
}


vector<Move> pick_long_path(Board &board) {
  int n = board_size(board);
  Graph graph;
  for (int pos = 0; pos < n * n; pos++) {
    if (pos % n % 2 == 0 && pos / n % 2 == 0) {
      if (board[pos] == EMPTY) {
        for (auto e : collect_edges(n, board, pos)) {
          add_edge(graph, e);
        }
      }
    }
  }

  vector<int> longest;

  for (int pos = 0; pos < n*n; pos++) {
    if (pos % n % 2 == 0 && pos / n % 2 == 0) {
      if (board[pos] == EMPTY) continue;

      auto es = collect_edges(n, board, pos);
      //cerr << es << endl;
      if (!es.empty()) {
        Graph g = graph;  // TODO: this copy is a huge source of inefficiency
        for (auto e : es)
          add_edge(g, e);

        vector<int> path = longest_path_from(g, pos);
        if (path.size() > longest.size()) {
          longest = path;
          if (longest.size() > 2*n)
            break;
        }

        // cerr << g << pos << endl;
        // cerr << longest_path.size() << longest_path << endl;
        // cerr << graph_to_string(n, g) << endl;
      }
    }
  }
  cerr << longest << endl;
  if (longest.empty())
    return {};

  vector<Move> result;
  for (int i = 1; i < longest.size(); i++)
    result.emplace_back(longest[i - 1], (longest[i] - longest[i - 1]) / 2);
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

    //int expected_score = 0;

    moves = prepare_blobs(board);

    for (auto move : divide_and_optimize(board, 8)) {
      moves.push_back(move);
      move.apply(board);
    }

    while (true) {
      auto path = pick_long_path(board);
      if (path.size() < 10)
        break;
      for (auto move : path) {
        moves.push_back(move);
        move.apply(board);
      }
    }

    {
      TimeIt t("greedy_brute");
      while (true) {
        Board before = board;
        LongestPathFinder lpf(n, board);
        lpf.find();
        if (lpf.best_score <= 0) break;
        //expected_score += lpf.best_score;
        cerr << "best " << lpf.best_score << lpf.best_path << endl;
        vector<Move> extra_moves = lpf.get_moves();

        assert(before == board);

        for (Move move : extra_moves) {
          moves.push_back(move);
          move.apply(board);
        }
      }
    }

    //cerr << "expected score " << expected_score << endl;
    }
    print_timers(cerr);
    return moves_to_strings(n, moves);
  }

/*
  vector<Move> find_moves(const Board &board, int start) {
    #define TRY_DELTA(delta) \
      if (board[start + delta] != EMPTY && \
          board[start + 2*delta] == EMPTY) { \
        result.emplace_back(start, delta); \
      }

    vector<Move> result;

    if (board[start] == EMPTY)
      return result;

    if (start >= 2 * n) TRY_DELTA(-n);
    if (start % n >= 2) TRY_DELTA(-1);
    if (start % n < n - 2) TRY_DELTA(1);
    if (start < n * (n - 2)) TRY_DELTA(n);
    return result;

    #undef TRY_DELTA
  }
*/
};
