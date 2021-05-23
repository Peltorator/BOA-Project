#include <bits/stdc++.h>

using namespace std;

struct Distance {
    int g1;
    int h1;
    int f1;
    int g2;
    int h2;
    int f2;

    Distance(const int g1, const int h1, const int g2, const int h2): g1(g1), h1(h1), f1(g1 + h1), g2(g2), h2(h2), f2(g2 + h2) {}

    Distance(): g1(0), h1(0), f1(0), g2(0), h2(0), f2(0) {}

    bool operator<=(const Distance& other) const {
        return f1 <= other.f1 && f2 <= other.f2;
    }

    bool operator<(const Distance& other) const {
        return f1 <= other.f1 && f2 <= other.f2 && (f1 < other.f1 || f2 < other.f2);
    }

    bool operator>=(const Distance& other) const {
        return f1 >= other.f1 && f2 >= other.f2;
    }

    bool operator>(const Distance& other) const {
        return f1 >= other.f1 && f2 >= other.f2 && (f1 > other.f1 || f2 > other.f2);
    }
};


struct ParetoSet {
    list<pair<int, int>> paretoSet;

    bool dominates(const pair<int, int> dist) const {
        for (const auto paretoDist : paretoSet) {
            if (paretoDist.first <= dist.first && paretoDist.second <= dist.second) {
                return true;
            }
        }
        return false;
    }

    void add(const pair<int, int> dist) {
        list<pair<int, int>> newParetoSet;
        for (const auto paretoDist : paretoSet) {
            if (paretoDist.first <= dist.first && paretoDist.second <= dist.second) {
                return;
            } else if (!(paretoDist.first >= dist.first && paretoDist.second >= dist.second)) {
                newParetoSet.push_back(paretoDist);
            }
        }
        newParetoSet.push_back(dist);
        paretoSet = newParetoSet;
    }

    void push(const pair<int, int> g_value) {
        paretoSet.push_back(g_value);
    }

    void remove(const pair<int, int> g_value) {
        paretoSet.remove(g_value);
    }

    void remove_worse(const pair<int, int> dist) {
        list<pair<int, int>> newParetoSet;
        for (const auto paretoDist : paretoSet) {
            if (paretoDist.first <= dist.first && paretoDist.second <= dist.second) {
                return;
            } else if (!(paretoDist.first >= dist.first
                         && paretoDist.second >= dist.second
                         && (paretoDist.first > dist.first || paretoDist.second > dist.second ))
                    ) {
                newParetoSet.push_back(paretoDist);
            }
        }
        paretoSet = newParetoSet;
    }
};

struct Edge {
    int from;
    int to;
    pair<int, int> cost;

    Edge(const int from, const int to, const pair<int, int>& cost): from(from), to(to), cost(cost) {}
};

struct Graph {
    vector<list<Edge>> adjacencyList;

    Graph(const int n) {
        adjacencyList.resize(n);
    }

    void addEdge(const Edge& edge) {
        adjacencyList[edge.from].emplace_back(edge);
    }

    const list<Edge>& getVertexAdjacencyList(const int v) const {
        return adjacencyList[v];
    }
};

struct Node {
    int index;
    Distance dist;

    Node(): index(0), dist(Distance(0, 0, 0, 0)) {}

    Node(const int index, const Distance& dist): index(index), dist(dist) {}

    bool operator<(const Node& other) const {
        return dist < other.dist;
    }

    bool operator<=(const Node& other) const {
        return dist <= other.dist;
    }

    bool operator>(const Node& other) const {
        return dist > other.dist;
    }

    bool operator>=(const Node& other) const {
        return dist >= other.dist;
    }
};

bool NodeComparator (const Node& first, const Node& second) {
    return make_pair(first.dist.f1, first.dist.f2) > make_pair(second.dist.f1, second.dist.f2);
}

ParetoSet NAMOA_star(const int n,
                     const Graph& graph,
                     const int start,
                     const int goal,
                     const vector<pair<int, int>>& coords,
                     function<int(pair<int, int>, pair<int, int>)> heuristic1,
                     function<int(pair<int, int>, pair<int, int>)> heuristic2) {

    auto h1 = [&] (const int idx) { return heuristic1(coords[idx], coords[goal]); };
    auto h2 = [&] (const int idx) { return heuristic2(coords[idx], coords[goal]); };

    ParetoSet sols;
    map<int, ParetoSet> Gop;
    map<int, ParetoSet> Gcl;
    Gop[start].add({0, 0});

    std::set<Node, decltype(NodeComparator)*> Open(NodeComparator);
    Open.insert(Node(start, Distance(0, h1(start), 0, h2(start))));

    while (!Open.empty()) {
        Node s = *Open.begin();
        int v = s.index;
        Distance curDist = s.dist;
        pair<int, int> g_value = {s.dist.g1, s.dist.g2};
        Open.erase(Open.begin());

        Gop[v].remove(g_value);
        Gcl[v].push(g_value);

        if (v == goal) {
            sols.push(g_value);
            for (auto u: Open) {
                if (s < u) {
                    Open.erase(u);
                }
            }
            continue;
        }

        for (const Edge& e : graph.getVertexAdjacencyList(v)) {
            int u = e.to;
            Distance newDist(curDist.g1 + e.cost.first, h1(u), curDist.g2 + e.cost.second, h2(u));

            bool worse_way = false;
            if (Gop[u].dominates({newDist.g1, newDist.g2})) {
                continue;
            }

            if (Gcl[u].dominates({newDist.g1, newDist.g2})) {
                continue;
            }

            if (sols.dominates({newDist.f1, newDist.f2})) {
                continue;
            }

            Gop[u].add({newDist.g1, newDist.g2});

            Gcl[u].remove_worse({newDist.g1, newDist.g2});

            Open.insert(Node(u, newDist));
        }
    }
    return sols;
}

