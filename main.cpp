// ============================================================================
//  Smart Campus Route Finder - Pragati Engineering College, Surampalem
//  ADS&AA Laboratory Project (B.Tech CSE, 3rd semester)
//
//  Finds the shortest walking route between two campus locations using
//  Dijkstra's algorithm on a weighted undirected graph, implemented with
//  BOTH an adjacency list (min-heap) and an adjacency matrix (linear scan),
//  and compares their execution times.
//
//  Compile:  g++ -std=c++11 -Wall -O2 main.cpp -o campus_route
//  Run    :  ./campus_route
// ============================================================================
#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include <chrono>
using namespace std;

// ----------------------------------------------------------------------------
// STAGE 1: CAMPUS DATA  (EDIT THIS SECTION)
// ----------------------------------------------------------------------------
const int V = 11;               // number of vertices (locations)
const int INF = 1000000000;     // "no edge" / "unreachable"

// Vertex index = position in this array (0 to 10). Do not rename locations.
const string locationNames[V] = {
    "Boys' Hostel",   // 0
    "EWS Lab",        // 1
    "BEEE Lab",       // 2
    "Civil Block",    // 3
    "AIML Block",     // 4
    "Library",        // 5
    "Canteen",        // 6
    "Main Block",     // 7
    "ECE Block",      // 8
    "Playground",     // 9
    "Parking Area"    // 10
};

struct Edge {
    int from;
    int to;
    int distance;   // metres, must be >= 0
};

// Walkable pathways (undirected). Connections follow the student's description
// of the campus. DISTANCES BELOW ARE *SAMPLE* VALUES, NOT MEASURED.
// Replace the third number of each line with verified distances in metres.
const Edge campusEdges[] = {
    //  from  to  metres
    {  0,  1,  40 },   // Boys' Hostel  - EWS Lab         (SAMPLE)
    {  1,  2,  25 },   // EWS Lab       - BEEE Lab        (SAMPLE)
    {  2,  3,  45 },   // BEEE Lab      - Civil Block     (SAMPLE)
    {  3,  4,  35 },   // Civil Block   - AIML Block      (SAMPLE)
    {  3,  5,  60 },   // Civil Block   - Library         (SAMPLE)
    {  5,  6,  30 },   // Library       - Canteen         (SAMPLE)
    {  5,  7,  70 },   // Library       - Main Block      (SAMPLE)
    {  6,  7,  35 },   // Canteen       - Main Block      (SAMPLE)
    {  7,  9,  55 },   // Main Block    - Playground      (SAMPLE)
    {  9,  8,  80 },   // Playground    - ECE Block       (SAMPLE)
    { 10,  4,  30 },   // Parking Area  - AIML Block      (SAMPLE)
    { 10,  2,  40 },   // Parking Area  - BEEE Lab        (SAMPLE)
    { 10,  1,  45 },   // Parking Area  - EWS Lab         (SAMPLE)
    { 10,  5,  75 }    // Parking Area  - Library         (SAMPLE)
};
const int EDGE_COUNT = sizeof(campusEdges) / sizeof(campusEdges[0]);

// Number of repetitions used when timing (11 vertices is far too small to
// measure with a single run).
const int TIMING_REPEATS = 100000;

// ----------------------------------------------------------------------------
// STAGE 2: GRAPH REPRESENTATIONS
// ----------------------------------------------------------------------------
// Adjacency list: adjList[u] holds pairs (neighbour, distance)
vector< vector< pair<int, int> > > adjList(V);
// Adjacency matrix: adjMatrix[u][v] = distance, INF if no direct edge
int adjMatrix[V][V];

