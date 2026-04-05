#pragma once

#include "globals.h"

namespace lvlEditor {
    vec2 currentDisplacement = {0,0};
    Level workingLevel = lvl;
    vector<vecRGBA> outlineColors;

    void drawCurrentLevel() {
        while (outlineColors.size() < lvl.sectorList.size() + lvl.billboardList.size()) {
            float r = (((float)std::rand() / RAND_MAX) * 0.8f) + 0.2f;
            float g = (((float)std::rand() / RAND_MAX) * 0.8f) + 0.2f;
            float b = (((float)std::rand() / RAND_MAX) * 0.8f) + 0.2f;
            outlineColors.push_back({r, g, b, 1.0});
        }

        for (int i = 0; i < lvl.sectorList.size(); ++i) {
            vecRGBA& color = outlineColors[i];
            vector<line2> outline = lvl.sectorList[i].outline.toLines();
            glColor3f(color.r, color.g, color.b);
            for (line2& line: outline) {
                line.translate(currentDisplacement);

                glBegin(GL_LINE);
                glVertex2d(line.pnt1.x, line.pnt1.y);
                glVertex2d(line.pnt2.x, line.pnt2.y);
                glEnd();

                
            }
        }
    }
};

