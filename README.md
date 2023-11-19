# Island counting algorithms (flood fill and beyond)

## Iteration 1: naive recursion

Simplified code (0 is water, -1 is not-yet-visited piece of "land"):

```
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

```
struct Cell {int r; int c;};

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