double GetCurTime() {
    return clock() * 1.0 / CLOCKS_PER_SEC;
}

int main()
{
    string mapName = "NY";

    ifstream coordf("maps/" + mapName + "/coordinates.txt");

    int n;
    coordf >> n;
    std::vector<std::pair<int, int>> coordinates(n);
    for (int i = 0; i < n; i++) {
        int index;
        coordf >> index;
        index--;
        assert(index == i);
        coordf >> coordinates[index].first >> coordinates[index].second;
    }
    coordf.close();

    Graph graph(n);
    long double maxspeed = 0;
    long double maxmult = 0;

    std::ifstream distf("maps/" + mapName + "/distances.txt");
    std::ifstream timef("maps/" + mapName + "/time.txt");
    int m;
    distf >> m;
    for (int i = 0; i < m; i++) {
        int from, to, length;
        distf >> from >> to >> length;
        int from2, to2, time;
        timef >> from2 >> to2 >> time;
        assert(from2 == from && to2 == to);

        from--;
        to--;
        long long dx = coordinates[from].first - coordinates[to].first;
        long long dy = coordinates[from].second - coordinates[to].second;
        if (length != 0) {
            maxmult = std::max(maxmult, sqrtl(dx * dx + dy * dy) / static_cast<long double>(length));
        }
        if (time != 0) {
            maxspeed = std::max(maxspeed, static_cast<long double>(length) / static_cast<long double>(time));
        }
        graph.addEdge(Edge(from, to, {length, time}));
    }
    distf.close();
    timef.close();
    std::string heuristicName = "euclid";
    //std::string heuristicName = "no_heurist";
    //std::string heuristicName = "chebyshev";

    auto h1 = [&](std::pair<int, int> a, std::pair<int, int> b) -> int {
        //return 0;
        long double dx = a.first - b.first;
        long double dy = a.second - b.second;
        return floor(sqrtl(dx * dx + dy * dy) / maxmult);
        //return std::max(a.first - b.first, a.second - b.second) / maxmult;
    };
    auto h2 = [&](std::pair<int, int> a, std::pair<int, int> b) -> int {
        return h1(a, b) / maxspeed;
    };

    const int TESTCASES = 1;
    std::mt19937 rnd(1234);
    std::ofstream outp("results/" + mapName + "/BOAStar_" + heuristicName + ".txt");
    long double sumTime = 0;
    long long sumAnsSize = 0;
    for (int i = 0; i < TESTCASES; i++) {
        int source = rnd() % n, target = rnd() % n;
        std::cerr << "Starting BOAStar search. Map: " << mapName << ", Heuristic: " << heuristicName << " Test #" << i + 1 << std::endl;
        double startTime = GetCurTime();
        ParetoSet ansBOAStar = NAMOA_star(n, graph, source, target, coordinates, h1, h2);

        double workTime = GetCurTime() - startTime;
        std::cerr << "Current task work time = " << workTime << std::endl;
        sumTime += workTime;
        sumAnsSize += ansBOAStar.paretoSet.size();
        std::cerr << "Current average time per task: " << sumTime / (i + 1) << std::endl;
        std::cerr << "Current average Pareto set size per task: " << sumAnsSize / (i + 1) << std::endl;

        ansBOAStar.paretoSet.sort();
        outp << "Optimal set for path from " << source + 1 << " to " << target + 1 << '\n';
        for (const std::pair<int, int>& dist : ansBOAStar.paretoSet) {
            outp << dist.first << " " << dist.second << '\n';
        }
        outp << "\n\n\n";
    }
    std::cerr << "\n\nResults for BOAStar with heuristic '" << heuristicName << "' on map '" << mapName << "'\n";
    std::cerr << "Final average time per task: " << sumTime / TESTCASES << std::endl;
    std::cerr << "Final average Pareto set size per task: " << sumAnsSize / TESTCASES << " (sum of sizes is " << sumAnsSize << ")" << std::endl;
}
