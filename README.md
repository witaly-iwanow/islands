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
        auto n = neibs.front();
        neibs.pop();
        getNeighbors(islandMap, islandId, n, neibs);
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

## Iteration 5: getting help from the compiler
Let's add `-O3` to the compiler options and see what happens, now the queue method is much faster:

| | default | -O3 |
| --- | --- | --- |
| Recursion | 3.7 | 1 |
| Queue | 5.8 | 0.64 |
| Recursion + queue | 4.1 | 0.95 |

Another thing to try is tighter packing with `int8_t` as the cell type, but that doesn't seem to affect performance on my hardware even with very large maps. It might be useful in terms of memory savings, if marking islands with unique ids is not required (or the number of islands is guaranteed to be within 127).

## Iteration 6: boundary check elimination and loop unroll
The code checking the cell's neighbors is, in fact, a little more complicated than shown before - it needs extra checks to make sure we're within the map's boundaries:
```cpp
for (int rr = r - 1; rr <= r + 1; ++rr) {
    if (rr < 0 || rr >= Map::rows)
        continue;

    for (int cc = c - 1; cc <= c + 1; ++cc) {
        if (cc < 0 || cc >= Map::cols)
            continue;

        // do things
    }
}
```
These checks might cause performance issues on CPUs with high "branch penalty". Let's get rid of these checks by means of adding 1-cell-wide water padding, plus throw in some manual loop unrolling.

By water padding I mean internally storing this:
```
1 1 1
1 1 1
1 1 1
```
as
```
0 0 0 0 0
0 1 1 1 0
0 1 1 1 0
0 1 1 1 0
0 0 0 0 0
```
while on the outside the matrix is still presented as 3x3. This allows us not to worry about the code trying to access matrix[-1][-1] or such - it won't cause any problems as long as it's initialized with 0 ("water").

And onto the manual loop unroll:
```cpp
#define GN(_r, _c) \
    if (islandMap[_r][_c] < 0) { \
        islandMap[_r][_c] = islandId; \
        neibs.push({_r, _c}); \
    }

void getNeighbors(Map& islandMap, int islandId, int r, int c, std::queue<Cell>& neibs) {
    GN(r - 1, c - 1)
    GN(r - 1, c)
    GN(r - 1, c + 1)
    GN(r, c - 1)
    GN(r, c + 1)
    GN(r + 1, c - 1)
    GN(r + 1, c)
    GN(r + 1, c + 1)
}
```