// Builds both representations from campusEdges so they always hold the same graph.
void buildGraphs() {
    for (int i = 0; i < V; i++) {
        for (int j = 0; j < V; j++) {
            adjMatrix[i][j] = (i == j) ? 0 : INF;
        }
    }
    for (int i = 0; i < EDGE_COUNT; i++) {
        int u = campusEdges[i].from;
        int v = campusEdges[i].to;
        int w = campusEdges[i].distance;

        // Dijkstra requires non-negative weights, so reject bad data.
        if (u < 0 || u >= V || v < 0 || v >= V || w < 0) {
            cout << "Warning: edge " << i + 1 << " ignored (invalid data).\n";
            continue;
        }
        adjList[u].push_back(make_pair(v, w));
        adjList[v].push_back(make_pair(u, w));   // undirected: add both ways
        adjMatrix[u][v] = w;
        adjMatrix[v][u] = w;
    }
}

// ----------------------------------------------------------------------------
// STAGE 3: DIJKSTRA - two versions
// dist[v]   = shortest known distance from source to v
// parent[v] = previous vertex on that shortest path (-1 if none)
// ----------------------------------------------------------------------------

// Version A: adjacency list + min-heap.  Time O((V + E) log V)
void dijkstraList(int source, int dist[], int parent[]) {
    for (int i = 0; i < V; i++) {
        dist[i] = INF;
        parent[i] = -1;
    }
    dist[source] = 0;

    // Min-heap of (distance, vertex); smallest distance is always on top.
    priority_queue< pair<int, int>, vector< pair<int, int> >, greater< pair<int, int> > > pq;
    pq.push(make_pair(0, source));

    while (!pq.empty()) {
        pair<int, int> top = pq.top();
        pq.pop();
        int d = top.first;
        int u = top.second;

        if (d > dist[u]) continue;   // outdated heap entry, skip

        // Relaxation of every edge leaving u
        for (size_t i = 0; i < adjList[u].size(); i++) {
            int v = adjList[u][i].first;
            int w = adjList[u][i].second;
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                parent[v] = u;
                pq.push(make_pair(dist[v], v));
            }
        }
    }
}

// Version B: adjacency matrix + linear scan for the minimum.  Time O(V^2)
void dijkstraMatrix(int source, int dist[], int parent[]) {
    bool visited[V];
    for (int i = 0; i < V; i++) {
        dist[i] = INF;
        parent[i] = -1;
        visited[i] = false;
    }
    dist[source] = 0;

    for (int count = 0; count < V; count++) {
        // Greedy choice: unvisited vertex with the smallest distance
        int u = -1;
        for (int i = 0; i < V; i++) {
            if (!visited[i] && (u == -1 || dist[i] < dist[u])) {
                u = i;
            }
        }
        if (u == -1 || dist[u] == INF) break;   // remaining vertices unreachable
        visited[u] = true;

        // Relaxation: scan the whole row u of the matrix
        for (int v = 0; v < V; v++) {
            if (!visited[v] && adjMatrix[u][v] != INF &&
                dist[u] + adjMatrix[u][v] < dist[v]) {
                dist[v] = dist[u] + adjMatrix[u][v];
                parent[v] = u;
            }
        }
    }
}

// ----------------------------------------------------------------------------
// STAGE 4: PATH RECONSTRUCTION AND DISPLAY
// ----------------------------------------------------------------------------
// Walk back from destination to source using parent[], then reverse.
vector<int> buildPath(const int parent[], int destination) {
    vector<int> path;
    int current = destination;
    while (current != -1) {
        path.push_back(current);
        current = parent[current];
    }
    reverse(path.begin(), path.end());
    return path;
}

void printPath(const vector<int>& path) {
    for (size_t i = 0; i < path.size(); i++) {
        cout << locationNames[path[i]];
        if (i + 1 < path.size()) cout << " -> ";
    }
    cout << "\n";
}

// ----------------------------------------------------------------------------
// INPUT HELPERS (validation)
// ----------------------------------------------------------------------------
// Reads an integer in [low, high]; keeps asking until valid. Exits on EOF.
int readChoice(const string& prompt, int low, int high) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            if (value >= low && value <= high) return value;
            cout << "Invalid choice. Enter a number from " << low << " to " << high << ".\n";
        } else {
            if (cin.eof()) {
                cout << "\nInput ended. Exiting.\n";
                exit(0);
            }
            cin.clear();                 // non-numeric input: reset and discard
            cin.ignore(10000, '\n');
            cout << "Invalid input. Please enter a number.\n";
        }
    }
}

