//#define NDEBUG
#include "common.h"


typedef pair<int, int> Edge;


vector<Edge> collect_edges(int n, const Board &board, int pos) {
  vector<Edge> result;
  for (int delta : get_deltas(n, pos)) {
    if (board[pos + delta] != EMPTY && board[pos + 2 * delta] == EMPTY)
      result.emplace_back(pos, pos + 2 * delta);
  }
  return result;
}


typedef map<int, vector<int>> Graph;

const int SENTINEL = -42;


void add_edge(Graph &g, const Edge &e) {
  auto p = g.find(e.first);
  if (p != g.end()) {
    if (find(p->second.begin(), p->second.end(), e.second) != p->second.end())
      return;  // edge is already there
  }
  g[e.first].push_back(e.second);
  g[e.second].push_back(e.first);
}

void remove_edge(Graph &g, const Edge &e) {
  auto &adj1 = g.at(e.first);
  auto p = find(adj1.begin(), adj1.end(), e.second);
  assert(p != adj1.end());
  adj1.erase(p);
  // TODO: delete vertex as well

  auto &adj2 = g.at(e.second);
  p = find(adj2.begin(), adj2.end(), e.first);
  assert(p != adj2.end());
  adj2.erase(p);
}


bool has_edge(const Graph &g, const Edge &e) {
  auto p = g.find(e.first);
  if (p == g.end())
    return false;
  return find(p->second.begin(), p->second.end(), e.second) != p->second.end();
}


void draw_graph(Board &board, const Graph &g) {
  for (const auto &kv : g) {
    int v = kv.first;
    for (int w : kv.second) {
      assert((v + w) % 2 == 0);
      board.at((v + w) / 2) = 1;
    }
  }
}

string graph_to_string(int n, const Graph &g) {
  Board board(n*n);
  draw_graph(board, g);
  return show_edges(board, 0, 0);
}

// http://stackoverflow.com/questions/11218746/bridges-in-a-connected-graph
class BridgeForest {
private:
  const Graph &g;
  map<int, int> low;
  map<int, int> pre;
  set<int> visited;
  set<Edge> bridge_set;
  int cnt;

public:
  vector<int> roots;
  vector<vector<int>> children;
  vector<Graph> bridge_blocks;
  vector<Edge> bridge_edges;
  vector<int> parent_block;
  map<int, int> block_by_vertex;


  BridgeForest(const Graph &g, int start = SENTINEL) : g(g) {
    for (auto kv : g) {
      int v = kv.first;
      assert(v != SENTINEL);  // it is used as special value below
      pre[v] = low[v] = -1;
    }
    cnt = 0;

    if (start == SENTINEL) {
      for (auto kv : g) {
        int v = kv.first;
        if (pre[v] == -1) {
          dfs(SENTINEL, v);
        }
      }
      // now traverse again and collect bridge blocks
      for (auto kv : g) {
        int v = kv.first;
        if (visited.count(v) == 0) {
          roots.push_back(bridge_blocks.size());

          bridge_blocks.emplace_back();
          bridge_edges.emplace_back(SENTINEL, v);
          parent_block.emplace_back(SENTINEL);
          children.emplace_back();

          dfs2(SENTINEL, v, roots.back());
        }
      }

    } else {
      // It's possible that start is not in the graph.
      pre[start] = -1;
      low[start] = -1;
      dfs(SENTINEL, start);

      // now traverse again and collect bridge blocks
      int v = start;

      roots.push_back(bridge_blocks.size());

      bridge_blocks.emplace_back();
      bridge_edges.emplace_back(SENTINEL, v);
      parent_block.emplace_back(SENTINEL);
      children.emplace_back();

      dfs2(SENTINEL, v, roots.back());
    }
  }

  void show(ostream &out) const {
    out << "Block by vertex: " << block_by_vertex << endl;
    for (int root : roots) {
      show_tree(out, "  ", root);
    }
    out << "--- end of forest ---" << endl;
  }

  int block_entry_point(int block_index) {
    return bridge_edges.at(block_index).second;
  }

private:
  void dfs(int prev, int v) {
    pre[v] = cnt++;
    low[v] = pre[v];
    auto p = g.find(v);
    if (p == g.end())
      return;
    for (int w : p->second) {
      if (pre[w] == -1) {
        dfs(v, w);
        low[v] = min(low[v], low[w]);
        if (low[w] == pre[w]) {
          assert(low[w] != low[v]);
          bridge_set.emplace(v, w);
        }
      } else if (w != prev) {
        low[v] = min(low[v], pre[w]);
      }
    }
  }

