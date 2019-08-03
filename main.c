#include <SDL2/SDL.h>
#include <time.h>
#include <limits.h>

typedef struct
{
    int* grid;
    int size;
}
Map;

typedef struct
{
    int x;
    int y;
}
Cart;

typedef struct
{
    Cart a;
    Cart b;
}
Mesh;

static void Interpolate(Map* map, const Mesh m)
{
    for(int x = m.a.x; x < m.b.x; x++)
    for(int y = m.a.y; y < m.b.y; y++)
        map->grid[y + map->size * x] = (
            map->grid[m.a.y + map->size * m.a.x] * (m.b.x - x) * (m.b.y - y) +
            map->grid[m.a.y + map->size * m.b.x] * (x - m.a.x) * (m.b.y - y) +
            map->grid[m.b.y + map->size * m.a.x] * (m.b.x - x) * (y - m.a.y) +
            map->grid[m.b.y + map->size * m.b.x] * (x - m.a.x) * (y - m.a.y)
        ) / ((m.b.x - m.a.x) * (m.b.y - m.a.y));
}

static void Noise(Map* map, const int square)
{
    if(square == 2)
        return;
    const int m = (square - 1) / 2;
    for(int i = m; i < map->size; i += 2 * m)
    for(int j = m; j < map->size; j += 2 * m)
        map->grid[j + map->size * i] += rand() % (2 * square + 1) - square;
    for(int i = 0; i < map->size - 1; i += m)
    for(int j = 0; j < map->size - 1; j += m)
    {
        const Mesh mesh = { { i, j }, { i + m, j + m } };
        Interpolate(map, mesh);
    }
    Noise(map, m + 1);
}

static int Max(Map* map)
{
    int max = INT_MIN;
    for(int i = 0; i < map->size; i++)
    for(int j = 0; j < map->size; j++)
        if(map->grid[j + map->size * i] > max)
            max = map->grid[j + map->size * i];
    return max;
}

static int Min(Map* map)
{
    int min = INT_MAX;
    for(int i = 0; i < map->size; i++)
    for(int j = 0; j < map->size; j++)
        if(map->grid[j + map->size * i] < min)
            min = map->grid[j + map->size * i];
    return min;
}

static void Add(Map* map, const int val)
{
    for(int i = 0; i < map->size; i++)
    for(int j = 0; j < map->size; j++)
        map->grid[j + map->size * i] += val;
}

static void Normalize(Map* map)
{
    const int lowest = Min(map);
    Add(map, abs(lowest));
    const int highest = Max(map);
    for(int i = 0; i < map->size; i++)
    for(int j = 0; j < map->size; j++)
        map->grid[j + map->size * i] = 0xFF * map->grid[j + map->size * i] / (float) highest;
}

static void Draw(Map* map, SDL_Renderer* const renderer)
{
    Normalize(map);
    for(int i = 0; i < map->size; i++)
    for(int j = 0; j < map->size; j++)
    {
        const int a = 0x2A; // Sea floor.
        const int b = 0x5F; // Sea level.
        const int p = map->grid[j + map->size * i];
        p < a ?
            SDL_SetRenderDrawColor(renderer, 0, 0, a, 0): // Sea floor.
        p < b ?
            SDL_SetRenderDrawColor(renderer, 0, 0, p, 0): // Sea.
            SDL_SetRenderDrawColor(renderer, p, p, p, 0); // Ice and snow.
        SDL_RenderDrawPoint(renderer, i, j);
    }
    SDL_RenderPresent(renderer);
    SDL_Delay(1000);
}

Map MP_Build(const int power)
{
    const int size = pow(2, power) + 1;
    Map map = { 0 };
    map.grid = calloc(size * size, sizeof(*map.grid));
    map.size = size;
    Noise(&map, map.size);
    return map;
}

void MP_Clean(Map* map)
{
    free(map->grid);
}

int main()
{
    srand(time(0));
    const int power = 9;
    Map map = MP_Build(power);
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_CreateWindowAndRenderer(map.size, map.size, 0, &window, &renderer);
    Draw(&map, renderer);
    MP_Clean(&map);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return 0;
}
