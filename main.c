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

static void bterp(const Map map, const Mesh m)
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

static void noise(const Map map, const int square)
{
    if(square == 2)
        return;
    const int m = (square - 1) / 2;
    /* Points of interest */
    for(int i = m; i < map.size; i += 2 * m)
    for(int j = m; j < map.size; j += 2 * m)
        map.grid[i][j] += rand() % (2 * square + 1) - square;
    /* Interpolation */
    for(int i = 0; i < map.size - 1; i += m)
    for(int j = 0; j < map.size - 1; j += m)
    {
        const Mesh mesh = { { i, j }, { i + m, j + m } };
        bterp(map, mesh);
    }
    /* Down the rabbit hole we go */
    noise(map, m + 1);
}

static int max(const Map map)
{
    int max = INT_MIN;
    for(int i = 0; i < map.size; i++)
    for(int j = 0; j < map.size; j++)
        if(map.grid[i][j] > max)
            max = map.grid[i][j];
    return max;
}

static int min(const Map map)
{
    int min = INT_MAX;
    for(int i = 0; i < map.size; i++)
    for(int j = 0; j < map.size; j++)
        if(map.grid[i][j] < min)
            min = map.grid[i][j];
    return min;
}

static void add(const Map map, const int val)
{
    for(int i = 0; i < map.size; i++)
    for(int j = 0; j < map.size; j++)
        map.grid[i][j] += val;
}

static void normalize(const Map map)
{
    const int lowest = min(map);
    add(map, abs(lowest));
    const int highest = max(map);
    for(int i = 0; i < map.size; i++)
    for(int j = 0; j < map.size; j++)
        map.grid[i][j] = 0xFF * map.grid[i][j] / (float) highest;
}

static void draw(const Map map, SDL_Renderer* const renderer)
{
    normalize(map);
    for(int i = 0; i < map.size; i++)
    for(int j = 0; j < map.size; j++)
    {
        const int a = 0x2A; /* Sea floor */
        const int b = 0x5F; /* Sea level */
        const int p = map.grid[i][j];
        p < a ?
            SDL_SetRenderDrawColor(renderer, 0, 0, a, 0): /* Sea floor */
        p < b ?
            SDL_SetRenderDrawColor(renderer, 0, 0, p, 0): /* Sea */
            SDL_SetRenderDrawColor(renderer, p, p, p, 0); /* Ice and snow */
        SDL_RenderDrawPoint(renderer, i, j);
    }
    SDL_RenderPresent(renderer);
    SDL_Delay(5e3);
}

static Map build(const int size)
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

static void clean(const Map map)
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
