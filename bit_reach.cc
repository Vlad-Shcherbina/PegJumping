#include "common.h"


class CombinedTracker {
public:
  int n;

  vector<int> pattern_offsets;

  CombinedTracker(int n, vector<int> pattern_offsets)
    : n(n), pattern_offsets(pattern_offsets) {
    assert(pattern_offsets.size() <= 6);  // log BitPowerset bitness
    assert(find(pattern_offsets.begin(), pattern_offsets.end(), 0) !=
           pattern_offsets.end());
  }

  vector<BitPowerset> from_board(const Board &board) const {
    vector<BitPowerset> powersets;
    assert(board.size() == n*n);
    for (int i = 0; i < n * n; i++) {
      int set = 0;
      for (int j = 0; j < pattern_offsets.size(); j++) {
        int offset = pattern_offsets[j];
        if (board[(i + offset + n*n) % (n*n)] != EMPTY) {
          set |= 1 << j;
        }
      }
      powersets.push_back(ONE << set);
    }
    return powersets;
  }

  string show_raw(const vector<BitPowerset> &powersets) const {
    ostringstream out;
    out << "offsets: " << pattern_offsets << endl;
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++)
        out << hex << setw(3) << powersets[i * n + j];
      out << endl;
    }
    return out.str();
  }
};


class CompiledPattern {
public:
  int n;
  int n2;
  typedef tuple<int, BitPowerset, int> Mask;  // (offset, mask, shift_to_invert)
  vector<Mask> masks;

  //const CombinedTracker *ct;

  //CompiledPattern(const CompiledPattern&) = default;

  CompiledPattern(const CombinedTracker &ct, const map<int, bool> &shape) {
    n = ct.n;
    n2 = n * n;

    map<int, bool> shape_mod;
    for (auto kv : shape)
      shape_mod[(kv.first + n2) % n2] = kv.second;

    for (int i = 0; i < n2; i++) {
      BitPowerset mask = ~ZERO;
      int shift = 0;
      for (int j = 0; j < ct.pattern_offsets.size(); j++) {
        int offset = ct.pattern_offsets[j];
        int point = (i + offset + n2) % (n2);
        auto p = shape_mod.find(point);
        if (p == shape_mod.end())
          continue;
        int delta = 1 << j;
        BitPowerset divisor = (ONE << delta) + 1;
        //cerr << "div: " << (~ZERO) << " " << divisor << " " << j << "!" << endl;
        assert(~ZERO % divisor == 0);
        BitPowerset local_mask = ~ZERO / divisor;
        if (p->second) {
          mask &= local_mask << delta;
          shift -= delta;
        } else {
          mask &= local_mask;
          shift += delta;
        }
      }
      assert((mask == ~ZERO && shift == 0) || (mask != ~ZERO && shift != 0));
      if (shift) {
        masks.emplace_back(i, mask, shift);
      }
    }
  }

  int num_matches(const vector<BitPowerset> &powersets, int pos) const {
    int result = 0;
    for (const auto &mask : masks) {
      if (powersets[(pos + std::get<0>(mask) + n2) % n2] & std::get<1>(mask))
        result++;
    }
    return result;
  }

  void maybe_flip(vector<BitPowerset> &powersets, int pos) const {
    for (const auto &mask : masks) {
      int index = (pos + std::get<0>(mask) + n2) % n2;
      BitPowerset m = std::get<1>(mask);
      int shift = std::get<2>(mask);
      assert(powersets[index] & m);
      if (shift > 0)
        powersets[index] |= (powersets[index] & m) << shift;
      else
        powersets[index] |= (powersets[index] & m) >> -shift;
      //result++;
    }
  }
};


typedef vector<vector<BitPowerset>> Possibility;


class MultiPattern {
public:
  vector<CompiledPattern> patterns;
  int max;

  MultiPattern() {}
  //MultiPattern(const MultiPattern&) = default;

  MultiPattern(const vector<CombinedTracker> &cts, const map<int, bool> &shape) {
    max = 0;
    for (const auto &ct : cts) {
      patterns.emplace_back(ct, shape);
      max += patterns.back().masks.size();
    }
  }

  int num_matches(const Possibility &powersetss, int pos) const {
    assert(powersetss.size() == patterns.size());
    int result = 0;
    for (int i = 0; i < patterns.size(); i++) {
      result += patterns[i].num_matches(powersetss[i], pos);
    }
    return result;
  }

