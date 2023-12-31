#include <queue>
#include <memory>
#include <chrono>

#include "islands-shared.h"

struct Cell {int r, c;};

constexpr int maxRecDepth = 1000;

#define GN(_r, _c) \
    if (islandMap[_r][_c] < 0) { \
        islandMap[_r][_c] = islandId; \
        if (recDepth < maxRecDepth) \
            getNeighbors(islandMap, islandId, _r, _c, recDepth + 1, neibs); \
        else \
            neibs.push({_r, _c}); \
    }

void getNeighbors(Map& islandMap, int islandId, int r, int c, int recDepth, std::queue<Cell>& neibs) {
    GN(r - 1, c - 1)
    GN(r - 1, c)
    GN(r - 1, c + 1)
    GN(r, c - 1)
    GN(r, c + 1)
    GN(r + 1, c - 1)
    GN(r + 1, c)
    GN(r + 1, c + 1)
}

static int g_maxNumNeibs = 0;
int floodFill(Map& islandMap, Map::CellType islandId, int r, int c) {
    if (islandMap[r][c] >= 0)
        return 0;

    islandMap[r][c] = islandId;
    std::queue<Cell> neibs;
    neibs.push({r, c});

    while (!neibs.empty()) {
        //std::cout << "Num neighbors: " << neibs.size() << std::endl;
        //islandMap.print();

        if (neibs.size() > g_maxNumNeibs)
            g_maxNumNeibs = neibs.size();

        auto p = neibs.front();
        neibs.pop();
        getNeighbors(islandMap, islandId, p.r, p.c, 1, neibs);
    }

    return 1;
}

int main() {
    std::unique_ptr<Map> islandMap(new Map);
    Map::CellType currIsland = 1;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (int i = 0; i < NumIterations; ++i) {
        islandMap->set(TestPattern);

        currIsland = 1;
        for (int r = 0; r < Map::rows; ++r) {
            for (int c = 0; c < Map::cols; ++c) {
                currIsland += floodFill(*islandMap, currIsland, r, c);
            }
        }
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "\nRecursion (depth=1000) + neighbor queue. Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms\n";

    std::cout << "Num islands: " << (currIsland - 1) << ", max num neighbors: " << g_maxNumNeibs <<  "\n";
    if (islandMap->cols * islandMap->rows < 100)
        islandMap->print();

    printResult(*islandMap);

    return 0;
}
