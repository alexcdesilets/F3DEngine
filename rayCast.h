#include "globals.h"

namespace RayCast {
    vector<line2> viewRays(Player& plr, int numRays, double fovRadians, double viewDistance);
    
    double halfFovH, minRay, maxRay, rayStep, rayDistance, viewDistance, lineWidth, verticalShift, horizonCenterY, upperViewLimit, lowerViewLimit;
    vector<line2> rays = viewRays(pl, numRays, fovH, rayDistance);

    struct QuadData {
        float x1, y1, x2, y2;
        vecRGBA color;
    };

    vector<line2> viewRays(Player& plr, int numRays = 256, double fovRadians = pi * 0.5f, double viewDistance = 1000.0f) {
        pair<double, double> minMaxyaw = plr.viewAngles(fovRadians);
        double yawPerRay = fovRadians / numRays;
        vector<line2> rays;
        for (int i = 0; i < numRays; ++i) {
            double rayyaw = minMaxyaw.first + (i * yawPerRay);
            vec2 rayDeff = vec2::fromAngle(rayyaw) * viewDistance;
            line2 ray = {plr.position, plr.position + rayDeff};
            rays.push_back(ray);
        }
        return rays;
    }
    
    void drawBaseWorld() {
        minRay = pl.yaw - (0.5 * fovH);
        rayStep = fovH / numRays;
        viewDistance = 1000;
        lineWidth = static_cast<double>(windowWidth) / numRays;
    
        rays = viewRays(pl, numRays, fovH, viewDistance);
    
        verticalShift = pl.pitch * (windowHeight / (pi * 0.25));
        horizonCenterY = (0.5 * windowHeight) - verticalShift + pl.verticalOffset;
    
        vector<QuadData> imageDefinition;
        imageDefinition.reserve(numRays * veritcalRays * 1.25);
        std::mutex imageLock;

        const unsigned int threadCount = std::thread::hardware_concurrency();
        
        // ==== SkyBox ====
        if (horizonCenterY > 0.0) {
            double yawRatio = (pl.yaw + pi) / (2*pi);
            int yScans = static_cast<int>(std::ceil((horizonCenterY - 1) / lineWidth));

            vector<std::thread> threads;
            for (unsigned int t = 0; t < threadCount; ++t) {
                threads.emplace_back([&, t] {
                    vector<QuadData> localQuads;
                    for (int y = yScans - 1 - t; y >= 0; y -= threadCount) {
                        for (int x = 0; x < numRays; ++x) {
                            int textY = textures["./textures/night_sky.ppm"].height - 1 - y;
                            int textureX = static_cast<int>(textures["./textures/night_sky.ppm"].width * yawRatio + x);
                            vecRGBA color = textures["./textures/night_sky.ppm"].get(textureX, textY);
                            float x1 = x * lineWidth, x2 = (x + 1) * lineWidth;
                            float y1 = horizonCenterY - (y * lineWidth), y2 = horizonCenterY - ((y - 1) * lineWidth);
                            localQuads.push_back({x1, y1, x2, y2, color});
                        }
                    }

                    std::scoped_lock lock(imageLock);
                    imageDefinition.insert(imageDefinition.end(), localQuads.begin(), localQuads.end());
                });
            }

            for (auto& thread : threads) thread.join();
        }

        // ==== Floor ====
        if (horizonCenterY < windowHeight) {
            int yScans = static_cast<int>(std::ceil((windowHeight - horizonCenterY - 1) / lineWidth));
            vector<std::thread> threads;

            for (unsigned int t = 0; t < threadCount; ++t) {
                threads.emplace_back([&, t] {
                    vector<QuadData> localQuads;
                    for (int y = t; y < yScans; y += threadCount) {
                        double horizonRatio = halfWindowHeight / ((y + 1) * lineWidth);
                        for (int x = 0; x < numRays; ++x) {
                            double rayAngle = minRay + (x * rayStep);
                            double floorRayLength = (Player::BASE_CAMERA_HEIGHT + pl.verticalOffset) * horizonRatio;
                            floorRayLength /= std::cos(std::abs(rayAngle - pl.yaw));
                            double brightness = (viewDistance - floorRayLength) / viewDistance;

                            double worldX = pl.position.x + floorRayLength * std::cos(rayAngle);
                            double worldY = pl.position.y + floorRayLength * std::sin(rayAngle);
                            vecRGBA color = textures["./textures/coarse_dirt.ppm"].get(worldX, worldY) * brightness;

                            float x1 = x * lineWidth, x2 = (x + 1) * lineWidth;
                            float y1 = horizonCenterY + (y * lineWidth), y2 = y1 + lineWidth;

                            localQuads.push_back({x1, y1, x2, y2, color});
                        }
                    }

                    std::scoped_lock lock(imageLock);
                    imageDefinition.insert(imageDefinition.end(), localQuads.begin(), localQuads.end());
                });
            }

            for (auto& thread : threads) thread.join();
        }

        // ==== Render ====
        glBegin(GL_QUADS);
        for (const QuadData& quad : imageDefinition) {
            glColor3f(quad.color.r, quad.color.g, quad.color.b);
            glVertex2f(quad.x1, quad.y1);
            glVertex2f(quad.x1, quad.y2);
            glVertex2f(quad.x2, quad.y2);
            glVertex2f(quad.x2, quad.y1);
        }
        glEnd();
    }

