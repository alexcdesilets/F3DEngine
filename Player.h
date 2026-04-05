#pragma once

#include "globals.h"
#include "level.h"
#include "sector.h"

using std::vector;
using std::pair;
using namespace vecSys;

extern Level lvl;

struct Player {
    static constexpr double BASE_CAMERA_HEIGHT = 64.0; // Your base camera height
    static constexpr double GRAVITY = 0.18; // Acceleration due to gravity (pixels/second^2) - Adjust this value!
    static constexpr double JUMP_PEAK_HEIGHT = 64.0; // Desired height relative to base

    vec2 position;
    double yaw, pitch;
    double speed;

    // New Members for Jumping
    double verticalOffset = 0.0; // Current height offset from the ground (or base height)
    double verticalSpeed = 0.0;  // Current upward/downward speed
    bool isOnGround = false;      // Flag to track if the player can jump

    double cameraHeight = BASE_CAMERA_HEIGHT + verticalOffset;
    int lastDominantSector = -1;

    pair<double, double> viewAngles(double fovRadians = pi * 0.5f) {
        fovRadians /= 2.0f;
        return {yaw - fovRadians, yaw + fovRadians};
    }

    void rotate(const double& radians) {
        yaw += radians;
        if (yaw <= -pi) yaw += tau;
        else if (yaw > pi) yaw -= tau;
    }

    void rotatePitch(const double& radians) {
        pitch += radians;
        pitch = std::clamp(pitch, -0.5 * pi, 0.5 * pi);
    }

    void move(const double& ratio = 1.0, double wallBumpAmount = 1e-2) {
        // Create a list of sectors the player is vertically inside
        std::vector<Sector> onRelevantZHeight;
        std::copy_if(lvl.sectorList.begin(), lvl.sectorList.end(), std::back_inserter(onRelevantZHeight),
            [&](const Sector& sect) {
                return (verticalOffset < sect.floatingHeight + sect.baseHeight + sect.defaultWallHeight) &&
                       (cameraHeight > sect.floatingHeight);
            });
    
        auto collideAndSlide = [&](std::vector<Sector>& sectList, vec2& movAmnt) -> vec2 {
            for (Sector& sect : sectList) {
                line2 movement = {position, position + movAmnt};
    
                for (int i = 0; i < sect.wallCount(); ++i) {
                    Sector::Wall& aWall = sect.walls[i];
                    line2 collisionLine = {sect.outline.points[i], sect.outline.points[(i + 1) % sect.wallCount()]};
    
                    double bottomHeight = sect.floatingHeight;
                    double floorHeight = bottomHeight + sect.baseHeight;
                    double wallTop = floorHeight + aWall.wallHeight;
    
                    // Accurate vertical overlap checks:
                    bool inBaseWall = (verticalOffset < floorHeight && cameraHeight > bottomHeight);
                    bool inMidWall  = aWall.isBarrier && (verticalOffset < wallTop && cameraHeight > floorHeight);
    
                    if (!(inBaseWall || inMidWall)) continue;
    
                    vec2 intersectionPoint = movement.intersection(collisionLine);
                    if (intersectionPoint.isInfinite()) continue;
    
                    // 1. Move up to just before the collision point
                    vec2 toCollisionPoint = intersectionPoint - position;
                    if (toCollisionPoint.lengthSquared() > wallBumpAmount * wallBumpAmount) {
                        vec2 bumpOffset = toCollisionPoint.normalized() * wallBumpAmount;
                        position += (toCollisionPoint - bumpOffset);
                    }
    
                    // 2. Calculate leftover movement
                    vec2 leftoverMovement = movAmnt - toCollisionPoint;
    
                    // 3. Slide along wall
                    vec2 wall_normal = collisionLine.normal().normalized();
                    if (movAmnt.normalized().dot(wall_normal) < 0.0) {
                        wall_normal = wall_normal.opposite();
                    }
    
                    vec2 perp_component = wall_normal * leftoverMovement.dot(wall_normal);
                    vec2 slide_vector = leftoverMovement - perp_component;
    
                    return slide_vector;
                }
            }
    
            return vec2{infinity, infinity};
        };
    
        // Initial movement
        double adjustedSpeed = speed * ratio;
        vec2 initial_mov = vec2::fromAngle(yaw) * adjustedSpeed;
    
        if (onRelevantZHeight.empty()) {
            position += initial_mov;
            return;
        }
    
        // Attempt move and slide
        vec2 leftOverMovement = collideAndSlide(onRelevantZHeight, initial_mov);
    
        if (leftOverMovement.isInfinite()) {
            position += initial_mov;
            return;
        }
    
        vec2 finalSignal = collideAndSlide(onRelevantZHeight, leftOverMovement);
    
        if (finalSignal.isInfinite()) {
            position += leftOverMovement;
        }
    }
    

    void jump() {
        if (isOnGround) {
            // Calculate initial vertical speed needed to reach JUMP_PEAK_HEIGHT
            // Formula: v0 = sqrt(2 * g * h)
            verticalSpeed = std::sqrt(2.0 * GRAVITY * JUMP_PEAK_HEIGHT);
            isOnGround = false;
        }
    }

