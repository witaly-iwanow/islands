#include <queue>
#include <stack>
#include <memory>
#include <chrono>

#include "islands-shared.h"

struct Cell {int r, c;};

// queue or stack? Stack appears to be 45% slower, with Clang/M1 at least
#if 1
using Neighbors = std::queue<Cell>;
#define FIRST front
#else
using Neighbors = std::stack<Cell>;
#define FIRST top
#endif

#define GN(_r, _c) \
    if (islandMap[_r][_c] < 0) { \
        islandMap[_r][_c] = islandId; \
        neibs.push({_r, _c}); \
    }

void getNeighbors(Map& islandMap, Map::CellType islandId, Cell c, Neighbors& neibs) {
    GN(c.r - 1, c.c - 1)
    GN(c.r - 1, c.c)
    GN(c.r - 1, c.c + 1)
    GN(c.r, c.c - 1)
    GN(c.r, c.c + 1)
    GN(c.r + 1, c.c - 1)
    GN(c.r + 1, c.c)
    GN(c.r + 1, c.c + 1)
}

static int g_maxNumNeibs = 0;
int floodFill(Map& islandMap, Map::CellType islandId, int r, int c) {
    if (islandMap[r][c] >= 0)
        return 0;

    islandMap[r][c] = islandId;
    Neighbors neibs;
    neibs.push({r, c});

    while (!neibs.empty()) {
        //std::cout << "Num neighbors: " << neibs.size() << std::endl;
        //islandMap.print();

        if (neibs.size() > g_maxNumNeibs)
            g_maxNumNeibs = neibs.size();

        auto n = neibs.FIRST();
        neibs.pop();
        getNeighbors(islandMap, islandId, n, neibs);
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
            for (int c = 0; c < Map::cols; ++c)
                currIsland += floodFill(*islandMap, currIsland, r, c);
        }
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "\nNeighbor queue. Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms\n";

    std::cout << "Num islands: " << (currIsland - 1) << ", max num neighbors: " << g_maxNumNeibs <<  "\n";
    if (islandMap->cols * islandMap->rows < 100)
        islandMap->print();

    printResult(*islandMap);

    return 0;
}
