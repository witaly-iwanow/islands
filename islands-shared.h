#include <iostream>
#include <array>
#include <map>
#include <set>

// fixed size "map" with "islands" surrounded by "water"
class Map {
public:
    using CellType = int32_t;

    static constexpr int rows = 302;
    static constexpr int cols = 202;

    static constexpr CellType Water = 0;
    static constexpr CellType Island = -1;

    enum class Pattern { OnlyWater, OnlyIsland, Cross, Checkerboard, None };

    void set(Pattern pattern) {
        if (pattern == Pattern::OnlyWater) {
            map.fill(Water);
        }
        else if (pattern == Pattern::OnlyIsland) {
            map.fill(Water);
            for (int r = 0; r < rows; ++r) {
                for (int c = 0; c < cols; ++c)
                    this->operator[](r)[c] = Island;
            }
        }
        else if (pattern == Pattern::Cross) {
            // cut it into 4 islands with row = 1 and col = 1 lines,
            // so that it's somewhat complicated, and we can be sure
            // the whole thing works as intended
            for (int r = 0; r < rowsPadded; ++r) {
                for (int c = 0; c < colsPadded; ++c)
                    map[r * colsPadded + c] = (r == 0 || r == 2 || r == (rowsPadded - 1) || c == 0 || c == 2 || c == (colsPadded - 1)) ? Water : Island;
            }
        }
        else if (pattern == Pattern::Checkerboard) {
            map.fill(Water);
            for (int r = 0; r < rows; ++r) {
                for (int c = 0; c < cols; ++c)
                    this->operator[](r)[c] = ((r ^ c) & 1) ? Water : Island;
            }
        }
    }

    CellType* operator[](int row) {
        return map.data() + 1 + (row + 1) * colsPadded;
    }
    const CellType* operator[](int row) const {
        return map.data() + 1 + (row + 1) * colsPadded;
    }

    void print() const {
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                auto val = this->operator[](r)[c];
                if (val < 0)
                    std::cout << "- ";
                else
                    std::cout << val << " ";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }

private:
    static constexpr int rowsPadded = 1 + rows + 1;
    static constexpr int colsPadded = 1 + cols + 1;

    std::array<CellType, rowsPadded * colsPadded> map;
};

// prints [number of islands]: [biggest island size] [second biggest island size] ... [smallest island size]
// for correctness verification purposes. Every island is assumed to be marked with a unique non-negative id
static void printResult(const Map& map) {
    std::map<int, int> islandSizes;
    for (int r = 0; r < Map::rows; ++r) {
        for (int c = 0; c < Map::cols; ++c) {
            auto islandId = map[r][c];
            if (islandId > 0)
                ++islandSizes[islandId];
        }
    }

    std::multiset<int, std::greater<int>> sortedIslandSizes;
    for (auto& [islandId, islandSize] : islandSizes)
        sortedIslandSizes.emplace(islandSize);

    std::cout << sortedIslandSizes.size() << ": ";
    for (auto i = sortedIslandSizes.cbegin(); i != sortedIslandSizes.cend(); ++i)
        std::cout << *i << " ";

    std::cout << "\n";
}

// performance test parameters
static constexpr int NumIterations = 1000;
static constexpr Map::Pattern TestPattern = Map::Pattern::Cross;
