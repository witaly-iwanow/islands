#include <chrono>

#include "islands-shared.h"

static int g_maxRecDepth = 0;

void floodFillRec(Map& islandMap, int islandId, int r, int c, int recDepth) {
    if (recDepth > g_maxRecDepth)
        g_maxRecDepth = recDepth;

    for (int rr = r - 1; rr <= r + 1; ++rr) {
        if (rr < 0 || rr >= Map::rows)
            continue;

        for (int cc = c - 1; cc <= c + 1; ++cc) {
            if (cc < 0 || cc >= Map::cols)
                continue;

            if (islandMap[rr][cc] < 0) {
                islandMap[rr][cc] = islandId;
                floodFillRec(islandMap, islandId, rr, cc, recDepth + 1);
            }
        }
    }
}

int floodFill(Map& islandMap, int islandId, int r, int c) {
    if (islandMap[r][c] >= 0)
        return 0;

    islandMap[r][c] = islandId;
    floodFillRec(islandMap, islandId, r, c, 1);

    return 1;
}

int main() {
    Map islandMap;
    int currIsland = 1;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (int i = 0; i < NumIterations; ++i) {
        islandMap.set(TestPattern);

        currIsland = 1;
        for (int r = 0; r < Map::rows; ++r) {
            for (int c = 0; c < Map::cols; ++c) {
                currIsland += floodFill(islandMap, currIsland, r, c);
            }
        }
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "\nNaive recursion. Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms\n";

    std::cout << "Num islands: " << (currIsland - 1) << ", max recursion depth: " << g_maxRecDepth << "\n";
    if (islandMap.cols * islandMap.rows < 100)
        islandMap.print();

    printResult(islandMap);

    return 0;
}
