#include <SDL2/SDL.h>
#include <stdio.h>
#include <assert.h>

struct Point {
    float x;
    float y;
};

int
binomial(int n, int k)
{
    // Overflows abound
    assert(n < 30);

    long p = 1;
    long q = 1;

    if (n - k < k)
        k = n - k;

    for (int i = 1; i <= k; ++i) {
        p *= (n + 1 - i);
        q *= i;
    }

    int result = p / q;
    return result;
}

Point
bezier(float t, Point *points, int num_points)
{
    Point result = {0.0f, 0.0f};
    //float t_acc = 1.0f;
    //float s_acc = powf(1.0f - t, num_points);;

    int n = num_points-1;
    for (int i = 0; i <= n; ++i) {
        int binom = binomial(n, i);
        float t_acc = pow(t, i);
        float s_acc = pow(1.0f - t, n-i);
        float x = binom * t_acc * s_acc * points[i].x;
        float y = binom * t_acc * s_acc * points[i].y;
        result.x += x;
        result.y += y;
        //t_acc *= t;
        //s_acc /= (1.0f - t);
    }

    return result;
}

void
draw_circle(uint8_t *pix, Point p, float rsq, int color)
{
    for (int y = 0; y < 480; y++)
        for (int x = 0; x < 640; x++) {
            float dx = p.x - x;
            float dy = p.y - y;
            float distsq = dx*dx + dy*dy;
            float diff = rsq - distsq;
            if (diff >= 0) {
                /* Blend if diff < 1 */
                uint8_t *bufpos = pix + 3*(640*y + x);
                *bufpos++ = (color & 0x0000ff);
                *bufpos++ = (color & 0x00ff00) >> 0x08;
                *bufpos++ = (color & 0xff0000) >> 0x10;
            }
        }
}

int
main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow(
            "Splines",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            640,
            480,
            0);

    SDL_Surface *canvas_surf = SDL_CreateRGBSurface(
            0,
            640,
            480,
            24,
            0x0000ff,
            0x00ff00,
            0xff0000,
            0);
    uint8_t *canvas = (uint8_t *) canvas_surf->pixels;
    memset(canvas, 0xff, 640*480*3);

    SDL_Surface *windowsurf = SDL_GetWindowSurface(window);
    SDL_BlitSurface(canvas_surf, NULL, windowsurf, NULL);

    bool quit = false;
    Point mouse_pos;
    Point selection[100];
    int select_count = 0;
    while (!quit) {
        bool new_mouse_pos = false;
        bool select = false;
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_MOUSEMOTION:
                {
                    int x = ev.motion.x;
                    int y = ev.motion.y;
                    if (x > 0 && x < 640 && y > 0 && y < 480) {
                        new_mouse_pos = true;
                        mouse_pos.x = x;
                        mouse_pos.y = y;
                    }
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                select = true;
                break;
            }
        }
        if (new_mouse_pos || select) {
            if (select)
                selection[select_count++] = mouse_pos;
            memset(canvas, 0xff, 640*480*3);
            for (int i = 0; i < select_count; ++i)
                draw_circle(canvas, selection[i], 32, 0x00ff00);
            if (select_count >= 2) {
                int samples = 10 * select_count;
                for (int i = 1; i < samples; ++i) {
                    float t = i / (float) samples;
                    Point p = bezier(t, selection, select_count);
                    draw_circle(canvas, p, 16, 0xff);
                }
            }
            draw_circle(canvas, mouse_pos, 32, 0);
            SDL_BlitSurface(canvas_surf, NULL, windowsurf, NULL);
        }
        SDL_UpdateWindowSurface(window);
    }
    puts(SDL_GetError());
}