    void floorFill(vector<QuadData>* lclQuads, Sector* sect, double rayAngle, double cosineFactor, int rayIndex, double yStart, double yEnd) {
        const Texture* fillTexture = &textures[sect->floorTextureFile];
        double ySpan = yEnd - yStart;
        int yScans = ySpan > lineWidth ? static_cast<int>(ySpan / lineWidth): 1;
        double yScanLineSize = ySpan / yScans;
    
        double cameraZ = Player::BASE_CAMERA_HEIGHT + pl.verticalOffset;
        double floorZ = sect->baseHeight + sect->floatingHeight;
        double zDistance = cameraZ - floorZ;
        vec2 sectorCenter = sect->outline.centerPoint();
    
        for (int y = 0; y < yScans; ++y) {
            double screenY = yStart + (y + 0.5) * yScanLineSize; // center of scanline
            double screenYFromHorizon = screenY - horizonCenterY;
    
            if (std::abs(screenYFromHorizon) < 1e-5) continue;
    
            double rayLength = (zDistance * halfWindowHeight) / screenYFromHorizon;
            rayLength /= cosineFactor;

            vec2 worldCoord = {pl.position.x + rayLength * std::cos(rayAngle), pl.position.y + rayLength * std::sin(rayAngle)};
            worldCoord.orbit(sectorCenter, -sect->rotation);
            worldCoord -= sectorCenter;
    
            double brightness = (viewDistance - rayLength) / viewDistance;
            vecRGBA color = fillTexture->get(worldCoord.x, worldCoord.y) * brightness;
    
            float x1 = rayIndex * lineWidth, x2 = x1 + lineWidth;
            float y1 = yStart + y * yScanLineSize, y2 = y1 + yScanLineSize;
    
            if ((y1 < 0 && y2 < 0) || (y1 >= windowHeight && y2 >= windowHeight)) continue;
            if (y1 < 0) y1 = 0;
            if (y2 > windowHeight) y2 = windowHeight;
    
            lclQuads->push_back({x1, y1, x2, y2, color});
        }
    }
    
