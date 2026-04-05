
    vector<Level::CastingPackage> rayView(const line2& ray) {
        vector<Level::CastingPackage> finalView = lvl.sectorPack; // start by copying the list of sectors
        for (Level::CastingPackage& pack: finalView) {
            for (int i = 0; i < pack.lines.size(); ++i) {
                auto& subgroup = pack.lines[i];
                subgroup.info = ray.findIntersection(subgroup.line, i);
            }
            pack.lines.erase(std::remove_if(pack.lines.begin(), pack.lines.end(), [](Level::CastingPackage::subgroup& subgroup) {
                return !subgroup.info.collisionFound;
            }),pack.lines.end());
            if (pack.lines.empty()) continue;
            std::stable_sort(pack.lines.begin(), pack.lines.end(), [](const Level::CastingPackage::subgroup& a, const Level::CastingPackage::subgroup& b){
                return a.info.rayLength > b.info.rayLength;
            });
        }

        finalView.erase(std::remove_if(finalView.begin(), finalView.end(),
            [](const Level::CastingPackage& pack){
                return pack.lines.size() < 1;
            }), finalView.end()
        );

        struct SortingPackage {
            double vOffset = -INFINITY, vOffsetFromBellow = -INFINITY, nearEdgeRayDistance = -INFINITY, farEdgeRayDistance = -INFINITY, maxZ = -INFINITY, minZ = -INFINITY; 
            bool inside = false, plAbove = false;
        };
        std::unordered_map<int, SortingPackage> prevs;

        for (const Level::CastingPackage& pack: finalView) {
            prevs[pack.index_parentList].maxZ = pack.maxZ; // the index exists now
            SortingPackage& p = prevs[pack.index_parentList]; // shortcut to the index now that it exists
            const Sector& s = lvl.sectorList[pack.index_parentList];
            p.minZ = pack.minZ;
            p.vOffset = std::abs(pack.maxZ - pl.cameraHeight);
            p.vOffsetFromBellow = std::abs(pack.minZ - pl.cameraHeight);
            if (s.outline.isInside_AABB(pl.position)) {
                p.inside = s.outline.isInside(pl.position);
            }
            p.nearEdgeRayDistance = pack.lines.back().info.rayLength;
            p.farEdgeRayDistance = pack.lines[0].info.rayLength;
            p.plAbove = pl.cameraHeight >= pack.maxZ;
        }

        std::stable_sort(finalView.begin(), finalView.end(), [&prevs](const Level::CastingPackage& a, const Level::CastingPackage& b) {
            // SortingPackage calculation if they exist
            const SortingPackage& prevA = prevs[a.index_parentList];
            const SortingPackage& prevB = prevs[b.index_parentList];

            // Higher level blocks
            if (prevA.inside && prevB.inside) {
                if ((prevA.plAbove && prevB.plAbove)) {
                    if (prevA.vOffset <= prevB.vOffset) { // Sector A is the closer one
                        return true; // Sector A always occludes sector B
                    }
                    // Sector B is the closer one
                    return false; // Sector B always occludes sector A
                }
                else if (!(prevA.plAbove && prevB.plAbove)) {
                    if (prevA.vOffsetFromBellow <= prevB.vOffsetFromBellow) { // Sector A is the closer one
                        return true; // Sector A always occludes sector B
                    }
                    // Sector B is the Closer one
                    return false; // Sector B always occludes sector A
                }
                // sectors can't interefere visually use closest distance
                return prevA.nearEdgeRayDistance < prevB.nearEdgeRayDistance; // farthest sector get drawn first
            }
            else if (prevA.inside && !prevB.inside) { // We can see every edge on B but only the closest edge on A matters
                if (prevA.plAbove && prevB.plAbove) {
                    if (prevA.vOffset <= prevB.vOffset) {
                        return true;
                    }
                    // sector B is technically above sector A but it's floating bellow the camera height
                    // sector B only occludes sector a if the near egdge of sector b is closer than the near edge of sector A
                    return prevA.nearEdgeRayDistance < prevB.nearEdgeRayDistance;
                }
                else if (!(prevA.plAbove && prevB.plAbove)) { // both of the sectors are above the player
                    if (prevA.vOffsetFromBellow <= prevB.vOffsetFromBellow) { // Sector A always occludes sector B
                        return true;
                    }
                    // sector A still has a chance at occludding Sector B
                    else if (prevA.vOffsetFromBellow <= prevB.vOffset) { // Sector B's bottom is bellow sector A's bottom but the top of Sector B is still occluded by sector A
                        return true;
                    }
                    // Sector be is sandwiched between the cameraZheight and Sector A's bottom, but it could still be farther away visually than Sector B
                    return prevA.nearEdgeRayDistance < prevB.nearEdgeRayDistance;
                }
                // the sectors are on different sides of the camera's division line, they cannot visually interfere
                return prevA.nearEdgeRayDistance < prevB.nearEdgeRayDistance;
            }
            else if (prevB.inside && !prevA.inside) { // we can see every edge on A but only the closest edge on B matters invert logic of previous block
                if (prevB.plAbove && prevA.plAbove) {
                    if (prevB.vOffset <= prevA.vOffset) {
                        return true;
                    }
                    return prevB.nearEdgeRayDistance < prevA.nearEdgeRayDistance;
                }
                else if (!(prevB.plAbove && prevA.plAbove)) {
                    if (prevB.vOffsetFromBellow <= prevA.vOffsetFromBellow) {
                        return true;
                    }
                    else if (prevB.vOffsetFromBellow <= prevA.vOffset) {
                        return true;
                    }
                    return prevB.nearEdgeRayDistance < prevA.nearEdgeRayDistance;
                }
                return prevB.nearEdgeRayDistance < prevA.nearEdgeRayDistance;
            }
            // Do the inside neither logic here
            // we can see every edge on both sectors without factoring in occlusion
            // Figure out the occlusion
            if ((a.index_parentList == 1 || a.index_parentList == 2) && (b.index_parentList == 1 || b.index_parentList == 2)) {
                using namespace std;
                cout << format("sector {}: {}, {}, {}, {}, {}, {}", "A",a.index_parentList, prevA.farEdgeRayDistance, prevA.nearEdgeRayDistance, prevA.plAbove, prevA.vOffset, prevA.vOffsetFromBellow) << endl;
                cout << format("sector {}: {}, {}, {}, {}, {}, {}", "B",b.index_parentList, prevB.farEdgeRayDistance, prevB.nearEdgeRayDistance, prevB.plAbove, prevB.vOffset, prevB.vOffsetFromBellow) << endl;
            }
            if (prevA.plAbove && prevB.plAbove) { // Both sectors are above or bellow the player
                if (prevA.vOffset <= prevB.vOffset) { // Sector A is vertically closer than sector B
                    // Use the near edge of A and the far edge of B, because all of B could be present if it's far edge is closer than A near edge

                    return prevA.nearEdgeRayDistance > prevB.farEdgeRayDistance;
                }
                // Use the near edge of B and the far edge of A, because B can occlude A if it's far edge is farther than B's near edge
                return prevB.nearEdgeRayDistance < prevA.farEdgeRayDistance;
            }
            else if (!(prevA.plAbove && prevB.plAbove)) {
                if (prevA.vOffsetFromBellow < prevB.vOffsetFromBellow) { // Sector A only occludes sector B if it's near edge is closer
                     // Use the near edge of A and the far edge of B, because all of B could be present if it's far edge is closer than A near edge
                    return prevA.nearEdgeRayDistance < prevB.farEdgeRayDistance;
                }
                // Use the near edge of B and the far edge of A, because B can occlude A if it's far edge is farther than B's near edge
                return prevB.nearEdgeRayDistance < prevA.farEdgeRayDistance;
            }
            // At this point, neither sector is capable of visually interfering with each other
            
            return prevA.nearEdgeRayDistance < prevB.nearEdgeRayDistance;
        });

        vector<Level::CastingPackage> bbpack = lvl.billboardPack;
        if (bbpack.empty()) return finalView;

        for (Level::CastingPackage& pack: bbpack) {
            pack.lines[0].info = ray.findIntersection(pack.lines[0].line, -1);
        }

        bbpack.erase(
            std::remove_if(bbpack.begin(), bbpack.end(), 
            [] (Level::CastingPackage& pack) {
                return !pack.lines[0].info.collisionFound;
            }) , bbpack.end()
        );

        if (bbpack.empty()) return finalView;

        std::stable_sort(bbpack.begin(), bbpack.end(), [](const Level::CastingPackage& a, const Level::CastingPackage& b) {
            return a.lines[0].info.rayLength >= b.lines[0].info.rayLength;
        });

        finalView.reserve(finalView.size() + bbpack.size());
        vector<int> bblowerDominant(bbpack.size(), -1); // Where to find the dominant sector, if any in the finalView
        vector<int> bbuppderDominant(bbpack.size(), -1);
        vector<int> insertionPoints(finalView.size(), -1); // Where to insert a given billboard based on occlusion checks between the dominant sector and every sector visually in front of it

        for (int i = 0; i < bbpack.size(); ++i) {
            Billboard& bb = lvl.billboardList[bbpack[i].index_parentList];
            vec2 bbcenter = bb.base.centerPoint();
            double SortingPackageZMax = -std::numeric_limits<double>::infinity();
            double SortingPackageZuppMax = std::numeric_limits<double>::infinity();
            for (int j = 0; j < finalView.size(); ++j) {
                if (lvl.sectorList[finalView[j].index_parentList].outline.isInside(bbcenter, 1e-6)) {
                    if (finalView[j].maxZ <= bb.verticalOffset) {
                        if (finalView[j].maxZ >= SortingPackageZMax) {
                            SortingPackageZMax = finalView[j].maxZ;
                            bblowerDominant[i] = j;
                        }
                    }
                }
            }
            for (int j = finalView.size() - 1; j > bblowerDominant[i]; --j) {
                if (lvl.sectorList[finalView[j].index_parentList].outline.isInside(bbcenter, 1e-6)) {
                    if (finalView[j].maxZ > bb.verticalOffset) {
                        if (finalView[j].maxZ < SortingPackageZuppMax) {
                            SortingPackageZuppMax = SortingPackageZMax = finalView[j].maxZ;
                            bbuppderDominant[i] = j;
                        }
                    }
                }
            }
        }

        // We now know what sector a billboard shoul be between if any.
        // we have to work backwards down the bbpack and the final view, so largest index to smallest
        // if the bbupperdominant wasn't found, we work from the start of the list to the bblowerdominnant if found
        // we use closest edge comparison to place the billboard visually in front of sectors (higher index) that are between the sandwich, even if it's an open face sandwich
        // we have to work backwards because the insertion will push everything behind it to higher index locations and this also means we have to increment bbupperdominant with each insertion
        // a special case is that if bblowerdominant is above the player camera, then we automatically know that the billboard comes iimedidiately before bblowerdominant (lower index) without any additional checks
        
        // Calculate all insertionPoints
        for (int i = 0; i < bbpack.size(); ++i) {
            Level::CastingPackage& bb = bbpack[i];
            int lowerDom = bblowerDominant[i];
            int upperDom = bbuppderDominant[i];

            if (lowerDom != -1 && finalView[lowerDom].maxZ > pl.cameraHeight) {
                insertionPoints[i] = lowerDom; // Special case: above player
            }
            else if (upperDom != -1) {
                for (int j = upperDom -1; j > (lowerDom != -1 ? lowerDom : 0); --j) {
                    const auto& sector = finalView[j];
                    if (bb.lines[0].info.rayLength <= sector.lines.back().info.rayLength) {
                        insertionPoints[i] = j + 1;
                        break;
                    }
                    insertionPoints[i] = lowerDom + 1;
                }
            }
            else if (lowerDom != -1) {
                for (int j = finalView.size() - 1; j > lowerDom; --j) {
                    const auto& sector = finalView[j];
                    if (bb.lines[0].info.rayLength <= sector.lines.back().info.rayLength) {
                        insertionPoints[i] = j + 1;
                        break;
                    }
                    insertionPoints[i] = lowerDom + 1;
                }
            }
            else {
                for (int j = finalView.size() - 1; j >= 0; --j) {
                    const auto& sector = finalView[j];
                    if (bb.lines[0].info.rayLength <= sector.lines.back().info.rayLength) {
                        insertionPoints[i] = j;
                        break;
                    }
                    insertionPoints[i] = 0;
                }
            }
        }

        for (int i = bbpack.size()-1; i >= 0; --i) {
            finalView.insert(finalView.begin() + insertionPoints[i], bbpack[i]);
        }

        return finalView;
    }