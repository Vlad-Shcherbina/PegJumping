
class PatchOptimizer {
public:
  int n;
  Board board;
  vector<Move> moves, best_moves;
  int best_score;
  int cnt;
  int iteration_limit;

  PatchOptimizer(Board board, int iteration_limit=1000000000)
      : board(board), iteration_limit(iteration_limit) {
    n = board_size(board);
    best_score = -1;
    best_score = score_upper_bound() - 10;
    cnt = 0;
  }

  int score_upper_bound() {
    int result = 0;
    for (int i = 0; i < n; i++)
      for (int j = 0; j < n; j++)
        if (i % 2 != j % 2 && board[i * n + j] != EMPTY)
          result++;
    return result;
  }

  int naive_score() {
    int num_edges = 0;

    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        int pos = i * n + j;

        if (i % 2 == 0 && j % 2 == 1) {
          // horizontal edge
          if (board[pos] != EMPTY &&
              (j == 0 || board[pos - 1] == EMPTY) &&
              (j == n - 1 || board[pos + 1] == EMPTY))
            num_edges++;
        }

        if (i % 2 == 1 && j % 2 == 0) {
          // vertical edge
          if (board[pos] != EMPTY &&
              (i == 0 || board[pos - n] == EMPTY) &&
              (i == n - 1 || board[pos + n] == EMPTY))
              num_edges++;
        }
      }
    }

    //cerr << num_edges << " edges" << endl;
    return num_edges;
  }

  void rec() {
    if (cnt > iteration_limit) {
      return;
    }
    cnt++;
    if (score_upper_bound() <= best_score) {
      return;
    }

    int score = naive_score();
    if (score > best_score) {
      //cerr << "improvement: " << score << " at " << cnt << endl;
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


class Patcher {
public:
  Board board;
  int n;
  int i0, j0, size;

  Patcher(const Board &board, int i0, int j0, int size)
    : board(board), i0(i0), j0(j0), size(size) {
    n = board_size(board);
    assert(i0 >= 0);
    assert(i0 + size <= n);
    assert(j0 >= 0);
    assert(j0 + size <= n);
  }

  Board get() const {
    Board patch(size * size);
    for (int i = 0; i < size; i++) {
      for (int j = 0; j < size; j++)
        patch[i * size + j] = board[(i + i0) * n + j + j0];
    }
    return patch;
  }

  // void set(const Board &patch) {
  //   assert(board_size(patch) == size);
  //   for (int i = 0; i < size; i++) {
  //     for (int j = 0; j < size; j++)
  //       board[(i + i0) * n + j + j0] = patch[i * size + j];
  //   }
  // }

  vector<Move> translate_moves(const vector<Move> &patch_moves) {
    vector<Move> result;
    for (const Move &move : patch_moves) {
      assert(move.middle == EMPTY);

      int start = move.start;
      assert(start >= 0);
      assert(start < size*size);
      start = (start/size + i0) * n + start % size + j0;

      int end = move.start + move.delta;
      assert(end >= 0);
      assert(end < size*size);
      end = (end/size + i0) * n + end % size + j0;

      result.emplace_back(start, end - start);
    }
    return result;
  }
};


vector<Move> divide_and_optimize(Board board, int tile_size, int preferred_parity) {
  TimeIt t("divide_and_optimize");
  assert(tile_size % 2 == 0);
  int n = board_size(board);

  vector<Move> all_moves;

  for (int q = 0; q < 2; q++) {
    if (check_deadline())
      break;

    vector<Patcher> patchers;
    vector<Move> moves;

    int base_i = (rand() % tile_size) / 2 * 2;
    int base_j = (rand() % tile_size) / 2 * 2;
    for (int i = base_i + preferred_parity; i + tile_size <= n; i += tile_size) {
      for (int j = base_j; j + tile_size <= n; j += tile_size) {
        if (check_deadline())
          break;
        patchers.emplace_back(board, i, j, tile_size);
        //cerr << "tile at " << i << ", " << j << " of size " << tile_size << endl;
        //cerr << "before:" << endl;

        auto patch = patchers.back().get();
        //cerr << show_edges(patch, 0, 0) << endl;

        PatchOptimizer po(patch, 500000);
        add_work(0.1);

        {
          TimeIt t("optimizing_patch");
          po.rec();
        }
        for (auto move : po.best_moves) {
          move.apply(patch);
          moves.push_back(patchers.back().translate_moves({move}).front());
        }

        //auto patch_moves = po(patch)

        //cerr << "after:" << endl;
        //cerr << show_edges(patch, 0, 0) << endl;
      }
    }

    for (auto move : moves) {
      move.apply(board);
      all_moves.push_back(move);
    }
  }

  cerr << "whole board after: " << endl;
  cerr << show_edges(board, preferred_parity, 0) << endl;
  cerr << endl;
  return all_moves;
}