    void underSideFill(vector<QuadData>* lclQuads, Sector* sect, double rayAngle, double cosineFactor, int rayIndex, double yStart, double yEnd) {
        const Texture* fillTexture = &textures[sect->bottomTextureFile];
        double ySpan = yStart - yEnd;
        int yScans = ySpan > lineWidth ? static_cast<int>(ySpan / lineWidth) : 1;
        double yScanLineSize = ySpan / yScans;
    
        double cameraZ = Player::BASE_CAMERA_HEIGHT + pl.verticalOffset;
        double underSideZ = sect->floatingHeight;
        double zDistance = underSideZ - cameraZ; // opposite sign of floor (looking up)
        vec2 sectorCenter = sect->outline.centerPoint();
    
        for (int y = 0; y < yScans; ++y) {
            double screenY = yStart - (y * yScanLineSize); // working upward from yStart
            double screenYFromHorizon = horizonCenterY - screenY;
    
            if (std::abs(screenYFromHorizon) < 1e-5) continue;
    
            double rayLength = (zDistance * halfWindowHeight) / screenYFromHorizon;
            rayLength /= cosineFactor;
    
            vec2 worldCoord = {pl.position.x + rayLength * std::cos(rayAngle), pl.position.y + rayLength * std::sin(rayAngle)};
            worldCoord.orbit(sectorCenter, -sect->rotation);
            worldCoord -= sectorCenter;
    
            double brightness = (viewDistance - rayLength) / viewDistance;
            vecRGBA color = fillTexture->get(worldCoord.x, worldCoord.y) * brightness;
    
            float x1 = rayIndex * lineWidth, x2 = x1 + lineWidth;
            float y1 = screenY, y2 = y1 - yScanLineSize;
    
            // Clip
            if ((y1 < 0 && y2 < 0) || (y1 >= windowHeight && y2 >= windowHeight)) continue;
            if (y2 < 0) y2 = 0;
            if (y1 > windowHeight) y1 = windowHeight;
    
            lclQuads->push_back({x1, y1, x2, y2, color});
        }
    }

    void basicColumnFill(vector<QuadData>* lclQuads, const Texture* fillTexture, double x1, double x2, double yTop, double yBottom, int textureXIndex, double brightness = 1.0, double textureScaleY = 1.0, bool invertTexureY = false, double textureBaseSize = Sector::defaultWallHeight) {
        int yScans = static_cast<int>(textureBaseSize * textureScaleY);
        if (yScans < 1) yScans = 1.0;
        double ySpan = yBottom - yTop;
        double yScanlineHeight = ySpan / yScans;

        for (int y = 0; y < yScans; ++y) {
            int textureYIndex = invertTexureY ? y: fillTexture->height - y; // can be negative, texture lookup auto-wraps
            vecRGBA color = brightness * fillTexture->get(textureXIndex, textureYIndex);
            float y1 = yTop + (y * yScanlineHeight), y2 = y1 + yScanlineHeight;
            if ((y1 < 0 && y2 < 0) || (y1 >= windowHeight && y2 >= windowHeight)) continue;
            if (y1 < 0) y1 = 0;
            if (y2 >= windowHeight) y2 = windowHeight;
            lclQuads->push_back({ (float) x1, y1, (float) x2, y2, color});
        }
    }

    struct sectorFillMetadata {double basewWallBottomY, baseWallTopY, midWallBottomY, midWallTopY, brightness; int textureXIndex;};

    sectorFillMetadata  sectorDataCalc(const Sector* workingSector, const Sector::Wall* workingWall, const line2::collisionInfo& info, const double& rayLength) {
        double relativeYcomparedToHorizon = pl.cameraHeight / (rayLength * halfWindowHeight);
        double virtualY = horizonCenterY + relativeYcomparedToHorizon;
        double distanceFactor = halfWindowHeight / rayLength;
        double basewWallBottomY = virtualY + ((Player::BASE_CAMERA_HEIGHT + pl.verticalOffset - workingSector->floatingHeight) * distanceFactor); // bootom screen space y posiiton of sector base wall
        double baseWallTopY = basewWallBottomY - (workingSector->baseHeight * distanceFactor);
        double midWallBottomY = baseWallTopY;
        double midWallTopY = midWallBottomY - (workingWall->wallHeight * distanceFactor);
        
        double brightness = (viewDistance - rayLength) / viewDistance;
        int textureXIndex = static_cast<int>(info.fromP1.length());

        return {basewWallBottomY, baseWallTopY, midWallBottomY, midWallTopY, brightness, textureXIndex};
    }

