#include <SDL2/SDL.h>
#include <stdio.h>

void
draw_circle(uint8_t *pix, int x, int y, int rsq, int color)
{
    for (int py = 0; py < 480; py++)
        for (int px = 0; px < 640; px++) {
            int dx = px - x;
            int dy = py - y;
            if (dx*dx + dy*dy <= rsq) {
                uint8_t *bufpos = pix + 3*(640*py + px);
                *bufpos++ = color & 0x0000ff;
                *bufpos++ = color & 0x00ff00;
                *bufpos++ = color & 0xff0000;
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
    int mousex, mousey;
    while (!quit) {
        bool have_mouse_pos = false;
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
                        have_mouse_pos = true;
                        mousex = x;
                        mousey = y;
                    }
                }
                break;
            }
        }
        if (have_mouse_pos) {
            memset(canvas, 0xff, 640*480*3);
            draw_circle(canvas, mousex, mousey, 32, 0);
            SDL_BlitSurface(canvas_surf, NULL, windowsurf, NULL);
        }
        SDL_UpdateWindowSurface(window);
    }
    puts(SDL_GetError());
}
