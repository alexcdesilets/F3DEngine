#pragma once

#include "globals.h"
#include "rayCast.h"
#include "keyHandler.h"

void onReshape(int width, int height) {
    int targetWidth = width;
    int targetHeight = (targetWidth * 9) / 16;

    if (targetHeight > height) {
        targetHeight = height;
        targetWidth = (targetHeight * 16) / 9;
    }

    targetWidth -= targetWidth % 16;
    targetHeight = (targetWidth / 16) * 9;

    windowWidth = targetWidth;
    windowHeight = targetHeight;
    halfWindowWidth = windowWidth / 2;
    halfWindowHeight = windowHeight / 2;

    numRays = windowWidth / 4;
    veritcalRays = (windowWidth / 16) * 9;

    int viewportX = (width - targetWidth) / 2;
    int viewportY = (height - targetHeight) / 2;

    glViewport(viewportX, viewportY, targetWidth, targetHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, targetWidth, targetHeight, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void display(SDL_Window* window) {
    glClear(GL_COLOR_BUFFER_BIT);

    if (currentGameState == GAMESTATES::TITLE_SCREEN) {
        // Title logic
    }
    else if (currentGameState == GAMESTATES::GAME_PLAY || currentGameState == GAMESTATES::MENU_OPEN || currentGameState == GAMESTATES::PAUSED) {
        if (currentGameState == GAMESTATES::GAME_PLAY) {
            gamePLayKeys();
            lvl.sectorList[5].orbit({360, 360}, 0.001 * pi);
            lvl.sectorList[5].rotate(0.002 * pi);
            lvl.update();
            pl.physicUpdate();
            for (Sector& sector : lvl.sectorList) {
                sector.lastFrame.reset();
            }
        }

        RayCast::drawBaseWorld();
        RayCast::rayCast();
    }

    SDL_GL_SwapWindow(window);  // replaces glutSwapBuffers
}


bool captureMouse = true, firstMouse = true;

void mouseMotion(int x, int y, SDL_Window* window) {
    if (currentGameState == GAMESTATES::GAME_PLAY || currentGameState == GAMESTATES::MENU_OPEN || currentGameState == GAMESTATES::PAUSED) {
        if (!captureMouse) {
            firstMouse = true;
            return;
        }

        int centerX = halfWindowWidth;
        int centerY = halfWindowHeight;

        if (firstMouse) {
            if (x != centerX || y != centerY) {
                SDL_WarpMouseInWindow(window, centerX, centerY);
            }
            firstMouse = false;
            return;
        }

        int deltaX = x - centerX;
        int deltaY = centerY - y;

        const double mouseSensitivity = 0.0015;

        if (deltaX != 0) pl.rotate(deltaX * mouseSensitivity);
        if (deltaY != 0) pl.rotatePitch(-deltaY * mouseSensitivity * 1.5);

        if (x != centerX || y != centerY) {
            SDL_WarpMouseInWindow(window, centerX, centerY);
        }
    }
}

void keyDown(SDL_Keycode key, SDL_Window* window) {
    switch (key) {
        case SDLK_i:
            keyFlags['i'] = !keyFlags['i'];
            if (currentGameState == GAMESTATES::GAME_PLAY || currentGameState == GAMESTATES::MENU_OPEN) {
                if (currentGameState == GAMESTATES::GAME_PLAY) {
                    captureMouse = false;
                    currentGameState = GAMESTATES::MENU_OPEN;
                    SDL_ShowCursor(SDL_ENABLE);
                } else {
                    captureMouse = true;
                    currentGameState = GAMESTATES::GAME_PLAY;
                    SDL_ShowCursor(SDL_DISABLE);
                    SDL_WarpMouseInWindow(window, halfWindowWidth, halfWindowHeight);
                }
            }
            break;

        case SDLK_w: keyFlags['w'] = true; break;
        case SDLK_s: keyFlags['s'] = true; break;
        case SDLK_a: keyFlags['a'] = true; break;
        case SDLK_d: keyFlags['d'] = true; break;
        case SDLK_SPACE: keyFlags[' '] = true; break;

        case SDLK_ESCAPE:
            SDL_Event quitEvent;
            quitEvent.type = SDL_QUIT;
            SDL_PushEvent(&quitEvent);
            break;
    }

    // Handle shift logic here (on press)
    if (SDL_GetModState() & KMOD_SHIFT) {
        movementRatio = 1.5;
    }
}

void keyUp(SDL_Keycode key) {
    switch (key) {
        case SDLK_w: keyFlags['w'] = false; break;
        case SDLK_s: keyFlags['s'] = false; break;
        case SDLK_a: keyFlags['a'] = false; break;
        case SDLK_d: keyFlags['d'] = false; break;
        case SDLK_SPACE: keyFlags[' '] = false; break;
    }

    // Re-evaluate shift status (on release)
    if (!(SDL_GetModState() & KMOD_SHIFT)) {
        movementRatio = 1.0;
    }
}
