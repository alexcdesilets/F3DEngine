#include <fstream>

#include "globals.h"
#include "glCallBacks.h"
#include "IOUtils/saveAndLoad.h"

int main(int argc, char* argv[]) {
    // --- Game setup ---
    loadTextures("./gameDef/textures.json");
    lvl = loadLevelFromFile("./gameDef/savedLevel.json");
    lvl.update();

    // --- SDL + OpenGL Setup ---
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Ask SDL to give us an OpenGL 2.1 context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Window* window = SDL_CreateWindow("Test Project",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        windowWidth, windowHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        std::cerr << "SDL_GL_CreateContext Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_GL_SetSwapInterval(1); // Enable vsync

    // --- OpenGL state setup ---
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background to black

    // Set orthographic 2D projection (matches FreeGLUT)
    onReshape(windowWidth, windowHeight); // Sets viewport + matrices

    SDL_ShowCursor(SDL_DISABLE);
    SDL_WarpMouseInWindow(window, halfWindowWidth, halfWindowHeight);

    // --- Main loop ---
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;

                case SDL_KEYDOWN:
                    if (!event.key.repeat)
                        keyDown(event.key.keysym.sym, window);
                    break;

                case SDL_KEYUP:
                    keyUp(event.key.keysym.sym);
                    break;

                case SDL_MOUSEMOTION:
                    mouseMotion(event.motion.x, event.motion.y, window);
                    break;

                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        onReshape(event.window.data1, event.window.data2);
                    }
                    break;
            }
        }

        display(window); // Your draw logic (calls rayCast(), etc.)
    }

    // --- Cleanup ---
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
