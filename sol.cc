#define NDEBUG

#include <unistd.h>
#include "common.h"
#include "sparsify.h"
#include "bridges.h"


const float TIME_LIMIT = 9.0;


vector<Move> pick_long_path(const Board &board, int i_parity, int j_parity) {
  int n = board_size(board);
  Graph graph;
  for (int pos = 0; pos < n * n; pos++) {
    if (pos % n % 2 == j_parity && pos / n % 2 == i_parity) {
      if (board[pos] == EMPTY) {
        for (auto e : collect_edges(n, board, pos)) {
          add_edge(graph, e);
        }
      }
    }
  }

  vector<int> best;
  int best_score = -1;

  vector<int> poss(n*n);
  iota(poss.begin(), poss.end(), 0);
  shuffle(poss.begin(), poss.end(), std::default_random_engine(42));

  for (int pos : poss) {
    if (pos % n % 2 == j_parity && pos / n % 2 == i_parity) {
      if (best_score > 0 && check_deadline()) {
        cerr << "shit" << endl;
        break;
      }
      // if (rand() % 13 == 0)
      //   cerr << 100 * pos / (n * n) << "%" << endl;

      if (board[pos] == EMPTY) continue;

      auto es = collect_edges(n, board, pos);
      if (!es.empty()) {
        Graph g = graph;
        for (auto e : es)
          add_edge(g, e);

        vector<int> path = longest_path_from(g, pos);
        int score = path_score(board, path);
        if (score > best_score) {
          best = path;
          best_score = score;
        }
      }
    }
  }
  //cerr << best << endl;
  if (best.empty())
    return {};

  vector<Move> result;
  for (int i = 1; i < best.size(); i++)
    result.emplace_back(best[i - 1], (best[i] - best[i - 1]) / 2);
  return result;
}


vector<Move> pick_long_path(const Board &board) {
  add_work(1e-6 * pow(board_size(board), 3));
  vector<Move> best_path;
  int best_score = -1;
  for (int i_parity = 0; i_parity < 2; i_parity++)
    for (int j_parity = 0; j_parity < 2; j_parity++) {
      auto path = pick_long_path(board, i_parity, j_parity);
      int score = path_score(board, path);
      //cerr << "score " << score << endl;
      if (score > best_score) {
        if (best_score > 5000)
          cerr << "# improvement = " << 1.0 * score / best_score << endl;
        best_score = score;
        best_path = path;
      }
    }
  return best_path;
}


class PegJumping {
public:
  int n;

  vector<string> getMoves(vector<int> peg_values, vector<string> board_) {
    auto start_time = get_time();
    set_deadline_from_now(TIME_LIMIT);

    //benchmark_timers(cerr);

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
    preferred_parity = 0;

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

    vector<int> path_scores;
    int i = 0;
    while (true) {
      add_subdeadline(0.8);
      auto long_path = pick_long_path(board);
      deadlines.pop_back();

      if (long_path.empty()) break;
      //cerr << "# long_path = " << long_path.size() << endl;
      int score = path_score(board, long_path);
      path_scores.push_back(score);

      for (auto move : long_path) {
        final_moves.push_back(move);
        move.apply(board);
      }
      i++;

      if (check_deadline())
        break;
    }

    cerr << path_scores << endl;
    if (path_scores.size() >= 2) {
      cerr << "# score_ratio = " << 1.0 * path_scores[1] / path_scores[0] << endl;
    }

    //cerr << "# long_path_ratio = " << 1.0 * long_path2.size() / long_path.size() << endl;

    //cerr << show_edges(board, 0, preferred_parity) << endl;

    }  // TimeIt
    print_timers(cerr);


    #ifdef LP_CACHE
    cerr << "# lp_cache_size = " << cache.size() << endl;
    #endif

    assert(deadlines.size() == 1);
    cerr << "# total_time_for_realz = " << (get_time() - start_time) << endl;
    cerr << "# get_time_counter = " << get_time_counter << endl;

    #ifndef SUBMISSION
    // Just in case, because there were some mysterious problems.
    cerr.flush();
    usleep(200000);
    #endif

    return moves_to_strings(n, final_moves);
  }
};