    int findDominantSector() {
        // Removed bestSectorIndex and highestZInRange as they relate to being *inside* the solid
        double highestFloorBelow = -infinity; // Track highest floor *below* player
        int floorBelowIndex = -1;
    
        // Iterate through all sectors in the level
        for (int i = 0; i < lvl.sectorList.size(); ++i) {
            const Sector& sect = lvl.sectorList[i]; // Use const reference
            
            if (sect.outline.isInside_AABB(position)) {
            // Check if player is horizontally inside the sector's outline
                if (sect.outline.isInside(position)) { // Checks if position is inside the shape outline
                    double sectorTop = sect.floatingHeight + sect.baseHeight; // Calculate top Z of the sector floor
                    // double sectorBottom = sect.floatingHeight; // No longer needed for this check
        
                    // Check if player is *at or above* this sector's floor
                    // Use a small tolerance if needed for floating point comparisons near the floor
                    constexpr double floorTolerance = 1e-4;
                    if (verticalOffset >= sectorTop - floorTolerance) {
                        // If this sector's top is higher than the highest floor found *below* (or at) the player so far
                        if (sectorTop > highestFloorBelow) {
                            highestFloorBelow = sectorTop; // Update the highest floor below/at
                            floorBelowIndex = i;       // Store the index of this sector
                        }
                    }
                    // No need for the else-if checking inside the solid volume
                }
            }
        }
    
        // Return the index of the highest floor found below or at the player's height.
        if (floorBelowIndex != -1) {
             lastDominantSector = floorBelowIndex;
             return floorBelowIndex;
        }
    
        // If player is not inside any sector horizontally, or is below all floors they are inside.
        lastDominantSector = -1;
        return -1; // No relevant sector found
    }

    // In updateVerticalMovement() within Player.h
    void physicUpdate() {
        // ... (epsilon definition, cameraHeight update) ...

        int sectorIndex = findDominantSector(); // Call the updated function
        double sectorFloorHeight = 0.0; // Default floor height to 0 (absolute ground)

        // Check if a valid sector index was returned
        if (sectorIndex != -1) {
            // Get the sector using the potentially updated index
            const Sector& sect = lvl.sectorList[sectorIndex]; // Use const reference
            // Set the floor height based on the dominant sector
            sectorFloorHeight = sect.floatingHeight + sect.baseHeight;

            if (isOnGround) {
                if (sect.lastFrame.moved) {
                    position += sect.lastFrame.motion;
                    verticalOffset += sect.lastFrame.baseHeightChange;
                }
                if (sect.lastFrame.orbited) {
                    position.orbit(sect.lastFrame.orbitalPoint, sect.lastFrame.orbitRads);
                    if(sect.lastFrame.rotated) {
                        yaw += sect.lastFrame.rotationRads;
                        double rotOrbit = sect.lastFrame.rotationRads - sect.lastFrame.orbitRads;
                        position.orbit(sect.outline.centerPoint(), rotOrbit);
                    }
                    else {
                        position.orbit(sect.outline.centerPoint(), -sect.lastFrame.orbitRads);
                    }
                }
                else if(sect.lastFrame.rotated) {
                    yaw += sect.lastFrame.rotationRads;
                    position.orbit(sect.outline.centerPoint(), sect.lastFrame.rotationRads);
                }  
            }
        }

        std::vector<Sector> movingSectors;
        std::copy_if(lvl.sectorList.begin(), lvl.sectorList.end(), std::back_inserter(movingSectors),
            [&](const Sector& sect) {
                return (sect.lastFrame.moved || sect.lastFrame.orbited || sect.lastFrame.rotated) && 
                    (verticalOffset < sect.floatingHeight + sect.baseHeight + sect.defaultWallHeight) &&
                    (cameraHeight > sect.floatingHeight);
        });

        vec2 totalPush = {0.0, 0.0};
        constexpr double pushRadius = 0.5; // Adjust based on collision expectations

        for (const Sector& ms : movingSectors) {
            vec2 closest = ms.outline.closestPoint(position);
            vec2 delta = position - closest;
            double dist = delta.length();
            if (dist < pushRadius && dist > 1e-6) {
                // Repel proportionally to distance inside the push zone
                vec2 pushDir = delta.normalized() * (pushRadius - dist);
                totalPush += pushDir;
            }
        }

        position += totalPush;

        // --- Apply physics ---
         if (verticalOffset > sectorFloorHeight + 0.01) { // Check against the determined floor height
              isOnGround = false;
         }


        if (!isOnGround) {
            verticalSpeed -= GRAVITY;
            verticalOffset += verticalSpeed;

            // Check for landing ON or BELOW the determined floor height
            if (verticalOffset <= sectorFloorHeight) {
                verticalOffset = sectorFloorHeight; // Snap to the correct floor
                verticalSpeed = 0.0;
                isOnGround = true;
            }
        } else {
            // If already on ground, ensure position matches the current floor height
             verticalOffset = sectorFloorHeight;
             verticalSpeed = 0.0;
        }
         cameraHeight = BASE_CAMERA_HEIGHT + verticalOffset; // Update camera height after position change
    }
};