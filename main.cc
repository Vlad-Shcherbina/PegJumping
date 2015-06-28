#include <iostream>

#include "pretty_printing.h"

using namespace std;


// Terrible, but it's topcoder. Everything should be in one file.
#include "sol.cc"


int main(int argc, char **argv) {
  test_bitpowersets();

  int m;
  cin >> m;
  vector<int> peg_values;
  for (int i = 0; i < m; i++) {
    peg_values.push_back(-1);
    cin >> peg_values.back();
  }
  int n;
  cin >> n;

  vector<string> board;
  for (int i = 0; i < n; i++) {
    board.push_back("");
    cin >> board.back();
  }
  //cerr << board << endl;

  vector<string> moves = PegJumping().getMoves(peg_values, board);

  cout << moves.size() << endl;
  for (auto move : moves)
    cout << move << endl;

  return 0;
}
