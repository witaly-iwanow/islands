# Island counting algorithms (flood fill and beyond)

## Iteration 1: naive recursion

Simplified code (0 is water, -1 is not yet visited piece of "land"):

```cpp
void floodFillRec(Map& islandMap, int islandId, int r, int c, int recDepth) {
    for (int rr = r - 1; rr <= r + 1; ++rr) {
        for (int cc = c - 1; cc <= c + 1; ++cc) {
            if (islandMap[rr][cc] < 0) {
                islandMap[rr][cc] = islandId;
                floodFillRec(islandMap, islandId, rr, cc, recDepth + 1);
            }
        }
    }
}
```
Performance for 302x202 map with 4 islands (`clang++ -std=c++17` on M1 MB Air, macOS 14.1.1): 3.7ms to count the islands (lower is better obviously). Max recursion depth: 59999 (oh my), segfaults on 500x400 and beyond.

## Iteration 2: put neighboring cells in a queue

Store neighbors in a queue and handle them sequentially instead of using recursion:

```cpp
struct Cell {int r, c;};

void getNeighbors(Map& islandMap, int islandId, int r, int c, std::queue<Cell>& neibs) {
    for (int rr = r - 1; rr <= r + 1; ++rr) {
        for (int cc = c - 1; cc <= c + 1; ++cc) {
            if (islandMap[rr][cc] < 0) {
                islandMap[rr][cc] = islandId;
                neibs.push({rr, cc});
            }
        }
    }
}

void floodFill(Map& islandMap, int islandId, int r, int c) {
    islandMap[r][c] = islandId;
    std::queue<Cell> neibs;
    neibs.push({r, c});

    while (!neibs.empty()) {
        auto p = neibs.front();
        neibs.pop();
        getNeighbors(islandMap, islandId, p.r, p.c, neibs);
    }
}
```
Performance: 5.8ms (boooo). Max neighbor queue size: 399, doesn't segfault on 30k x 20k (yay!).

## Iteration 3: hybrid - depth-limited recursion and a queue

Use recursion up to certain depth (1000), then put neighbors in the queue for further processing in the main loop:
```cpp
constexpr int maxRecDepth = 1000;
void getNeighbors(Map& islandMap, int islandId, int r, int c, int recDepth, std::queue<Cell>& neibs) {
    for (int rr = r - 1; rr <= r + 1; ++rr) {
        for (int cc = c - 1; cc <= c + 1; ++cc) {
            if (islandMap[rr][cc] < 0) {
                islandMap[rr][cc] = islandId;
                if (recDepth < maxRecDepth)
                    getNeighbors(islandMap, islandId, rr, cc, recDepth + 1, neibs);
                else
                    neibs.push({rr, cc});
            }
        }
    }
}
```
Performance: 4.1ms. Max neighbor queue size: 1835.

## Intermediate summary

| Method | Performance (ms per map)  |
| --- | --- |
| Recursion | 3.7 |
| Queue | 5.8 |
| Recursion + queue | 4.1 |

The hybrid approach seems to be the best choice - we can ramp up the max depth even further and get very close to performance of the pure recursive method, while enjoying safe handling of large maps. Stay tuned though, it's not the end of the story...
