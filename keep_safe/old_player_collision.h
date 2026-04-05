    // void move(const double& ratio = 1.0, double wallBumpAmount = 1e-2) {

    //     // create a list of meaningfully collideable sectors
    //     vector<Sector> onRelevantZHeight;
    //     std::copy_if(lvl.sectorList.begin(), lvl.sectorList.end(), std::back_inserter(onRelevantZHeight),
    //         [&](Sector sect) {
    //             return (verticalOffset >= sect.floatingHeight && verticalOffset <= sect.floatingHeight + sect.baseHeight);
    //     });
    //     // On relevant Z height now only contains sectors the player could coliide with

    //     auto collideAndSlide = [&](vector<Sector>& sectList, vec2& movAmnt) -> vec2 { // Per Sector collide and slide
    //         // This is the movement to be had

    //         for (Sector& sect : sectList) {
    //             line2 movement = {position, position + movAmnt};
    //             for (int i = 0; i < sect.wallCount(); ++i) {
    //                 Sector::Wall& aWall = sect.walls[i]; //reference to wall to make it less verbose to access it's properties
    //                 line2 collisionLine = {sect.outline.points[i], sect.outline.points[(i+1) % sect.wallCount()]};
                    
    //                 double bottomHeight = sect.floatingHeight, floorHeight = bottomHeight + sect.baseHeight, wallTop = floorHeight + aWall.wallHeight;

    //                 if ((bottomHeight >= verticalOffset && bottomHeight <= cameraHeight) ||
    //                    (floorHeight >= verticalOffset && floorHeight <= cameraHeight) ||
    //                    (aWall.isBarrier && verticalOffset > floorHeight && verticalOffset <= wallTop)
    //                 ) {
    //                     vec2 intersectoinPoint = movement.intersection(collisionLine);
    //                     if (intersectoinPoint.isInfinite()) continue;
                        
    //                     // 1. Move player to just before the collision point
    //                     vec2 toCollionPoint = intersectoinPoint - position;
    //                     // Slightly shorter to avoid floating point issues / embedding in wall
    //                     if (toCollionPoint.lengthSquared() > wallBumpAmount * wallBumpAmount) {
    //                         vec2 bumpOffset = toCollionPoint.normalized() * wallBumpAmount;
    //                         position += (toCollionPoint - bumpOffset);
    //                     }
                    
    //                     // 2. Calculate remaining movement *potential*
    //                     vec2 leftoverMovment = movAmnt-toCollionPoint;

    //                     // 3. Calculate the slide vector by projecting remaining movement onto the wall plane
    //                     vec2 wall_normal = collisionLine.normal().normalized();

    //                     // Ensure the normal points OUTWARD from the wall relative to the player's approach.
    //                     // The movement vector points from player towards collision. If dot product with normal is positive, normal is already pointing away.
    //                     // If negative, the normal points towards the player, so flip it.
    //                     if (movAmnt.normalized().dot(wall_normal) < 0.0) {
    //                         wall_normal = wall_normal.opposite();
    //                     }

    //                     // Project remaining_mov onto the wall normal (this is the part of movement INTO the wall)
    //                     vec2 perpendicular_component = wall_normal * leftoverMovment.dot(wall_normal);

    //                     // Subtract the perpendicular component to get the sliding component (parallel to the wall)
    //                     vec2 slide_vector = leftoverMovment - perpendicular_component;

    //                     return slide_vector;
    //                 }
    //             }
    //         }
    //         return vec2{vec2::_inf, vec2::_inf}; // singal no collision found
    //     };

    //     double adjustedSpeed = speed * ratio;
    //     vec2 initial_mov = vec2::fromAngle(yaw) * adjustedSpeed; // Initial full movement vector
    
    //     if (onRelevantZHeight.empty()) {
    //         position += initial_mov;
    //         return;
    //     }

    //     vec2 leftOverMovement = collideAndSlide(onRelevantZHeight, initial_mov);

    //     if (leftOverMovement.isInfinite()){
    //          position += initial_mov;
    //          return;
    //     };
        
    //     vec2 finalSignal = collideAndSlide(onRelevantZHeight, leftOverMovement);

    //     if (finalSignal.isInfinite()) position += leftOverMovement;
    // }