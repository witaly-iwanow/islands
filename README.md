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
Performance for 302x202 map with 4 islands (`clang++ -std=c++17` on M1 MB Air, macOS 14.1.1): 3.3ms to count the islands (lower is better obviously). Max recursion depth: 59999 (oh my), segfaults on 500x400 and beyond.