// ----------------------------------------------------------------------------
// MENU OPTIONS
// ----------------------------------------------------------------------------
// Option 1
void displayLocations() {
    cout << "\n--- Campus Locations ---\n";
    for (int i = 0; i < V; i++) {
        cout << " " << (i + 1) << ". " << locationNames[i] << "\n";
    }
}

// Option 2 (visual only - the algorithm uses the graph data, not this drawing)
void displayAsciiMap() {
    cout << "\n--- Simplified Campus Map (not to scale) ---\n\n";
    cout << "                                           +-----------+\n";
    cout << "                                           | CANTEEN   |------+\n";
    cout << "                                           +-----+-----+      |\n";
    cout << "                                                 |            |\n";
    cout << "+---------+  +-----+  +------+  +-------+   +----+----+   +---+----------+   +-------+\n";
    cout << "| BOYS'   |--| EWS |--| BEEE |--| CIVIL |---| LIBRARY |---| MAIN BLOCK   |   | ECE   |\n";
    cout << "| HOSTEL  |  | LAB |  | LAB  |  | BLOCK |   |         |   |              |   | BLOCK |\n";
    cout << "+---------+  +--+--+  +--+---+  +---+---+   +----+----+   +------+-------+   +---+---+\n";
    cout << "                |        |          |            |               |               |\n";
    cout << "             +--+--------+---+  +---+---+        |     +---------+---------+     |\n";
    cout << "             | PARKING AREA  |--| AIML  |        |     | PLAYGROUND        |-----+\n";
    cout << "             | (bus parking) |  | BLOCK |        |     +-------------------+\n";
    cout << "             +-------+-------+  +-------+        |\n";
    cout << "                     |                           |\n";
    cout << "                     +---------------------------+\n\n";
    cout << "Legend:\n";
    cout << "  +-----+  box        = building / lab block\n";
    cout << "  PLAYGROUND box      = open area (not a building)\n";
    cout << "  --- and |           = walkable pathway between two locations\n";
    cout << "  Back row left to right: Hostel, EWS Lab, BEEE Lab, Civil, Library, Main, ECE\n";
    cout << "  Canteen is behind the Library; Playground is in front of the Main Block.\n";
}

// Option 3
void findShortestRoute() {
    displayLocations();
    int source = readChoice("Enter SOURCE location number (1-11): ", 1, V) - 1;
    int destination = readChoice("Enter DESTINATION location number (1-11): ", 1, V) - 1;

    if (source == destination) {
        cout << "\nSource and destination are the same location ("
             << locationNames[source] << "). Distance = 0 m.\n";
        return;
    }

    int dist[V], parent[V];
    dijkstraList(source, dist, parent);   // either version gives the same distance

    if (dist[destination] == INF) {
        cout << "\n" << locationNames[destination] << " is NOT reachable from "
             << locationNames[source] << ".\n";
        return;
    }

    vector<int> path = buildPath(parent, destination);
    cout << "\nShortest route : ";
    printPath(path);
    cout << "Total distance : " << dist[destination] << " metres\n";
    cout << "(Distances are SAMPLE values until replaced with measured data.)\n";
}

// Option 4
void showDirectConnections() {
    displayLocations();
    int loc = readChoice("Select a location number (1-11): ", 1, V) - 1;

    cout << "\nDirect connections of " << locationNames[loc] << ":\n";
    if (adjList[loc].empty()) {
        cout << "  (none)\n";
        return;
    }
    for (size_t i = 0; i < adjList[loc].size(); i++) {
        cout << "  " << locationNames[adjList[loc][i].first]
             << "  (" << adjList[loc][i].second << " m)\n";
    }
}

