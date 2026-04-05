#pragma once

// --- STL Includes ---

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <cmath>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <iostream>
#include <algorithm>
#include <format>
#include <thread>
#include <chrono>
#include <omp.h>
#include <mutex>
#include <fstream>
#include <numbers>

// --- VecSys Includes ---

#include "vecSys/base.h"
#include "vecSys/vec2.h"
#include "vecSys/shape2.h"
#include "vecSys/line2.h"
#include "vecSys/vecRGBA.h"
#include "vecSys/frameBuffer.h"

using namespace vecSys;

// --- Generic GLobals ---

using std::vector;
using std::pair;
using std::map;
using std::unordered_map;
using std::string;

int xyIndextoscalarIndex(const int& xval, const int& yval, int maxX = 256) { return (yval * maxX) + xval; }

int windowWidth = 1024, windowHeight = 576, halfWindowWidth = windowWidth / 2, halfWindowHeight = windowHeight / 2;
int numRays = 256, veritcalRays = (numRays*9)/16;
bool keyFlags[256] = {false};
double fovH = 75*pi/180.0;
int previousFrameTime = 0;
double movementRatio = 1.0;

enum class GAMESTATES {TITLE_SCREEN, GAME_PLAY, GAME_OVER, MENU_OPEN, PAUSED, LEVEL_EDITOR, OPTIONS_MENU};
GAMESTATES currentGameState = GAMESTATES::GAME_PLAY;

// --- Game Engine Includes ---

#include "Player.h"
#include "texture.h"
#include "sector.h"
#include "level.h"

// --- Complex Globals ---

Level lvl;
Player pl = {{450, 400}, 0.25 * pi, 0.0, 2.0, 400, 0, false};
vec2 mouseMovement;

vector<std::string> textureFiles;
unordered_map<std::string, Texture> textures;
