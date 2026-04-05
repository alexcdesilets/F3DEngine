#pragma once
#include <vector>
#include <fstream>
#include <string>

#include "vecSys/vec2.h"
#include "vecSys/line2.h"
#include "vecSys/shape2.h"

#include "sector.h"
#include "billboard.h"

using namespace vecSys;
using std::vector;
using std::pair;

struct Level {

    struct CastingPackage {
        struct subgroup {line2 line; line2::collisionInfo info; int index_parentList;};
        bool isSector = true;
        vector<subgroup> lines;
        int index_parentList = -1;
        double minZ = - 1.0, maxZ = -1.0;
    };

    vector<Sector> sectorList;
    vector<Billboard> billboardList;
    vector<CastingPackage> sectorPack;
    vector<CastingPackage> billboardPack;

    void update() {
        // input things here that should be called before the level renders
        packageBillboards();
        packageSectors();
    }

    vector<CastingPackage> packageSectors() {
        sectorPack.clear();
        sectorPack.reserve(sectorList.size());
        for (int i = 0; i < sectorList.size(); ++i) {
            Sector& ws = sectorList[i];
            vector<line2> s_lines = ws.outline.toLines();
            CastingPackage pack;
            pack.isSector = true;
            pack.minZ = ws.floatingHeight;
            pack.maxZ = ws.floatingHeight + ws.baseHeight;
            pack.index_parentList = i;
            for (int j = 0; j < s_lines.size(); ++j) {
                line2::collisionInfo coll;
                pack.lines.push_back({s_lines[j], coll, j});
            }
            sectorPack.push_back(pack);
        }
        return sectorPack;
    }

    vector<CastingPackage> packageBillboards() {
        billboardPack.clear();
        billboardPack.reserve(billboardList.size());
        for (int i = 0; i < billboardList.size(); ++i) {
            const Billboard& bb = billboardList[i];
            CastingPackage pack;
            pack.isSector = false;
            pack.minZ = bb.verticalOffset;
            pack.maxZ = bb.verticalOffset + bb.height;
            pack.index_parentList = i;
            line2::collisionInfo coll;
            pack.lines.push_back({bb.base, coll, 0});
            billboardPack.push_back(pack);
        }
        return billboardPack;
    }
    
};