// Option 5: same graph, same source, same destination for both versions.
void comparePerformance() {
    displayLocations();
    int source = readChoice("Enter SOURCE location number (1-11): ", 1, V) - 1;
    int destination = readChoice("Enter DESTINATION location number (1-11): ", 1, V) - 1;

    int distL[V], parentL[V], distM[V], parentM[V];
    long long checksum = 0;   // uses results so the compiler cannot skip the loops

    // --- time adjacency list version ---
    chrono::steady_clock::time_point startL = chrono::steady_clock::now();
    for (int r = 0; r < TIMING_REPEATS; r++) {
        dijkstraList(source, distL, parentL);
        checksum += distL[destination];
    }
    chrono::steady_clock::time_point endL = chrono::steady_clock::now();

    // --- time adjacency matrix version ---
    chrono::steady_clock::time_point startM = chrono::steady_clock::now();
    for (int r = 0; r < TIMING_REPEATS; r++) {
        dijkstraMatrix(source, distM, parentM);
        checksum += distM[destination];
    }
    chrono::steady_clock::time_point endM = chrono::steady_clock::now();

    long long nsList = chrono::duration_cast<chrono::nanoseconds>(endL - startL).count();
    long long nsMatrix = chrono::duration_cast<chrono::nanoseconds>(endM - startM).count();

    cout << "\n=== Dijkstra: Adjacency List vs Adjacency Matrix ===\n";
    cout << "Source: " << locationNames[source] << "   Destination: "
         << locationNames[destination] << "\n\n";

    if (distL[destination] == INF) {
        cout << "Destination unreachable (both versions agree: "
             << (distM[destination] == INF ? "yes" : "NO") << ").\n";
    } else {
        cout << "Adjacency List   : " << distL[destination] << " m   Path: ";
        printPath(buildPath(parentL, destination));
        cout << "Adjacency Matrix : " << distM[destination] << " m   Path: ";
        printPath(buildPath(parentM, destination));
    }

    cout << "\nRuns per version : " << TIMING_REPEATS << "\n";
    cout << "List   total time : " << nsList / 1000 << " microseconds ("
         << nsList / TIMING_REPEATS << " ns per run)\n";
    cout << "Matrix total time : " << nsMatrix / 1000 << " microseconds ("
         << nsMatrix / TIMING_REPEATS << " ns per run)\n";

    if (nsList < nsMatrix) {
        cout << "Result: adjacency LIST was faster in this run.\n";
    } else if (nsMatrix < nsList) {
        cout << "Result: adjacency MATRIX was faster in this run.\n";
    } else {
        cout << "Result: both took the same time.\n";
    }
    cout << "Note: with only " << V << " vertices the difference is tiny; the list's\n"
         << "advantage grows for large sparse graphs (O((V+E) log V) vs O(V^2)).\n";
    cout << "(checksum " << checksum << " - ignore, prevents loop removal)\n";
}

// ----------------------------------------------------------------------------
// STAGE 5: MENU
// ----------------------------------------------------------------------------
void printMenu() {
    cout << "\n=========== SMART CAMPUS ROUTE FINDER ===========\n";
    cout << " Pragati Engineering College, Surampalem\n";
    cout << " 1. Display all campus locations\n";
    cout << " 2. Display campus map (ASCII)\n";
    cout << " 3. Find shortest route between two locations\n";
    cout << " 4. Display direct connections of a location\n";
    cout << " 5. Compare Dijkstra: adjacency matrix vs adjacency list\n";
    cout << " 6. Exit\n";
    cout << "=================================================\n";
}

int main() {
    buildGraphs();

    int choice = 0;
    while (choice != 6) {
        printMenu();
        choice = readChoice("Enter your choice (1-6): ", 1, 6);
        switch (choice) {
            case 1: displayLocations();      break;
            case 2: displayAsciiMap();       break;
            case 3: findShortestRoute();     break;
            case 4: showDirectConnections(); break;
            case 5: comparePerformance();    break;
            case 6: cout << "\nThank you for using Smart Campus Route Finder.\n"; break;
        }
    }
    return 0;
}