  void dfs2(int prev, int v, int current_block) {
    visited.insert(v);
    block_by_vertex[v] = current_block;
    auto p = g.find(v);
    if (p == g.end())
      return;
    for (int w : p->second) {
      if (visited.count(w) == 0) {
        if (bridge_set.count({v, w}) == 0) {
          add_edge(bridge_blocks[current_block], {v, w});
          dfs2(v, w, current_block);
        } else {
          bridge_blocks.emplace_back();
          bridge_edges.emplace_back(v, w);
          parent_block.emplace_back(current_block);
          children.emplace_back();

          int new_block = bridge_blocks.size() - 1;

          children[current_block].push_back(new_block);
          dfs2(v, w, new_block);
        }
      } else if (w != prev) {
        add_edge(bridge_blocks[current_block], {v, w});
      }
    }
  }

  void show_tree(ostream& out, string indent, int index) const {
    int num_vertices = bridge_blocks[index].size();
    int num_edges = 0;
    for (const auto &kv : bridge_blocks[index])
      num_edges += kv.second.size();
    assert(num_edges % 2 == 0);
    num_edges /= 2;
    out << "block " << index
        << " of size " << make_pair(num_vertices, num_edges)
        << endl;
    for (int child : children[index]) {
      out << indent << bridge_edges[child] << ": ";
      show_tree(out, indent + "  ", child);
      assert(parent_block[child] == index);
    }
  }
};


class ShortestPaths {
private:
  map<int, int> parent;
  map<int, int> distance;
  int start;
public:

  ShortestPaths(const Graph &g, int start) : start(start) {
    for (const auto &kv : g)
      assert(kv.first != SENTINEL);
    queue<pair<int, int>> work;
    work.emplace(start, SENTINEL);

    while (!work.empty()) {
      int v = work.front().first;
      int from = work.front().second;
      work.pop();

      if (distance.count(v) > 0)
        continue;

      assert(parent.count(v) == 0);

      if (from == SENTINEL) {
        distance[v] = 0;
      } else {
        distance[v] = distance[from] + 1;
        parent[v] = from;
      }

      auto p = g.find(v);
      if (p != g.end()) {
        for (int w : p->second)
          if (distance.count(w) == 0)
            work.emplace(w, v);
      }
    }
  }

  int get_distance(int to) const {
    if (distance.count(to) == 0)
      return -1;
    return distance.at(to);
  }

  vector<int> get_path(int to) const {
    vector<int> result;
    if (distance.count(to) == 0)
      return {};
    while (to != start) {
      result.push_back(to);
      to = parent.at(to);
    }
    result.push_back(to);
    reverse(result.begin(), result.end());
    return result;
  }
};


void extend_path(vector<int> &path, const vector<int> &extra) {
  assert(!path.empty());
  assert(!extra.empty());
  assert(path.back() == extra.front());

  copy(extra.begin() + 1, extra.end(), back_inserter(path));
}


vector<int> euler_path(const Graph &g, int from, int to) {
  assert(BridgeForest(g).roots.size() == 1);  // connected

  //cout << g << " " << from << " " << to << endl;
  //cout << graph_to_string(40, g) << endl;

  assert(g.count(from) == 1);
  assert(g.count(to) == 1);

  Graph g1 = g;
  //add_edge(g1, {from, to});

  // we are not using add_edge because we don't want
  // 'ignore existing edge' behavior.
  g1[from].push_back(to);
  g1[to].push_back(from);
  //cout << g1 << endl;

  int num_edges = 0;
  for (const auto &kv : g1) {
    assert(kv.second.size() % 2 == 0);
    num_edges += kv.second.size();
  }
  assert(num_edges % 2 == 0);
  num_edges /= 2;

  deque<Edge> path;
  path.emplace_back(to, from);
  remove_edge(g1, path.back());

  while (path.size() < num_edges) {
    //cout << g1 << endl;
    //cout << vector<Edge>(path.begin(), path.end()) << endl;

    int v = path.back().second;
    const auto &adj = g1.at(v);
    if (adj.empty()) {
      assert(path.front().first == v);
      path.push_back(path.front());
      path.pop_front();
    } else {
      path.emplace_back(v, adj.back());
      remove_edge(g1, path.back());
    }
  }

  //cout << g1 << endl;
  //cout << vector<Edge>(path.begin(), path.end()) << endl;
  for (int i = 0; i < path.size(); i++)
    assert(path[i].second == path[(i + 1) % path.size()].first);

  vector<int> result;

  auto aux = find(path.begin(), path.end(), Edge(to, from));
  assert(aux != path.end());
  for (auto p = aux; p != path.end(); p++)
    result.push_back(p->second);
  for (auto p = path.begin(); p != aux; p++)
    result.push_back(p->second);

  return result;
}


vector<int> longest_path(const Graph &g, int from, int to);