  bool possible(const Possibility &p, int pos) const {
    int x = num_matches(p, pos);
    assert(x >= 0);
    assert(x <= max);
    return x == max;
  }

  void maybe_flip(Possibility &p, int pos) const {
    assert(p.size() == patterns.size());
    assert(possible(p, pos));
    for (int i = 0; i < p.size(); i++) {
      patterns[i].maybe_flip(p[i], pos);
    }
  }

  void maybe_flip_if_possible(Possibility &p, int pos) const {
    if (possible(p, pos))
      maybe_flip(p, pos);
  }
};



const set<int> simple_pairs_ = {
  1, 2, 4, 8,
  5,  // *0
  10, // *1
  3,  // 0*
  12, // 1*
  15};// **


class Reasoner {
public:
  int n;
  vector<CombinedTracker> cts;

  MultiPattern zero;
  MultiPattern one;
  vector<MultiPattern> hor_pairs;
  vector<MultiPattern> ver_pairs;

  MultiPattern hop_right;
  MultiPattern hop_left;
  MultiPattern hop_down;
  MultiPattern hop_up;

  Reasoner(int n, vector<vector<int>> pattern_offsetss) : n(n) {
    for (const auto &pattern_offsets : pattern_offsetss) {
      cts.emplace_back(n, pattern_offsets);
    }

    zero = MultiPattern(cts, {{0, 0}});
    one = MultiPattern(cts, {{0, 1}});

    hor_pairs = {
      MultiPattern(cts, {{0, 0}, {1, 0}}),
      MultiPattern(cts, {{0, 0}, {1, 1}}),
      MultiPattern(cts, {{0, 1}, {1, 0}}),
      MultiPattern(cts, {{0, 1}, {1, 1}}),
    };

    ver_pairs = {
      MultiPattern(cts, {{0, 0}, {n, 0}}),
      MultiPattern(cts, {{0, 0}, {n, 1}}),
      MultiPattern(cts, {{0, 1}, {n, 0}}),
      MultiPattern(cts, {{0, 1}, {n, 1}}),
    };

    hop_right = MultiPattern(cts, {{0, 1}, {1, 1}, {2, 0}});
    hop_left = MultiPattern(cts, {{0, 1}, {-1, 1}, {-2, 0}});
    hop_down = MultiPattern(cts, {{0, 1}, {n, 1}, {2*n, 0}});
    hop_up = MultiPattern(cts, {{0, 1}, {-n, 1}, {-2*n, 0}});
  }

  Possibility from_board(const Board &board) const {
    assert(board.size() == n * n);
    Possibility result;
    for (const auto &ct : cts) {
      result.push_back(ct.from_board(board));
    }
    return result;
  }

  string show_raw(const Possibility &possibility) const {
    ostringstream out;
    out << "Possibility:" << endl;
    assert(possibility.size() == cts.size());
    for (int i = 0; i < cts.size(); i++)
      out << cts[i].show_raw(possibility[i]);
    return out.str();
  }

