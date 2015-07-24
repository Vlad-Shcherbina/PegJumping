#include "common.h"
#include "bridges.h"


void check_frontier(const Frontier &f) {
  for (int i = 1; i < f.size(); i++) {
    assert(f[i - 1].first < f[i].first);
    assert(f[i - 1].second > f[i].second);
  }
}

void build_frontier(Frontier &f) {
  if (f.empty()) {
    return;
  }

  sort(f.begin(), f.end());
  reverse(f.begin(), f.end());

  int i = 1;
  for (int j = 1; j < f.size(); j++) {
    bool dominated = false;
    for (int k = 0; k < i; k++) {
      if (f[k].second >= f[j].second) {
        dominated = true;
        break;
      }
    }
    if (!dominated) {
      f[i++] = f[j];
    }
  }

  f.erase(f.begin() + i, f.end());
  reverse(f.begin(), f.end());

  check_frontier(f);
}


typedef map<pair<int, int>, vector<int>> PathFamily;

void cleanup_path_family(PathFamily &pf) {
  Frontier frontier;
  for (const auto &kv : pf) {}
}

// PathFamily merge_path_families(const PathFamily &pf1, const PathFamily &pf2) {

// }


int main() {
  Frontier f = {{1, 1}, {2, 3}, {3, 2}, {1, 4}, {0, 5}, {5, 0}};

  build_frontier(f);
  cerr << f << endl;

  return 0;
}