vector<int> longest_path_in_2_edge_connected(const Graph &g, int from, int to) {
  //cout << "lpi2ec " << g << " " << from << " " << to << endl;
  if (g.empty()) {
    if (from == to)
      return {to};
    else
      return {};
  }
  assert(BridgeForest(g).roots.size() == 1);

  assert(g.count(from) == 1);
  assert(g.count(to) == 1);

  vector<int> odd;
  for (const auto &kv : g) {
    int v = kv.first;
    int x = kv.second.size();
    if (v == from) x++;
    if (v == to) x++;
    if (x % 2 == 1)
      odd.push_back(v);
  }
  //cout << "odd: " << odd << endl;
  assert(odd.size() % 2 == 0);

  if (odd.empty()) {
    auto result = euler_path(g, from, to);
    assert(!result.empty());
    return result;
  }

  Graph g1 = g;

  sort(odd.begin(), odd.end());
  // TODO: it's slow and bad.
  int trimmed = 0;
  for (int i = 1; i < odd.size(); i += 2) {
    Edge e = {odd[i - 1], odd[i]};
    if (has_edge(g1, e)) {
      remove_edge(g1, e);
      trimmed++;
      //return false;
    }
  }
  if (trimmed > 0) {
    //cout << "trimmed " << trimmed << endl;
  } else {
    assert(odd.size() >= 2);
    ShortestPaths sp(g, odd[0]);
    vector<int> to_remove = sp.get_path(odd[1]);
    for (int i = 2; i < odd.size(); i++) {
      if (sp.get_distance(odd[i]) < to_remove.size())
        to_remove = sp.get_path(odd[i]);
      //auto hz = sp.get_path(odd[i]);
      //if (hz.size() < to_remove.size()) to_remove = hz;
    }
    assert(!to_remove.empty());
    for (int i = 1; i < to_remove.size(); i++) {
      remove_edge(g1, {to_remove[i - 1], to_remove[i]});
    }
  }

  auto result = longest_path(g1, from, to);

  if (!result.empty()) {
    return result;
  } else {
    // TODO: we are returning shortest path. It's terrible.
    ShortestPaths sp(g, from);
    result = sp.get_path(to);
    assert(!result.empty());
    return result;
  }
}


vector<int> longest_path_in_bridge_forest(const BridgeForest &bf, int from, int to) {
  //assert(bf.block_by_vertex.count(from) > 0);
  if (bf.block_by_vertex.count(to) == 0)
    return {};

  int start_block = bf.block_by_vertex.at(from);

  vector<vector<int>> fragments;  // in reverse order

  int v = to;
  //bf.show(cout);
  //cout << "from: " << from << ", to: " << to << endl;
  while (bf.block_by_vertex.at(v) != start_block) {
    int i = bf.block_by_vertex.at(v);

    Edge e = bf.bridge_edges[i];
    assert(bf.parent_block[i] != SENTINEL);
    assert(bf.block_by_vertex.at(e.first) == bf.parent_block[i]);

    auto hz = longest_path_in_2_edge_connected(bf.bridge_blocks[i], e.second, v);
    assert(!hz.empty());
    fragments.push_back(hz);
    fragments.push_back({e.first, e.second});
    v = e.first;
  }

  auto hz = longest_path_in_2_edge_connected(bf.bridge_blocks[start_block], from, v);
  assert(!hz.empty());
  fragments.push_back(hz);

  vector<int> result = {from};
  for (int i = fragments.size() - 1; i >= 0; i--)
    extend_path(result, fragments[i]);

  return result;
}


vector<int> longest_path(const Graph &g, int from, int to) {
  BridgeForest bf(g, from);
  return longest_path_in_bridge_forest(bf, from, to);
}


vector<int> longest_path_from(const Graph &g, int from) {
  BridgeForest bf(g, from);
  assert(bf.roots == vector<int>{0});
  assert(bf.block_entry_point(0) == from);

  vector<vector<int>> best_path(bf.bridge_blocks.size());

  for (int i = best_path.size() - 1; i >= 0; i--) {

    const Graph &block = bf.bridge_blocks[i];

    // TODO: find best paths within a block.
    best_path[i] = {bf.block_entry_point(i)};

    for (int child : bf.children[i]) {
      assert(child > i);
      Edge e = bf.bridge_edges[child];
      vector<int> path = longest_path_in_2_edge_connected(
        block, bf.block_entry_point(i), e.first);
      extend_path(path, {e.first, e.second});
      extend_path(path, best_path[child]);

      if (path.size() > best_path[i].size()) {
        //cout << path << best_path[i] << endl;
        assert(path.front() == best_path[i].front());
        best_path[i] = path;
      }
    }
  }

  return best_path[0];
}
