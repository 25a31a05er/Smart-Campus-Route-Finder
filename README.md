# Smart Campus Route Finder
**Pragati Engineering College, Surampalem** - ADS&AA Lab Project (B.Tech CSE, Sem 3)

## 1. Description
A console application that finds the shortest walking route between any two of 11 campus
locations using **Dijkstra's algorithm**. The campus is a weighted undirected graph:
vertices = locations, edges = walkable pathways, weights = distance in metres.
Dijkstra is implemented twice (adjacency list + min-heap, adjacency matrix + linear scan)
and the two are timed on the same graph and source.

> **Important:** connections come from the student's description of the campus. All
> distances in `main.cpp` are **SAMPLE values, not measured**. Replace them in the
> `campusEdges[]` table before the final demo.

## 2. File structure
```
campus-route-finder/
|-- main.cpp     (all source code)
|-- README.md    (this file)
`-- VIVA.md      (viva questions and answers)
```

## 3. Features (menu)
1. Display all campus locations
2. Display ASCII campus map
3. Find shortest route (route as location sequence + total metres)
4. Display direct connections of a location
5. Compare Dijkstra: adjacency matrix vs adjacency list (timing)
6. Exit

Handles invalid input, source = destination, and unreachable destinations; any number of
queries per run.

## 4. Compile and run (works offline)
```
g++ -std=c++11 -Wall -O2 main.cpp -o campus_route
./campus_route          (Windows: campus_route.exe)
```

## 5. Editing distances
Open `main.cpp`, find `campusEdges[]`. Each line is `{ from, to, metres }`. Indices are
the positions in `locationNames[]` (0 = Boys' Hostel ... 10 = Parking Area). Weights must
be >= 0; invalid lines are skipped with a warning.

## 6. Graph representation
Vertices (index): 0 Boys' Hostel, 1 EWS Lab, 2 BEEE Lab, 3 Civil Block, 4 AIML Block,
5 Library, 6 Canteen, 7 Main Block, 8 ECE Block, 9 Playground, 10 Parking Area.

Edges (14, undirected):
Hostel-EWS, EWS-BEEE, BEEE-Civil, Civil-AIML, Civil-Library, Library-Canteen,
Library-Main, Canteen-Main, Main-Playground, Playground-ECE, Parking-AIML,
Parking-BEEE, Parking-EWS, Parking-Library.

- **Adjacency list:** `adjList[u]` = vector of `(neighbour, distance)`. Each edge is stored twice (u->v and v->u).
- **Adjacency matrix:** `adjMatrix[u][v]` = distance, `INF` if no direct edge, 0 on the diagonal. Symmetric because the graph is undirected.

Both are built from the same `campusEdges[]` table, so the graph is identical.

## 7. Dijkstra step by step
Greedy idea: always finalise the unvisited vertex with the smallest known distance; its
distance can never improve later because all weights are non-negative.

1. `dist[source] = 0`, every other `dist = INF`, all `parent = -1`.
2. Pick the unvisited vertex `u` with the smallest `dist` (heap top, or linear scan in the matrix version).
3. Mark `u` final.
4. **Relax** each neighbour `v` of `u`: if `dist[u] + w(u,v) < dist[v]`, set `dist[v]` to the new value and `parent[v] = u`.
5. Repeat 2-4 until no reachable vertex is left.
6. **Path:** start at the destination, follow `parent[]` back to the source, reverse the list.

Worked example (sample distances), source = Library:

| Step | Vertex finalised | Updates made |
|---|---|---|
| 1 | Library (0) | Canteen 30, Civil 60, Main 70, Parking 75 |
| 2 | Canteen (30) | Main 70 -> **65** (via Canteen) |
| 3 | Civil (60) | AIML 95, BEEE 105 |
| 4 | Main (65) | Playground 120 |
| 5 | Parking (75) | EWS 120 (AIML 105 and BEEE 115 are worse, no change) |
| 6 | AIML (95) | none |
| 7 | BEEE (105) | none (EWS via BEEE = 130 > 120) |
| 8 | Playground (120) | ECE 200 |
| 9 | EWS (120) | Hostel 160 |
| 10-11 | Hostel (160), ECE (200) | none |

So Library -> ECE Block = 200 m via Canteen, Main Block, Playground; Library -> Boys' Hostel = 160 m via Parking Area, EWS Lab.

## 8. Adjacency matrix vs adjacency list
| | Matrix | List |
|---|---|---|
| Space | O(V^2) | O(V + E) |
| Check edge (u,v) | O(1) | O(degree of u) |
| Visit all neighbours of u | O(V) | O(degree of u) |
| Dijkstra time | O(V^2) | O((V + E) log V) with min-heap |
| Best for | dense graphs | sparse graphs (campus maps) |

Here V = 11 and E = 14, so the graph is sparse, but V is so small that timings are
dominated by constant factors. The matrix version can even win, because a heap has
overhead. The list's advantage only appears for large sparse graphs. Timing repeats each
version 100000 times so the clock can measure it.

## 9. Complexity summary
- Dijkstra with list + binary heap: **O((V + E) log V)** time, **O(V + E)** space
- Dijkstra with matrix + linear scan: **O(V^2)** time, **O(V^2)** space
- Path reconstruction: O(V)
- Extra arrays `dist`, `parent`, `visited`: O(V)

## 10. Sample console output
```
Shortest route : Library -> Canteen -> Main Block
Total distance : 65 metres

Source: Library   Destination: ECE Block
Adjacency List   : 200 m   Path: Library -> Canteen -> Main Block -> Playground -> ECE Block
Adjacency Matrix : 200 m   Path: Library -> Canteen -> Main Block -> Playground -> ECE Block
Runs per version : 100000
List   total time : 19257 microseconds (192 ns per run)
Matrix total time : 16494 microseconds (164 ns per run)
```
(Timings vary per machine and run.)

## 11. Limitations
- Distances are samples until measured.
- The ASCII map is only a drawing; routing uses the graph data.
- The photograph gives approximate layout only, not verified path connections.
- Main Block-ECE and Library-Playground are not edges because they were not in the described routes. Add a line in `campusEdges[]` if a real path exists.
