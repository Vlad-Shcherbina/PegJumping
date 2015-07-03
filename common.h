#ifndef COMMON
#define COMMON

#include <assert.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <set>
#include <stdlib.h>
#include <iomanip>
#include <algorithm>
#include <tuple>
#include <iterator>
#include <queue>
#include <time.h>
#include <unordered_map>

#include "pretty_printing.h"
#include "bit_powersets.h"

using namespace std;



map<string, clock_t> timers;

class TimeIt {
private:
  string name;
public:
  TimeIt(string name) : name(name) {
    timers[name] -= clock();
  }
  ~TimeIt() {
    timers[name] += clock();
  }
};


void print_timers(ostream &out) {
  for (auto kv : timers) {
    out << "# " << kv.first << "_time = " << 1.0 * kv.second / CLOCKS_PER_SEC << endl;
  }
}


typedef int Cell;
const Cell EMPTY = 0;
typedef vector<Cell> Board;


struct Move {
  int start;
  int delta;
  Cell middle;

  Move(int start, int delta, Cell middle)
    : start(start), delta(delta), middle(middle) {}

  Move(int start, int delta)
    : start(start), delta(delta), middle(EMPTY) {}

  void apply(Board &board) {
    // board[start + 2 * delta] = board[start];
    // board[start] = EMPTY;
    // board[start + delta] = middle;

    assert(board.at(start + 2 * delta) == EMPTY);

    assert((board.at(start + delta) != EMPTY && middle == EMPTY) ||
           (board.at(start + delta) == EMPTY && middle != EMPTY));

    assert(board.at(start) != EMPTY);

    board.at(start + 2 * delta) = board.at(start);
    board.at(start) = EMPTY;
    board.at(start + delta) = middle;
  }
};

ostream& operator<<(ostream& out, Move move) {
  if (move.middle == EMPTY)
    out << "Move(" << move.start << ", " << move.delta << ")";
  else
    out << "Move(" << move.start << ", " << move.delta << ", " << move.middle << ")";
  return out;
}


int board_size(const Board& board) {
  for (int i = 0; ; i++) {
    assert(i * i <= board.size());
    if (i * i == board.size())
      return i;
  }
}


string board_to_string(const Board& board) {
  int n = board_size(board);
  ostringstream out;
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      if (board[i*n + j] == EMPTY)
        out << " .";
      else
        out << " " << board[i*n + j];
    }
    out << endl;
  }
  return out.str();
}


char delta_to_char(int delta) {
  if (delta == 1) return 'R';
  if (delta == -1) return 'L';
  if (delta < 0) return 'U';
  return 'D';
}


vector<string> moves_to_strings(int n, const vector<Move> &moves) {
  vector<string> result;
  int last = -1;
  for (auto move : moves) {
    if (move.start != last) {
      ostringstream r;
      r << move.start / n << " " << move.start % n << " ";
      result.push_back(r.str());
    }
    result.back().push_back(delta_to_char(move.delta));
    last = move.start + 2 * move.delta;
  }
  return result;
}

vector<int> get_deltas(int n, int index) {
  assert(index <= n*n);
  vector<int> result;
  if (index >= 2 * n) result.push_back(-n);
  if (index % n >= 2) result.push_back(-1);
  if (index % n < n - 2) result.push_back(1);
  if (index < n * (n - 2)) result.push_back(n);
  return result;
}


string show_edges(const Board &board, int x0, int y0) {
  ostringstream out;

  int n = board_size(board);

  for (int i = 0; i < n; i++) {
    out << " ";
    for (int j = 0; j < n; j++) {
      int pos = i * n + j;
      Cell c = board[pos];

      bool in_row = (i & 1) == y0;
      bool in_col = (j & 1) == x0;

      if (!in_row && !in_col) {
        if (c != EMPTY)
          out << ".";
        else
          out << ' ';
      } else if (in_row && in_col) {
        if (c != EMPTY)
          out << "X";
        else
          out << 'o';
      } else if (in_row && !in_col) {
        if (c != EMPTY)
          if ((j == 0 || board[pos - 1] == EMPTY) &&
              (j == n - 1 || board[pos + 1] == EMPTY))
            out << "-";
          else
            out << ".";
        else
          out << ' ';
      } else if (!in_row && in_col) {
        if (c != EMPTY)
          if ((i == 0 || board[pos - n] == EMPTY) &&
              (i == n - 1 || board[pos + n] == EMPTY))
            out << "|";
          else
            out << ".";
        else
          out << ' ';
      }
      else
        assert(false);

      out << " ";
    }
    out << endl;
  }

  string result = out.str();
  for (int i = 0; i < result.size() - 2; i++) {
    //cout << result.substr(0, 3) << endl;
    if (result.substr(i, 3) == " - ") {
      //assert(false);
      result = result.replace(i, 3, "---");

    }
  }
  return result;
}


bool moves_commute(int pos1, int delta1, int pos2, int delta2) {
  vector<int> all = {
      pos1, pos1 + delta1, pos1 + 2*delta1,
      pos2, pos2 + delta2, pos2 + 2*delta2};
  sort(all.begin(), all.end());
  return unique(all.begin(), all.end()) == all.end();
}


#endif
