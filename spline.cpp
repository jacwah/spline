#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <stdio.h>
#include <assert.h>

struct Point {
    float x;
    float y;
};

Point
bezier(float t, Point *points, int num_points)
{
    Point result = {0.0f, 0.0f};
    int n = num_points-1;
    int binom = 1;

    for (int i = 0; i <= n; ++i) {
        float s = powf(t, i) * powf(1.0f - t, n-i);
        float x = binom * s * points[i].x;
        float y = binom * s * points[i].y;
        result.x += x;
        result.y += y;
        binom *= n-i;
        assert(binom % (i+1) == 0);
        binom /= i+1;
    }

    return result;
}

#define ERRGL() errgl(__LINE__)

int
errgl(int line)
{
    GLenum err = glGetError();
    if (!err)
        return 0;

    const char *str = "unknown error";
    switch (err) {
        case GL_INVALID_ENUM:                   str = "invalid enum"; break;
        case GL_INVALID_VALUE:                  str = "invalid value"; break;
        case GL_INVALID_OPERATION:              str = "invalid operation"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:  str = "invalid framebuffer operation"; break;
        case GL_OUT_OF_MEMORY:                  str = "out of memory"; break;
        case GL_STACK_UNDERFLOW:                str = "stack underflow"; break;
        case GL_STACK_OVERFLOW:                 str = "stack overflow"; break;
    }

    printf("GL error %d (line %d): %s\n", err, line, str);
    return err;
}

GLuint compile_shader(GLenum type, const char *filename)
{
    GLuint name = glCreateShader(type);
    char text[4096];
    char *textptr = text;
    FILE *file;

    file = fopen(filename, "r");
    int len = fread(text, 1, sizeof(text), file);
    text[len] = 0;
    glShaderSource(name, 1, &textptr, NULL);
    glCompileShader(name);
    GLint status;
    glGetShaderiv(name, GL_COMPILE_STATUS, &status);
    printf("Compile %s: %s\n", filename, status ? "success" : "error");
    GLint infoLen;
    glGetShaderiv(name, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen) {
        char *info = (char *) malloc(infoLen);
        glGetShaderInfoLog(name, infoLen, NULL, info);
        puts(info);
    }
    if (status)
        return name;
    else
        return 0; // Leak shader
}

int
main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

    int width = 640;
    int height = 480;

    SDL_Window *window = SDL_CreateWindow(
            "Splines",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            width,
            height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    SDL_GLContext glCtx = SDL_GL_CreateContext(window);
    glewInit();

    glEnable(GL_MULTISAMPLE);

    GLfloat widthRange[2] = {};
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, widthRange);
    printf("Line width range: %f to %f\n", widthRange[0], widthRange[1]);

    int msbuffers, mssamples;
    SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &msbuffers);
    SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &mssamples);
    printf("ms buffers: %d\n", msbuffers);
    printf("ms samples: %d\n", mssamples);

    printf("Sync: %d\n", SDL_GL_GetSwapInterval());

    GLuint vertexShader = compile_shader(GL_VERTEX_SHADER, "shader.vert");
    GLuint fragmentShader = compile_shader(GL_FRAGMENT_SHADER, "shader.frag");
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);

    GLuint buffers[2];
    glGenBuffers(2, buffers);
    GLuint lineBuf = buffers[0];
    GLuint pointBuf = buffers[1];

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    ERRGL();

    bool quit = false;
    Point mouse_pos;
    Point selection[100];
    Point *line_verts = NULL;
    int line_vert_count = 0;
    int select_count = 0;

    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
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
                    if (x > 0 && x < width && y > 0 && y < height) {
                        new_mouse_pos = true;
                        mouse_pos.x = 2.0f * x / (float) width - 1.0;
                        mouse_pos.y = 2.0f * y / (float) -height + 1.0;
                    }
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                select = true;
                break;
            case SDL_KEYDOWN:
                switch (ev.key.keysym.sym) {
                case SDLK_c:
                    select_count = 0;
                    line_vert_count = 0;
                    break;
                }
                break;
            case SDL_WINDOWEVENT:
                switch (ev.window.event) {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    width = ev.window.data1;
                    height = ev.window.data2;
                    printf("Window size change: %d, %d\n", width, height);
                    glViewport(0, 0, width, height);
                    break;
                }
                break;
            }
        }
        if (select) {
            selection[select_count++] = mouse_pos;
            glBindBuffer(GL_ARRAY_BUFFER, pointBuf);
            glBufferData(
                    GL_ARRAY_BUFFER,
                    sizeof(*selection)*select_count,
                    selection,
                    GL_DYNAMIC_DRAW);
            ERRGL();
            if (select_count >= 2) {
                line_vert_count = 10 * select_count;
                printf("line_vert_count %d\n", line_vert_count);
                line_verts = (Point *) realloc(line_verts, sizeof(*line_verts)*line_vert_count);
                for (int i = 0; i < line_vert_count; ++i) {
                    float t = (float) i / (line_vert_count - 1.0f);
                    line_verts[i] = bezier(t, selection, select_count);
                }
                glBindBuffer(GL_ARRAY_BUFFER, lineBuf);
                glBufferData(
                        GL_ARRAY_BUFFER,
                        sizeof(*line_verts)*line_vert_count,
                        line_verts,
                        GL_DYNAMIC_DRAW);
                ERRGL();
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        ERRGL();

        glBindBuffer(GL_ARRAY_BUFFER, pointBuf);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        ERRGL();
        glDrawArrays(GL_POINTS, 0, select_count);
        ERRGL();

        glBindBuffer(GL_ARRAY_BUFFER, lineBuf);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glDrawArrays(GL_LINE_STRIP, 0, line_vert_count);
        //glDrawArrays(GL_POINTS, 0, line_vert_count);

        ERRGL();
        SDL_GL_SwapWindow(window);
    }
    puts(SDL_GetError());
}
