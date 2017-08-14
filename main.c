#include <SDL2/SDL.h>
#include <time.h>
#include <limits.h>

typedef struct
{
    int** grid;
    int size;
}
Map;

typedef struct
{
    int x;
    int y;
}
Point;

typedef struct
{
    Point a;
    Point b;
}
Mesh;

void bterp(const Map map, const Mesh m)
{
    for(int x = m.a.x; x < m.b.x; x++)
    for(int y = m.a.y; y < m.b.y; y++)
    map.grid[x][y] = (
        map.grid[m.a.x][m.a.y] * (m.b.x - x) * (m.b.y - y) +
        map.grid[m.b.x][m.a.y] * (x - m.a.x) * (m.b.y - y) +
        map.grid[m.a.x][m.b.y] * (m.b.x - x) * (y - m.a.y) +
        map.grid[m.b.x][m.b.y] * (x - m.a.x) * (y - m.a.y)
    ) / ((m.b.x - m.a.x) * (m.b.y - m.a.y));
}

void noise(const Map map, const int square)
{
    if(square == 2)
        return;
    const int m = (square - 1) / 2;
    // Points of interest
    for(int i = m; i < map.size; i += 2 * m)
    for(int j = m; j < map.size; j += 2 * m)
        map.grid[i][j] += rand() % (2 * square + 1) - square;
    // Interpolation
    for(int i = 0; i < map.size - 1; i += m)
    for(int j = 0; j < map.size - 1; j += m)
    {
        const Mesh mesh = { { i, j }, { i + m, j + m } };
        bterp(map, mesh);
    }
    // Down the rabbit hole we go
    noise(map, m + 1);
}

int max(const Map map)
{
    int max = INT_MIN;
    for(int i = 0; i < map.size; i++)
    for(int j = 0; j < map.size; j++)
        if(map.grid[i][j] > max)
            max = map.grid[i][j];
    return max;
}

void draw(const Map map, SDL_Renderer* const renderer)
{
    const int highest = max(map);
    struct { int water, shore, adj, scale; } h = { 0x00, 0x04, 0x0F, 0x15 };
    for(int i = 0; i < map.size; i++)
    for(int j = 0; j < map.size; j++)
    {
        const int grey =
            map.grid[i][j] < h.water ? (h.water)
          : map.grid[i][j] < h.shore ? (h.shore + h.adj)
          : map.grid[i][j] * h.scale / (double) highest + h.adj;
        SDL_SetRenderDrawColor(renderer, grey, grey, grey, grey);
        SDL_RenderDrawPoint(renderer, i, j);
    }
    SDL_RenderPresent(renderer);
    SDL_Delay(1e4);
}

Map build(const int size)
{
    #define toss(type, size) ((type*) (calloc(size, sizeof(type))))
    Map map;
    map.grid = toss(int*, size);
    for(int i = 0; i < size; i++)
        map.grid[i] = toss(int, size);
    map.size = size;
    #undef toss
    return map;
}

void clean(const Map map)
{
    for(int i = 0; i < map.size; i++)
        free(map.grid[i]);
    free(map.grid);
}

int main()
{
    srand(time(0));
    const int res = pow(2, 9) + 1;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_CreateWindowAndRenderer(res, res, 0, &window, &renderer);
    const Map map = build(res);
    noise(map, map.size);
    draw(map, renderer);
    clean(map);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return 0;
}
