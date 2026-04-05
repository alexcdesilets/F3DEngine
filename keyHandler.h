#pragma once

#include "globals.h"

void gamePLayKeys() {
    double movementAngleMod = 0.0;
    bool forwards = keyFlags['w'] && !keyFlags['s'];
    bool backwards = keyFlags['s'] && !keyFlags['w'];
    bool left = keyFlags['a'] && !keyFlags['d'];
    bool right = keyFlags['d'] && !keyFlags['a'];

    if (forwards) {
        if (right) {
            movementAngleMod += 0.25 * pi;
        }
        if (left) {
            movementAngleMod -= 0.25*pi;
        }
        if (movementAngleMod != 0.0) {
            pl.rotate(movementAngleMod);
            pl.move(movementRatio);
            pl.rotate(-movementAngleMod);
        }
        else {
            pl.move(movementRatio);
        }
    }
    else if (backwards) {
        if (right) {
            movementAngleMod -= 0.25 * pi;
        }
        if (left) {
            movementAngleMod += 0.25*pi;
        }
        if (movementAngleMod != 0.0) {
            pl.rotate(movementAngleMod);
            pl.move(-0.75);
            pl.rotate(-movementAngleMod);
        }
        else {
            pl.move(-0.75);
        }
    }
    else if (left) {
        movementAngleMod = 0.5*pi;
        pl.rotate(-movementAngleMod);
        pl.move(movementRatio);
        pl.rotate(movementAngleMod);
    }
    else if (right) {
        movementAngleMod = 0.5*pi;
        pl.rotate(movementAngleMod);
        pl.move(movementRatio);
        pl.rotate(-movementAngleMod);
    }

    if (keyFlags[' '] == true) {
        pl.jump();
        keyFlags[' '] = false;
    }
}