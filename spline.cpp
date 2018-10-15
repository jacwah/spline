#include <SDL2/SDL.h>
#include <stdio.h>

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

    SDL_Surface *canvas = SDL_CreateRGBSurface(
            0,
            640,
            480,
            24,
            0x0000ff,
            0x00ff00,
            0xff0000,
            0);
    uint8_t *pix = (uint8_t *) canvas->pixels;
    for (int y = 0; y < 480; y++)
        for (int x = 0; x < 640; x++) {
            int r = 0;
            int g = 0;
            int b = 0;
            switch ((x/8 + y/8) % 3) {
                case 0:
                    r = 0xff;
                    break;
                case 1:
                    g = 0xff;
                    break;
                case 2:
                    b = 0xff;
                    break;
            }
            *pix++ = r;
            *pix++ = g;
            *pix++ = b;
        }
    SDL_Surface *windowsurf = SDL_GetWindowSurface(window);
    SDL_BlitSurface(canvas, NULL, windowsurf, NULL);

    bool quit = false;
    while (!quit) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
            }
        }
        SDL_UpdateWindowSurface(window);
    }
    puts(SDL_GetError());
}
