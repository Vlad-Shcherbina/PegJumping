#define NDEBUG

#include "common.h"



class ChainEnumerator {
public:
  int n;
  Board &board;

  vector<vector<int>> paths;

  vector<int> path;
  int sum;

  ChainEnumerator(int n, Board &board)
    : n(n), board(board) {
  }

  void find() {
    assert(paths.empty());
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

    if (path.size() >= 2) {
      paths.push_back(path);
    }

    path.pop_back();
  }
};


int sum(const vector<int> xs) {
  int result = 0;
  for (int x : xs) result += x;
  return result;
}


vector<Cell> apply_path(Board &board, const vector<int> &path) {
  vector<Cell> consumed;

  assert(path.size() >= 2);

  Cell moving_peg = board[path.front()];
  assert(moving_peg != EMPTY);
  board[path.front()] = EMPTY;

  int sum = 0;
  for (int i = 1; i < path.size(); i++) {
    assert(board[path[i]] == EMPTY);
    int x = (path[i - 1] + path[i]) / 2;
    assert(board[x] != EMPTY);
    consumed.push_back(board[x]);
    board[x] = EMPTY;
  }

  board[path.back()] = moving_peg;
  return consumed;
}

void undo_path(Board &board, const vector<int> &path, const vector<Cell> &consumed) {
  assert(path.size() - 1 == consumed.size());
  for (int i = 1; i < path.size(); i++) {
    board[(path[i - 1] + path[i]) / 2] = consumed[i - 1];
  }
  Cell t = board[path.back()];
  board[path.back()] = EMPTY;
  board[path.front()] = t;
}


bool paths_commute(vector<int> path1, vector<int> path2) {
  int k = path1.size();
  for (int i = 1; i < k; i++)
    path1.push_back((path1[i - 1] + path1[i]) / 2);
  k = path2.size();
  for (int i = 1; i < k; i++)
    path2.push_back((path2[i - 1] + path2[i]) / 2);

  //cerr << path1 << path2 << endl;
  sort(path1.begin(), path1.end());
  sort(path2.begin(), path2.end());

  vector<int> intersection;
  set_intersection(path1.begin(), path1.end(), path2.begin(), path2.end(), back_inserter(intersection));
  return intersection.empty();
}


class Brute {
public:
  int n;
  Board board;
  int score, best_score;
  int cnt;

  vector<vector<int>> paths, best_paths;

  Brute(int n, const Board &board) : n(n), board(board) {
    score = 0;
    best_score = 0;
    cnt = 0;
  }

  void rec() {
    cnt++;
    if (cnt > 50000000) {
      cout << "EXIT!" << endl;
      return;
    }
    //cerr << "rec" << paths << score << endl;
    if (paths.size() == 1) {
      cerr << paths << "..." << endl;
    }
    if (score > best_score) {
      cerr << "improvement: " << score << paths << endl;
      best_score = score;
      best_paths = paths;
    }

    //Board backup = board;
    ChainEnumerator ce(n, board);
    ce.find();
    //assert(board == backup);

    for (const auto &path : ce.paths) {
      if (!paths.empty()) {
        if (paths.back().back() == path.front())
          continue;
        if (paths.back() < path && paths_commute(paths.back(), path))
          continue;
      }

      auto consumed = apply_path(board, path);
      int path_score = (path.size() - 1) * sum(consumed);

      paths.push_back(path);
      score += path_score;

      rec();

      score -= path_score;
      paths.pop_back();

      undo_path(board, path, consumed);
      // cout << path << endl;
      // cout << board_to_string(board);
      // cout << board_to_string(backup);
      // cout << "-------" << endl;
      //assert(board == backup);
      //board = backup;
    }
  }
};


vector<Move> path_to_moves(const vector<int> &path) {
  assert(path.size() >= 2);
  vector<Move> result;
  for (int i = 1; i < path.size(); i++) {
    int start = path[i - 1];
    int delta = path[i] - start;
    assert(delta % 2 == 0);
    delta /= 2;
    result.emplace_back(start, delta);
  }
  return result;
}


int main() {
  int n = 10;
  Board board;
  for (int i = 0; i < n * n; i++) {
    board.push_back(EMPTY);
    if (rand() % 1000 < 400)
      board.back() = rand() % 9 + 1;
  }

  // for (int i = 4; i < 6; i++)
  //   for (int j = 4; j < 6; j++)
  //     board[i * n + j ] = 5;

  cout << board_to_string(board) << endl;

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
    auto moves = path_to_moves(path);
    cout << moves_to_strings(n, moves) << endl;
    for (auto move : moves) move.apply(board);
    cout << board_to_string(board);
  }

  cout << "digraph {" << endl;
  for (int i = 0; i < b.best_paths.size(); i++) {
    cout << "  " << i << "[label=\"" << i << " (" << (b.best_paths[i].size() - 1) << ")\"];" << endl;
    for (int j = 0; j < i; j++) {
      if (!paths_commute(b.best_paths[i], b.best_paths[j])) {
        bool has_intermediate = false;
        for (int k = j + 1; k < i; k++) {
          if (!paths_commute(b.best_paths[i], b.best_paths[k]) && !paths_commute(b.best_paths[k], b.best_paths[j]))
            has_intermediate = true;
        }
        if (!has_intermediate)
          cout << "  " << j << "->" << i << ";" << endl;
      }
    }
  }
  cout << "}" << endl;

  //cout << paths_commute({1, 7, 11}, {2, 2}) << endl;
}
