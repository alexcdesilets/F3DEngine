void initLevel() { // test level for program desgin and render logic testing
    shape2 levelBorders = {{{0,0}, {320, -200} ,{640,0}, {640,640}, {100, 320}, {0,640}}};
    Sector sector1;
    sector1.setShape(levelBorders);
    for (Sector::Wall& wall: sector1.walls) {
        wall.textureFile = "./textures/cobbled_deepslate.ppm";
    }
    sector1.walls[5].isBarrier = false;
    sector1.walls[5].isVisibleWall = false;
    sector1.floorTextureFile = "./textures/coarse_dirt.ppm";
    sector1.bottomTextureFile = "./textures/cobbled_deepslate.ppm";

    shape2 platformDefinition = {{{300,300},{340,300},{360,320},{340,340},{300,340},{280,320}}};
    Sector sector2;
    sector2.setShape(platformDefinition);
    for (Sector::Wall& wall: sector2.walls) {
        wall.textureFile = "./textures/cobbled_deepslate.ppm";
        wall.isBarrier = false;
        wall.isVisibleWall = false;
    }
    sector2.floatingHeight = 16.0;
    sector2.floorTextureFile = "./textures/jungle_planks.ppm";
    sector2.bottomTextureFile = "./textures/cobbled_deepslate.ppm";

    Sector sector3 = sector2;
    sector3.move({40,40}, 48);
    sector3.rotate(0.5*pi);
    sector3.lastFrame.reset();

    Sector sector4 = sector3;
    sector4.move({40,40}, 48);
    sector4.rotate(0.5*pi);
    sector4.lastFrame.reset();

    Sector sector5 = sector4;
    sector5.move({40,0}, 48);
    sector5.rotate(0.5*pi);
    sector5.lastFrame.reset();

    Sector sector6 = sector5;
    sector6.move({40,0}, 48);
    sector6.rotate(0.5*pi);
    sector6.lastFrame.reset();

    lvl.sectorList.push_back(sector1);
    lvl.sectorList.push_back(sector2);
    lvl.sectorList.push_back(sector3);
    lvl.sectorList.push_back(sector4);
    lvl.sectorList.push_back(sector5);
    lvl.sectorList.push_back(sector6);

    lvl.update();
}