    struct billBoardFillMetaData {double yTop, Ybottom, brightness; int textureIndex;};

    billBoardFillMetaData billBoardFillCalc(const Billboard* blbd, line2::collisionInfo& info, double rayLength) {
        double relativeYcomparedToHorizon = pl.cameraHeight / (rayLength * halfWindowHeight);
        double virtualY = horizonCenterY + relativeYcomparedToHorizon;
        double distanceFactor = halfWindowHeight / rayLength;
        double blbdBottomY = virtualY + ((Player::BASE_CAMERA_HEIGHT + pl.verticalOffset - blbd->verticalOffset) * distanceFactor);
        double blbdTopY = blbdBottomY - (blbd->height * distanceFactor);

        double brightness = (viewDistance - rayLength) / viewDistance;
        int textureXIndex = static_cast<int>(info.fromP1.length());
        return {blbdTopY, blbdBottomY, brightness, textureXIndex};
    }

    struct rayPack {
        bool isSector = false, isBillboard = false, isEntryRay = false, isDangling = false;
        int parentIndex = -1, subIndex = -1, pairPartnerIndex = -1;
        double distance3d = infinity, verticalOffset = -infinity;
        line2::collisionInfo collInfo;
    };
    vector<rayPack> rayView(const line2& ray, int x) {
        vector<Level::CastingPackage> secHitPack = lvl.sectorPack; // start by copying the list of sectors
        vector<rayPack> view;
    
        for (Level::CastingPackage& pack : secHitPack) {
            // Intersect all lines with ray
            for (int i = 0; i < pack.lines.size(); ++i) {
                auto& subgroup = pack.lines[i];
                subgroup.info = ray.findIntersection(subgroup.line, i);
            }
    
            // Remove lines with no collision
            pack.lines.erase(
                std::remove_if(pack.lines.begin(), pack.lines.end(),
                    [](Level::CastingPackage::subgroup& subgroup) {
                        return !subgroup.info.collisionFound;
                    }),
                pack.lines.end()
            );
    
            // Sort by distance if any remain
            if (!pack.lines.empty()) {
                std::sort(pack.lines.begin(), pack.lines.end(),
                    [](const Level::CastingPackage::subgroup& a, const Level::CastingPackage::subgroup& b) {
                        return a.info.rayLength > b.info.rayLength;
                    });
            }
    
            bool playerInSector = lvl.sectorList[pack.index_parentList].outline.isInside(pl.position);
            double verticalOffset = pack.maxZ - pl.cameraHeight;
    
            for (int i = 0; i < pack.lines.size(); ++i) {
                Level::CastingPackage::subgroup& line = pack.lines[i];
                rayPack r;
                r.isSector = true;
                r.parentIndex = pack.index_parentList;
                r.subIndex = line.index_parentList;
                r.distance3d = std::hypot(line.info.rayLength, verticalOffset);
                r.verticalOffset = verticalOffset;
                r.collInfo = line.info;
                r.pairPartnerIndex = -1;
            
                int wallCount = static_cast<int>(pack.lines.size());
            
                if (playerInSector) {
                    if (wallCount == 1) {
                        r.isDangling = true;
                        r.isEntryRay = false;
                    } else if (wallCount % 2 == 1 && i == wallCount - 1) {
                        r.isDangling = true;  // The closest wall (last in sorted list)
                        r.isEntryRay = false;
                    } else {
                        r.isEntryRay = (i % 2 == 1);  // Entry/exit alternate
                        r.isDangling = false;
                    }
                } else {
                    r.isDangling = false;
                    if (wallCount == 1) {
                        r.isEntryRay = true;
                    } else if (wallCount % 2 == 0) {
                        r.isEntryRay = (i % 2 == 1);  // Entry/exit alternate
                    } else {
                        r.isEntryRay = true;  // Fallback for degenerate case
                    }
                }
            
                view.push_back(r);
            }            
        }
    
        // Global sort: farthest to nearest
        std::sort(view.begin(), view.end(),
            [](const rayPack& a, const rayPack& b) {
                return a.distance3d > b.distance3d;
            });
    
        // Pair matching (safe version)
        for (int i = 0; i < view.size(); ++i) {
            rayPack& r = view[i];
            if (!r.isEntryRay && !r.isDangling && r.isSector && i + 1 < view.size()) {
                auto it = std::find_if(view.begin() + i + 1, view.end(),
                    [&](const rayPack& p) {
                        return p.parentIndex == r.parentIndex;
                    });
    
                if (it != view.end()) {
                    int pairIndex = std::distance(view.begin(), it);
                    r.pairPartnerIndex = pairIndex;
                    view[pairIndex].pairPartnerIndex = i;
                } else {
                    r.pairPartnerIndex = -1;
                }
            }
        }
    
        return view;
    }