  string show(const Possibility &possibility) const {
    ostringstream out;
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        bool zero_possible = zero.possible(possibility, i * n + j);
        bool one_possible = one.possible(possibility, i * n + j);
        assert(zero_possible || one_possible);
        if (zero_possible)
          if (one_possible)
            out << "(*)";
          else
            out << "(0)";
        else
          if (one_possible)
            out << "(1)";
          else
            assert(false);

        out << " ";
        int hp = 0;
        for (int k = 0; k < 4; k++)
          if (hor_pairs[k].possible(possibility, i * n + j))
            hp |= 1 << k;
        if (simple_pairs_.count(hp))
          out << " ";
        else
          out << hex << hp;
        out << " ";
      }
      out << endl;
      for (int j = 0; j < n; j++) {
        out << " ";
        int vp = 0;
        for (int k = 0; k < 4; k++)
          if (ver_pairs[k].possible(possibility, i * n + j))
            vp |= 1 << k;
        if (simple_pairs_.count(vp))
          out << " ";
        else
          out << hex << vp;
        out << "    ";
      }
      out << endl;
    }
    out << '.' << endl;
    return out.str();
  }

  void maybe_hop(Possibility &p) const {
    for (int i = 0; i < n; i++)
      for (int j = 0; j < n - 2; j++)
        hop_right.maybe_flip_if_possible(p, i*n + j);
    for (int i = 0; i < n; i++)
      for (int j = 2; j < n; j++)
        hop_left.maybe_flip_if_possible(p, i*n + j);

    for (int i = 0; i < n - 2; i++)
      for (int j = 0; j < n; j++)
        hop_down.maybe_flip_if_possible(p, i*n + j);
    for (int i = 2; i < n; i++)
      for (int j = 0; j < n; j++)
        hop_up.maybe_flip_if_possible(p, i*n + j);
  }

  /*static bool eq(const Possibility &p1, const Possibility &p2) {
    assert(p1.size() == p2.size());
    for (int i = 0; i < p1.size(); i++) {
      assert(p1[i].size() == p2[i].size());
      if (!std::equal(p1[i].begin(), p1[i].end(), p2[i].begin()))
        return false;
    }
    return true;
  }*/

  void maybe_hop_all(Possibility &p) const {
    int cnt = 0;
    while (true) {
      //cerr << show(p) << endl;
      cnt++;
      Possibility new_p = p;
      maybe_hop(new_p);
      if (p == new_p) break;
      p = new_p;
    }
    cerr << cnt << " iterations to converge" << endl;
  }
};


int main() {
  int n = 8;
  Board board;
  for (int i = 0; i < n * n; i++) {
    board.push_back(EMPTY);
    if (rand() % 1000 < 124)
      board.back() = 1;
  }

  // board = Board(n*n, 0);
  // board[4*n + 5] = board[4*n + 6] = 1;
  // board[5*n + 5] = board[5*n + 6] = 1;
  // board[4*n + 7] = 1;
  // board[2*n + 5] = 1;
  // board[5*n + 5] = 1;
  // board[3*n + 7] = 1;
  // board[3*n + 3] = 1;


  /*
  CombinedTracker ct(n, {0, 1});
  auto ps = ct.from_board(board);

  cout << ct.show_raw(ps) << endl;

  CompiledPattern cp(ct, {{0, 1}, {1, 0}});

  cout << cp.masks << endl;

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++)
      cout << cp.num_matches(ps, i * n + j) << " ";
    cout << endl;
  }*/

  //Reasoner reasoner(n, {{0, 1, 2}, {0, n, 2*n}, {0, 1, n, n + 1}});
  const int w = 3;
  const int h = 2;
  vector<vector<int>> hz;
  hz.emplace_back();
  for (int i = 0; i < h; i++)
    for (int j = 0; j < w; j++)
      hz.back().push_back(i * n + j);
  hz.emplace_back();
  for (int i = 0; i < h; i++)
    for (int j = 0; j < w; j++)
      hz.back().push_back(j * n + i);
  hz.push_back({0, -1, 1, n, -n});
  hz.push_back({0, 1, 2, 3, 4, 5});
  hz.push_back({0, 1*n, 2*n, 3*n, 4*n, 5*n});

  hz.emplace_back();
  for (int i = 0; i < h; i++)
    for (int j = 0; j < w; j++)
      hz.back().push_back(j * (n + 1) + i * (n - 1));
  hz.emplace_back();
  for (int i = 0; i < h; i++)
    for (int j = 0; j < w; j++)
      hz.back().push_back(j * (n - 1) + i * (n + 1));


  Reasoner reasoner(n, hz);


  cout << board_to_string(board) << endl;

  Possibility p = reasoner.from_board(board);
  //cout << reasoner.show_raw(p);
  //cout << reasoner.show(p);

  // reasoner.zero.maybe_flip(p, 3 * n + 4);
  // reasoner.zero.maybe_flip(p, 4 * n + 4);
  // reasoner.zero.maybe_flip(p, 5 * n + 4);
  //cout << reasoner.show(p);


  //reasoner.hop_right.maybe_flip(p, 31);
  //reasoner.hop_right.maybe_flip(p, 31);
  reasoner.maybe_hop_all(p);

  //cout << reasoner.show_raw(p);
  cout << reasoner.show(p);

/*  cout << make_tuple() << endl;
  cout << make_tuple(1) << endl;
  cout << make_tuple(1, 2) << endl;
  cout << make_tuple(1, 2, 3) << endl;*/
  //cout << tuple<int, int, int>(1, 2, 3) << endl;
}
