#include <queue>
#include <memory>
#include <chrono>

#include "islands-shared.h"

struct Cell {
    int r;
    int c;
};

void getNeighbors(Map& islandMap, int islandId, int r, int c, std::queue<Cell>& neibs) {
    for (int rr = r - 1; rr <= r + 1; ++rr) {
        if (rr < 0 || rr >= Map::rows)
            continue;

        for (int cc = c - 1; cc <= c + 1; ++cc) {
            if (cc < 0 || cc >= Map::cols)
                continue;

            if (islandMap[rr][cc] < 0) {
                islandMap[rr][cc] = islandId;
                neibs.push({ rr, cc });
            }
        }
    }
}

static int g_maxNumNeibs = 0;
int floodFill(Map& islandMap, int islandId, int r, int c) {
    if (islandMap[r][c] >= 0)
        return 0;

    islandMap[r][c] = islandId;
    std::queue<Cell> neibs;
    neibs.push({ r, c });

    while (!neibs.empty()) {
        //std::cout << "Num neighbors: " << neibs.size() << std::endl;
        //islandMap.print();

        if (neibs.size() > g_maxNumNeibs)
            g_maxNumNeibs = neibs.size();

        auto p = neibs.front();
        neibs.pop();
        getNeighbors(islandMap, islandId, p.r, p.c, neibs);
    }

    return 1;
}

int main() {
    std::unique_ptr<Map> islandMap(new Map);
    int currIsland = 1;

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
    std::cout << "\nNeighbor queue. Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms\n";

    std::cout << "Num islands: " << (currIsland - 1) << ", max num neighbors: " << g_maxNumNeibs <<  "\n";
    if (islandMap->cols * islandMap->rows < 100)
        islandMap->print();

    printResult(*islandMap);

    return 0;
}