    void rayCast() {
        minRay = pl.yaw - (0.5 * fovH);
        maxRay = minRay + fovH;
        halfFovH = (0.5 * fovH);
        rayStep = fovH / numRays;
        rayDistance = 5000;
        viewDistance = 1000;
        lineWidth = static_cast<double>(windowWidth) / numRays;

        rays = viewRays(pl, numRays, fovH, rayDistance);

        verticalShift = pl.pitch * (windowHeight / (pi * 0.25));
        horizonCenterY = (0.5 * windowHeight) - verticalShift + pl.verticalOffset;

        vector<QuadData> imageDefinition;
        std::mutex imageLock;

        const unsigned int threadCount = std::thread::hardware_concurrency();
        vector<std::thread> threads;

        for (unsigned int t = 0; t < threadCount; ++t) {
            threads.emplace_back([&, t] {
                vector<QuadData> localQuads;

                for (int x = t; x < static_cast<int>(rays.size()); x += threadCount) {
                    const line2& ray = rays[x];
                    const double rayAngle = minRay + rayStep * x;
                    const double cosineFactor = std::cos(std::abs(rayAngle - pl.yaw));
                    const double x1 = x * lineWidth, x2 = x1 + lineWidth;
                    vector<rayPack> view = rayView(ray, x);

                    for (const rayPack& obj : view) {
                        if (!obj.isSector || obj.parentIndex < 0 || obj.parentIndex >= lvl.sectorList.size()) continue;

                        Sector& workingSector = lvl.sectorList[obj.parentIndex];
                        if (obj.subIndex < 0 || obj.subIndex >= workingSector.walls.size()) continue;

                        Sector::Wall& workingWall = workingSector.walls[obj.subIndex];
                        if (!std::isfinite(obj.collInfo.rayLength) || obj.collInfo.rayLength <= 0.001) continue;

                        double adjustedDistance = obj.collInfo.rayLength * cosineFactor;
                        sectorFillMetadata wallInfo = sectorDataCalc(&workingSector, &workingWall, obj.collInfo, adjustedDistance);
                        double brightness = std::clamp((viewDistance - obj.collInfo.rayLength) / viewDistance, 0.0, 1.0);

                        if (obj.isDangling) {
                            if (wallInfo.midWallBottomY > horizonCenterY) {
                                if (workingWall.isVisibleWall) {
                                    basicColumnFill(&localQuads, &textures[workingWall.textureFile], x1, x2,
                                                    wallInfo.midWallTopY, wallInfo.midWallBottomY,
                                                    wallInfo.textureXIndex, brightness,
                                                    workingWall.wallHeight / workingSector.defaultWallHeight);
                                }
                                floorFill(&localQuads, &workingSector, rayAngle, cosineFactor, x,
                                        wallInfo.midWallBottomY, (double)windowHeight - 1.0);
                            } else {
                                basicColumnFill(&localQuads, &textures[workingWall.textureFile], x1, x2,
                                                wallInfo.baseWallTopY, wallInfo.basewWallBottomY,
                                                wallInfo.textureXIndex, brightness,
                                                workingSector.baseHeight / workingSector.defaultWallHeight);
                                underSideFill(&localQuads, &workingSector, rayAngle, cosineFactor, x,
                                            wallInfo.basewWallBottomY, 0.0);
                            }
                        }
                        else if (obj.isEntryRay) {
                            if (workingWall.isVisibleWall) {
                                basicColumnFill(&localQuads, &textures[workingWall.textureFile], x1, x2,
                                                wallInfo.midWallTopY, wallInfo.midWallBottomY,
                                                wallInfo.textureXIndex, brightness,
                                                workingWall.wallHeight / workingSector.defaultWallHeight);
                            }

                            basicColumnFill(&localQuads, &textures[workingSector.bottomTextureFile], x1, x2,
                                            wallInfo.baseWallTopY, wallInfo.basewWallBottomY,
                                            wallInfo.textureXIndex, brightness,
                                            workingSector.baseHeight / workingSector.defaultWallHeight);

                            if (obj.pairPartnerIndex >= 0 && obj.pairPartnerIndex < static_cast<int>(view.size())) {
                                const rayPack& pair = view[obj.pairPartnerIndex];
                                if (!std::isfinite(pair.collInfo.rayLength)) continue;

                                sectorFillMetadata pairWallInfo = sectorDataCalc(&workingSector, &workingWall,
                                                                                pair.collInfo, pair.collInfo.rayLength * cosineFactor);
                                if (pairWallInfo.basewWallBottomY < horizonCenterY) {
                                    underSideFill(&localQuads, &workingSector, rayAngle, cosineFactor, x,
                                                pairWallInfo.basewWallBottomY, wallInfo.basewWallBottomY);
                                }
                            }
                        }
                        else { // Exit ray
                            if (workingWall.isVisibleWall) {
                                basicColumnFill(&localQuads, &textures[workingWall.textureFile], x1, x2,
                                                wallInfo.midWallTopY, wallInfo.midWallBottomY,
                                                wallInfo.textureXIndex, brightness,
                                                workingWall.wallHeight / workingSector.defaultWallHeight);
                            }

                            if (obj.pairPartnerIndex >= 0 && obj.pairPartnerIndex < static_cast<int>(view.size())) {
                                const rayPack& pair = view[obj.pairPartnerIndex];
                                if (!std::isfinite(pair.collInfo.rayLength)) continue;

                                sectorFillMetadata pairWallInfo = sectorDataCalc(&workingSector, &workingWall,
                                                                                pair.collInfo, pair.collInfo.rayLength * cosineFactor);
                                if (pairWallInfo.midWallBottomY > horizonCenterY) {
                                    floorFill(&localQuads, &workingSector, rayAngle, cosineFactor, x,
                                            wallInfo.baseWallTopY, pairWallInfo.baseWallTopY);
                                }
                            }
                        }
                    }
                }

                std::scoped_lock lock(imageLock);
                imageDefinition.insert(imageDefinition.end(), localQuads.begin(), localQuads.end());
            });
        }

        for (auto& thread : threads) thread.join();

        // Final rendering pass
        glBegin(GL_QUADS);
        for (const QuadData& quad : imageDefinition) {
            glColor3f(quad.color.r, quad.color.g, quad.color.b);
            glVertex2f(quad.x1, quad.y1);
            glVertex2f(quad.x1, quad.y2);
            glVertex2f(quad.x2, quad.y2);
            glVertex2f(quad.x2, quad.y1);
        }
        glEnd();
    }

    
};