All the methods benefited from the changes, although available recursion depth decreased somewhat:
| | default | -O3 | padding & unroll |
| --- | --- | --- | --- |
| Recursion | 3.7 | 1 | 0.54* |
| Queue | 5.8 | 0.64 | 0.47 |
| Recursion + queue | 4.1 | 0.95 | 0.66 |
\* the recursive method started crashing on the standard 302x202 map used for all the previous tests, so I had to reduce the map size and interpolate the result (it's approx 15% slower than the queue method)

## Iteration 7: down the rabbit hole
Let's see how things change if we handle neighbors in a different order - namely switching from a queue/FIFO to stack/LIFO:
```cpp
#if 1
using Neighbors = std::queue<Cell>;
#define FIRST front
#else
using Neighbors = std::stack<Cell>;
#define FIRST top
#endif

auto n = neibs.FIRST();
neibs.pop();
getNeighbors(islandMap, islandId, n, neibs);

```
In order to emphasize the effect of localized reads/writes, I've switched to a much bigger map (30k x 20k) for this test (4 consecutive runs, the fastest time):
| Container | time, s |
| --- | --- |
| Queue | 6.4 |
| Stack | 9.3 |

Whoa, using a stack slows it down 45%! Let's see what's happening here. If we log the order in which the cells are handled, we'll get this for a queue:
```
 1  2  5 10 17
 3  4  6 11 18
 7  8  9 12 19
13 14 15 16 20
21 22 23 24 25
26 27 28 29 30
31 32 33 34 35
```
Not too bad - it's relatively well localized, but you can do even better in terms of producing less cache misses (see Iteration 8). Now here's a stack:
```
 1  2  5 34 35
 3  4  6 10 33
 7  8  9 11 15
32 12 13 14 16
30 31 17 18 19
28 26 24 20 21
29 27 25 22 23
```
It's definitely much more chaotic, no wonder we are seeing a major performance drop.

## Iteration 8: progressive scanning (WIP)
The main idea is to scan the map row-by-row and keep track of areas connected to each other between rows. The CPU cache will be as happy as it can ever be.

An example to illustrate the concept. Here's the map ('-' represents unvisited "land", stored as -1):
```
- - 0 0 0 0 - -
0 - - 0 0 - - 0
0 0 0 - - 0 0 0
```
Fill consecutive ranges of "land" with unique island ids (those are not final as some of the areas will merge eventually). Row #1:
```
1 1 0 0 0 0 2 2
0 - - 0 0 - - 0
0 0 0 - - 0 0 0
```
Store a set of two island id sets: {{1}, {2}}.

Row #2:
```
1 1 0 0 0 0 2 2
0 3 3 0 0 4 4 0
0 0 0 - - 0 0 0
```
As `3` is connected to `1`, we put it into `1`'s set (same with `2` and `4`): {{1,3}, {2,4}}.

Row #3:
```
1 1 0 0 0 0 2 2
0 3 3 0 0 4 4 0
0 0 0 5 5 0 0 0
```
As `5` is connected to both `1+3` and `2+4` sets, they get merged into one set {1,2,3,4,5} - i.e. there's only one island.

#### Progressive scan, Step 1
Implement row-by-row filling, that alone will give us a decent performance estimate:
```cpp
void fillRow(Map& islandMap, int& islandId, int r) {
    int c = 0;
    auto row = islandMap[r];
    while (c < islandMap.cols) {
        if (row[c] < 0) {
            if (!row[c - 1])
                ++islandId; // water in prev cell = new island

            row[c] = islandId;
        }
        ++c;
    }
}

...

Map islandMap;
islandMap.set(Map::Pattern::Cross);

Map::CellType currIsland = 0;
for (int r = 0; r < Map::rows; ++r)
    fillRow(islandMap, currIsland, r);
```
And the result is
```
- 0 - - - - - -      1 0 2 2 2 2 2 2
0 0 0 0 0 0 0 0      0 0 0 0 0 0 0 0
- 0 - - - - - -  =>  3 0 4 4 4 4 4 4
- 0 - - - - - -      5 0 6 6 6 6 6 6
- 0 - - - - - -      7 0 8 8 8 8 8 8
```
which looks correct and takes 2.7s on a 30k x 20k map - 2.4x faster than the queue algorithm. We still need to do row merging and replace ids in the map, but there's a healthy performance margin, so it should work...

#### Progressive scan, Step 2
Now let's see how we can merge "row" islands we got in Step 1. A quick test to check if checking cells from the previous row will slow us down significantly (the code itself doesn't produce anything usable at this point):
```cpp
void fillRow(Map& islandMap, int& islandId, int r) {
    int c = 0;
    auto row = islandMap[r];
    const auto prevRow = islandMap[r - 1];
    while (c < islandMap.cols) {
        if (row[c] < 0) {
            if (!row[c - 1])
                ++islandId;

            if (prevRow[c - 1] > 0)
                row[c] = prevRow[c - 1];
            else if (prevRow[c] > 0)
                row[c] = prevRow[c];
            else if (prevRow[c + 1] > 0)
                row[c] = prevRow[c + 1];
            else
                row[c] = islandId;
        }
        ++c;
    }
}
```
Surprisingly this runs even faster than `fillRow` from Step 1, a lot faster in fact: 1.4s (vs 2.7s). It doesn't make much sense - anyone having the luxury of time to kill is very much welcome to go to `godbolt.org`, stare at the assembler code and solve the mystery.
Another observation is that `const auto prevRow` is slightly, but consistently faster than `auto prevRow` (by about 10%) - one more great reason to use `const` as much as possible.
