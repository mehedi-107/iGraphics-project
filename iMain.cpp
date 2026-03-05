#include "iGraphics.h"
#include <windows.h>
#include <cstdlib>
#include <cstdio>
#include <mmsystem.h>

// --- Constants ---
const int SCREEN_WIDTH  = 1900;
const int SCREEN_HEIGHT = 950;
const int MAZE_X        = 500;
const int MAZE_Y        = 150;
const int MAZE_PIXEL    = 40;
const int PAC_MOVE_DELAY = 15;
const int FOOD_SPACING  = 40;
const int LIFE_ICON_SPACING = 40;
const int FOOD_OFFSCREEN = 100000;

// Direction enum for readable key/movement handling
enum Direction { DIR_NONE = 0, DIR_UP = 1, DIR_DOWN = 2, DIR_RIGHT = 3, DIR_LEFT = 4 };

// --- Game State ---
bool musicEnabled   = true;
int  isPlaying      = 0;
int  pacPosX        = 540;
int  pacPosY        = 190;
int  moveSpeed      = 2;
int  showIntro      = 1;
int  showMenu       = 0;
int  pacSpriteIndex = 0;
int  livesRemaining = 3;
int  moveDirX       = 1;
int  moveDirY       = 0;
int  lastKeyPress   = 0;
int  score          = 0;
int  needsFoodInit  = 1;
int  timeCount      = 0;

// String buffers for HUD display
char scoreText[10];
char minuteText[5];
char secondText[5];

// --- Asset Paths ---
char introPic[20]       = "pic1\\intro.bmp";
char menuPic[20]        = "pic1\\option.bmp";
char playPic[20]        = "pic1\\play.bmp";
char pacSprites[4][20]  = { "pic1\\pacr.bmp", "pic1\\pacl.bmp", "pic1\\pacu.bmp", "pic1\\pacd.bmp" };
char lifePic[20]        = { "pic1\\life.bmp" };
char foodPic[1][20]     = { "pic1\\f0.bmp" };

// --- Food ---
struct Food {
    int x = 600;
    int y = 206;
};

// Food arrays for each maze ring (outer=1, mid=2, inner=3) and side (Down/Right/Up/Left)
Food food1Down[13], food1Right[13], food1Up[14], food1Left[13];
Food food2Down[11], food2Right[10], food2Up[10], food2Left[9];
Food food3Down[7],  food3Right[6],  food3Left[7], food3Up[7];

// --- Helper Functions ---

// Initialize a row/column of food pellets from a starting position
void initFoodRow(Food* arr, int count, int startX, int startY, bool horizontal) {
    arr[0].x = startX;
    arr[0].y = startY;
    for (int i = 1; i < count; i++) {
        arr[i].x = horizontal ? arr[i - 1].x + FOOD_SPACING : arr[0].x;
        arr[i].y = horizontal ? arr[0].y : arr[i - 1].y + FOOD_SPACING;
    }
}

// Check collision between pac-man and a food row, consuming food on contact
void checkFoodCollision(Food* arr, int count) {
    for (int i = 0; i < count; i++) {
        if (abs(pacPosX - arr[i].x) <= 20 && abs(pacPosY - arr[i].y) <= 20) {
            arr[i].x = FOOD_OFFSCREEN;
            arr[i].y = FOOD_OFFSCREEN;
            score += 10;
        }
    }
}

// Draw all food pellets in a row
void drawFoodRow(Food* arr, int count) {
    for (int i = 0; i < count; i++) {
        iShowBMP2(arr[i].x, arr[i].y, foodPic[0], 0);
    }
}

// Apply wall constraint: resets direction, then allows only the specified directions
void applyWallConstraint(bool allowUp, bool allowDown, bool allowRight, bool allowLeft) {
    moveDirX = 0;
    moveDirY = 0;
    switch (lastKeyPress) {
        case DIR_UP:    if (allowUp)    moveDirY =  1; break;
        case DIR_DOWN:  if (allowDown)  moveDirY = -1; break;
        case DIR_RIGHT: if (allowRight) moveDirX =  1; break;
        case DIR_LEFT:  if (allowLeft)  moveDirX = -1; break;
    }
}

// Free movement at hole intersections (does NOT reset direction first)
void applyFreeMovement() {
    switch (lastKeyPress) {
        case DIR_UP:    moveDirY =  1; break;
        case DIR_DOWN:  moveDirY = -1; break;
        case DIR_RIGHT: moveDirX =  1; break;
        case DIR_LEFT:  moveDirX = -1; break;
    }
}

// --- Food Position Initialization ---
void foodpos() {
    if (!needsFoodInit) return;

    // Ring 1 (outermost)
    initFoodRow(food1Down,  13, 600,  206, true);
    initFoodRow(food1Right, 13, 1108, 238, false);
    initFoodRow(food1Up,    14, 562,  758, true);
    initFoodRow(food1Left,  13, 556,  238, false);

    // Ring 2
    initFoodRow(food2Down,  11, 632,  278, true);
    initFoodRow(food2Right, 10, 1038, 322, false);
    initFoodRow(food2Up,    10, 624,  688, true);
    initFoodRow(food2Left,   9, 624,  322, false);

    // Ring 3 (innermost)
    initFoodRow(food3Down,   7, 726,  348, true);
    initFoodRow(food3Right,  6, 968,  386, false);
    initFoodRow(food3Up,     7, 722,  618, true);
    initFoodRow(food3Left,   7, 692,  352, false);
}

// --- Draw ---
void iDraw() {
    iClear();

    // Intro screen
    if (showIntro == 1) {
        iShowBMP(0, 0, introPic);
        iSetColor(0, 255, 255);
        iText(800, 150, "Press S key to start the game!", GLUT_BITMAP_TIMES_ROMAN_24);
    }

    // Menu screen
    if (showIntro == 0 && showMenu == 1 && isPlaying == 0) {
        iShowBMP(0, 0, menuPic);

        iSetColor(255, 0, 255);
        iFilledRectangle(950, 500, 30, 30);
        iSetColor(0, 255, 255);
        iText(1000, 505, "PLAY", GLUT_BITMAP_TIMES_ROMAN_24);

        iSetColor(255, 0, 255);
        iFilledRectangle(950, 425, 30, 30);
        iSetColor(0, 255, 255);
        iText(1000, 430, "HIGH SCORE", GLUT_BITMAP_TIMES_ROMAN_24);

        iSetColor(255, 0, 255);
        iFilledRectangle(950, 350, 30, 30);
        iSetColor(0, 255, 255);
        iText(1000, 355, "HIGH SCORE", GLUT_BITMAP_TIMES_ROMAN_24);
    }

    // Gameplay screen
    if (showIntro == 0 && showMenu == 0 && isPlaying == 1) {
        foodpos();

        // Draw maze walls (all cyan)
        iSetColor(0, 255, 255);

        // Outer ring (M1)
        iFilledRectangle(500, 150, 315, 40);   // m1 bottom-left
        iFilledRectangle(500, 150, 40, 315);    // m1 left-bottom
        iFilledRectangle(865, 790, 315, 40);    // m1 top-right
        iFilledRectangle(865, 150, 315, 40);    // m1 bottom-right
        iFilledRectangle(500, 515, 40, 315);    // m1 left-top
        iFilledRectangle(1140, 150, 40, 315);   // m1 right-bottom
        iFilledRectangle(1140, 515, 40, 315);   // m1 right-top
        iFilledRectangle(500, 790, 315, 40);    // m1 top-left

        // Second ring (M2)
        iFilledRectangle(590, 240, 390, 20);    // m2 bottom-1
        iFilledRectangle(1030, 240, 60, 20);    // m2 bottom-2
        iFilledRectangle(590, 310, 20, 350);    // m2 left-1
        iFilledRectangle(590, 710, 20, 30);     // m2 left-2
        iFilledRectangle(590, 720, 430, 20);    // m2 top
        iFilledRectangle(1070, 240, 20, 500);   // m2 right

        // Third ring (M3)
        iFilledRectangle(660, 310, 155, 20);    // m3 bottom-1
        iFilledRectangle(865, 310, 155, 20);    // m3 bottom-2
        iFilledRectangle(660, 310, 20, 155);    // m3 left-1
        iFilledRectangle(660, 515, 20, 155);    // m3 left-2
        iFilledRectangle(660, 650, 155, 20);    // m3 top-1
        iFilledRectangle(865, 650, 155, 20);    // m3 top-2
        iFilledRectangle(1000, 310, 20, 155);   // m3 right-1
        iFilledRectangle(1000, 515, 20, 155);   // m3 right-2

        // Inner box (M4) and center block
        iFilledRectangle(730, 380, 220, 20);    // m4 bottom
        iFilledRectangle(930, 380, 20, 220);    // m4 right
        iFilledRectangle(730, 380, 20, 220);    // m4 left
        iFilledRectangle(800, 580, 80, 20);     // m4 top (partial)
        iFilledRectangle(800, 450, 80, 80);     // center block

        // Draw lives
        for (int k = 0; k < livesRemaining; k++) {
            iShowBMP2(1560 + k * LIFE_ICON_SPACING, 550, lifePic, 0);
        }

        // Draw food pellets
        drawFoodRow(food1Down,  13);
        drawFoodRow(food1Right, 13);
        drawFoodRow(food1Up,    14);
        drawFoodRow(food1Left,  13);
        drawFoodRow(food2Down,  11);
        drawFoodRow(food2Right, 10);
        drawFoodRow(food2Up,    10);
        drawFoodRow(food2Left,   9);
        drawFoodRow(food3Down,   7);
        drawFoodRow(food3Right,  6);
        drawFoodRow(food3Up,     7);
        drawFoodRow(food3Left,   7);

        // Draw pac-man
        iShowBMP2(pacPosX, pacPosY, pacSprites[pacSpriteIndex], 0);

        // Draw HUD
        iSetColor(255, 255, 255);
        iText(1560, 827, "SCORE", GLUT_BITMAP_HELVETICA_18);
        iText(1600, 800, scoreText, GLUT_BITMAP_HELVETICA_18);
        iText(235, 827, "TIME", GLUT_BITMAP_HELVETICA_18);
        iText(240, 800, minuteText, GLUT_BITMAP_HELVETICA_18);
        iText(258, 800, ":", GLUT_BITMAP_HELVETICA_18);
        iText(268, 800, secondText, GLUT_BITMAP_HELVETICA_18);
    }
}

// --- Timer callback (called every 1 second) ---
void timer() {
    if (!showIntro && !showMenu) {
        timeCount++;
        int min = timeCount / 60;
        int sec = timeCount - min * 60;
        snprintf(minuteText, sizeof(minuteText), "%d", min);
        snprintf(secondText, sizeof(secondText), "%d", sec);
    }
}

// --- Food collision (called each frame) ---
void foodfunc() {
    checkFoodCollision(food1Down,  13);
    checkFoodCollision(food1Right, 13);
    checkFoodCollision(food1Up,    14);
    checkFoodCollision(food1Left,  13);
    checkFoodCollision(food2Down,  11);
    checkFoodCollision(food2Up,    10);
    checkFoodCollision(food2Left,   9);
    checkFoodCollision(food2Right, 10);
    checkFoodCollision(food3Down,   7);
    checkFoodCollision(food3Up,     7);
    checkFoodCollision(food3Left,   7);
    checkFoodCollision(food3Right,  6);

    snprintf(scoreText, sizeof(scoreText), "%d", score);
}

// --- Pac-man movement and wall collision ---
void pacmove() {

    // === Hole Pass-Through (teleportation) ===
    if (pacPosX > 815 && pacPosX < 825 && pacPosY < 150) {  // bottom
        pacPosX = 820;
        pacPosY = 790;
    }
    if (pacPosX > 815 && pacPosX < 825 && pacPosY > 790) {  // top
        pacPosX = 820;
        pacPosY = 150;
    }
    if (pacPosX < 500 && pacPosY > 465 && pacPosY < 475) {  // left
        pacPosX = 1140;
        pacPosY = 470;
    }
    if (pacPosX > 1140 && pacPosY > 465 && pacPosY < 475) { // right
        pacPosX = 500;
        pacPosY = 470;
    }

    // === M1 Hole Passage Constraints ===
    // Cancel up move through m1L/m1R hole
    if (pacPosY == 474 && ((pacPosX >= 460 && pacPosX <= 538) || (pacPosX >= 1102 && pacPosX <= 1180)))
        applyWallConstraint(false, true, true, true);

    // Cancel down move through m1L/m1R hole
    if (pacPosY == 470 && ((pacPosX >= 500 && pacPosX <= 538) || (pacPosX >= 1102 && pacPosX <= 1140)))
        applyWallConstraint(true, false, true, true);

    // Cancel left move through m1U/m1D hole
    if (pacPosX == 816 && ((pacPosY >= 150 && pacPosY <= 188) || (pacPosY >= 752 && pacPosY <= 790)))
        applyWallConstraint(true, true, true, false);

    // Cancel right move through m1U/m1D hole
    if (pacPosX == 824 && ((pacPosY >= 150 && pacPosY <= 188) || (pacPosY >= 752 && pacPosY <= 790)))
        applyWallConstraint(true, true, false, true);

    // === M1 Walls (except corners) ===
    if (pacPosX == 1100 && pacPosY != 750 && pacPosY != 190 && !(pacPosY >= 466 && pacPosY <= 474))
        applyWallConstraint(true, true, false, true);   // m1R: block right

    if (pacPosX == 540 && pacPosY != 750 && pacPosY != 190 && !(pacPosY >= 466 && pacPosY <= 474))
        applyWallConstraint(true, true, true, false);    // m1L: block left

    if (pacPosY == 750 && pacPosX != 1100 && pacPosX != 540 && !(pacPosX >= 816 && pacPosX <= 824))
        applyWallConstraint(false, true, true, true);    // m1U: block up

    // Free movement at m1 hole intersections (U/D holes at corners)
    if ((pacPosX >= 816 && pacPosX <= 824) && (pacPosY == 190 || pacPosY == 750))
        applyFreeMovement();

    // Free movement at m1 hole intersections (L/R holes at corners)
    if ((pacPosY >= 466 && pacPosY <= 474) && (pacPosX == 1100 || pacPosX == 540))
        applyFreeMovement();

    // m1D (note: original code uses pacPosY != 540, preserving as-is)
    if (pacPosY == 190 && pacPosX != 1100 && pacPosY != 540 && !(pacPosX >= 816 && pacPosX <= 824))
        applyWallConstraint(true, false, true, true);    // block down

    // === M1 Corners ===
    if (pacPosX == 1100 && pacPosY == 750)
        applyWallConstraint(false, true, false, true);   // top-right

    if (pacPosX == 1100 && pacPosY == 190)
        applyWallConstraint(true, false, false, true);   // bottom-right (c2)

    if (pacPosX == 540 && pacPosY == 750)
        applyWallConstraint(false, true, true, false);   // top-left (c4)

    if (pacPosX == 540 && pacPosY == 190)
        applyWallConstraint(true, false, true, false);   // bottom-left (c1)

    if (pacPosY == 760 && pacPosX == 1100)
        applyWallConstraint(false, true, true, true);    // c3

    // === M2 Walls ===
    // m2L outside
    if (pacPosX == 550 && !(pacPosY >= 190 && pacPosY <= 200) && !(pacPosY >= 260 && pacPosY <= 270)
        && !(pacPosY >= 660 && pacPosY <= 670) && !(pacPosY >= 740 && pacPosY <= 750))
        applyWallConstraint(true, true, false, true);    // block right

    // m2U outside
    if (pacPosY == 740 && !(pacPosX >= 540 && pacPosX <= 550) && !(pacPosX >= 1020 && pacPosX <= 1030)
        && !(pacPosX >= 1090 && pacPosX <= 1100))
        applyWallConstraint(true, false, true, true);    // block down

    // m2R outside
    if (pacPosX == 1090 && !(pacPosY >= 190 && pacPosY <= 200) && !(pacPosY >= 740 && pacPosY <= 750))
        applyWallConstraint(true, true, true, false);    // block left

    // m2D outside
    if (pacPosY == 200 && !(pacPosX >= 540 && pacPosX <= 548) && pacPosX != 600
        && !(pacPosX >= 980 && pacPosX <= 988) && pacPosX != 700 && !(pacPosX >= 1090 && pacPosX <= 1100))
        applyWallConstraint(false, true, true, true);    // block up

    // m2D inside
    if (pacPosY == 260 && pacPosX != 1030 && !(pacPosX >= 540 && pacPosX <= 550)
        && !(pacPosX >= 980 && pacPosX <= 990) && !(pacPosX >= 1090 && pacPosX <= 1100))
        applyWallConstraint(true, false, true, true);    // block down

    // m2R inside (except corner)
    if (pacPosX == 1030 && pacPosY != 260 && !(pacPosY >= 740 && pacPosY <= 750)
        && !(pacPosY >= 190 && pacPosY <= 200))
        applyWallConstraint(true, true, false, true);    // block right

    // m2D-m2R corner
    if (pacPosX == 1030 && pacPosY == 260)
        applyWallConstraint(true, false, false, true);   // block down+right

    // Right side of m2D hole
    if (pacPosX == 990 && (pacPosY >= 202 && pacPosY <= 258))
        applyWallConstraint(true, true, false, true);    // block right

    // Left side of m2D hole
    if (pacPosX == 980 && (pacPosY >= 202 && pacPosY <= 258))
        applyWallConstraint(true, true, true, false);    // block left

    // m2U inside (except corner)
    if (pacPosY == 680 && pacPosX != 610 && !(pacPosX >= 540 && pacPosX <= 550)
        && !(pacPosX >= 1020 && pacPosX <= 1030) && !(pacPosX >= 1090 && pacPosX <= 1100))
        applyWallConstraint(false, true, true, true);    // block up

    // m2L inside (except corner)
    if (pacPosX == 610 && pacPosY != 680 && !(pacPosY >= 190 && pacPosY <= 200)
        && !(pacPosY >= 260 && pacPosY <= 270) && !(pacPosY >= 660 && pacPosY <= 670)
        && !(pacPosY >= 740 && pacPosY <= 750))
        applyWallConstraint(true, true, true, false);    // block left

    // m2U-m2L corner
    if (pacPosX == 610 && pacPosY == 680)
        applyWallConstraint(false, true, true, false);   // block up+left

    // m2L down hole
    if (pacPosY == 270 && (pacPosX >= 552 && pacPosX <= 608))
        applyWallConstraint(false, true, true, true);    // block up

    // m2L upper hole (cancel down)
    if (pacPosY == 660 && (pacPosX >= 552 && pacPosX <= 608))
        applyWallConstraint(true, false, true, true);    // block down

    // m2L upper hole (cancel up)
    if (pacPosY == 670 && (pacPosX >= 552 && pacPosX <= 608))
        applyWallConstraint(false, true, true, true);    // block up

    // m2U hole (cancel left)
    if (pacPosX == 1020 && (pacPosY >= 682 && pacPosY <= 738))
        applyWallConstraint(true, true, true, false);    // block left

    // === M3 Walls ===
    // m3R inside
    if (pacPosX == 960 && pacPosY != 330 && pacPosY != 620 && !(pacPosY >= 466 && pacPosY <= 474)
        && !(pacPosY >= 190 && pacPosY <= 300) && !(pacPosY >= 670 && pacPosY <= 760))
        applyWallConstraint(true, true, false, true);    // block right

    // m3L inside
    if (pacPosX == 680 && pacPosY != 330 && pacPosY != 620 && !(pacPosY >= 466 && pacPosY <= 474)
        && !(pacPosY >= 190 && pacPosY <= 300) && !(pacPosY >= 670 && pacPosY <= 760))
        applyWallConstraint(true, true, true, false);    // block left

    // m3U inside
    if (pacPosY == 614 && pacPosX != 680 && pacPosX != 960 && !(pacPosX >= 816 && pacPosX <= 824)
        && !(pacPosX >= 540 && pacPosX <= 630) && !(pacPosX >= 1020 && pacPosX <= 1110))
        applyWallConstraint(false, true, true, true);    // block up

    // m3D inside
    if (pacPosY == 330 && pacPosX != 680 && pacPosX != 960 && !(pacPosX >= 816 && pacPosX <= 824)
        && !(pacPosX >= 540 && pacPosX <= 630) && !(pacPosX >= 1020 && pacPosX <= 1110))
        applyWallConstraint(true, false, true, true);    // block down

    // M3 corners
    if (pacPosX == 960 && pacPosY == 614)
        applyWallConstraint(false, true, false, true);   // c3: block up+right

    if (pacPosX == 960 && pacPosY == 330)
        applyWallConstraint(true, false, false, true);   // c2: block down+right

    if (pacPosX == 680 && pacPosY == 614)
        applyWallConstraint(false, true, true, false);   // c4: block up+left

    if (pacPosX == 680 && pacPosY == 330)
        applyWallConstraint(true, false, true, false);   // c1: block down+left

    if (pacPosY == 760 && pacPosX == 1100)
        applyWallConstraint(false, true, true, true);    // c3 overlap

    // m3R outside
    if (pacPosX == 1020 && pacPosY != 330 && pacPosY != 620 && !(pacPosY >= 466 && pacPosY <= 474)
        && !(pacPosY >= 190 && pacPosY <= 280) && !(pacPosY >= 670 && pacPosY <= 760))
        applyWallConstraint(true, true, true, false);    // block left

    // m3L outside
    if (pacPosX == 624 && pacPosY != 330 && pacPosY != 620 && !(pacPosY >= 466 && pacPosY <= 474)
        && !(pacPosY >= 190 && pacPosY <= 282) && !(pacPosY >= 670 && pacPosY <= 760))
        applyWallConstraint(true, true, false, true);    // block right

    // m3D outside
    if (pacPosY == 274 && pacPosX != 680 && pacPosX != 960 && !(pacPosX >= 816 && pacPosX <= 830)
        && !(pacPosX >= 540 && pacPosX <= 630) && !(pacPosX >= 1020 && pacPosX <= 1110))
        applyWallConstraint(false, true, true, true);    // block up

    // m3U outside
    if (pacPosY == 670 && pacPosX != 680 && pacPosX != 960 && !(pacPosX >= 816 && pacPosX <= 830)
        && !(pacPosX >= 540 && pacPosX <= 626) && !(pacPosX >= 1020 && pacPosX <= 1110))
        applyWallConstraint(true, false, true, true);    // block down

    // M3 hole passage constraints
    if (pacPosY == 476 && ((pacPosX >= 628 && pacPosX <= 678) || (pacPosX >= 962 && pacPosX <= 1018)))
        applyWallConstraint(false, true, true, true);    // cancel up

    if (pacPosY == 468 && ((pacPosX >= 628 && pacPosX <= 678) || (pacPosX >= 962 && pacPosX <= 1018)))
        applyWallConstraint(true, false, true, true);    // cancel down

    if (pacPosX == 816 && ((pacPosY >= 276 && pacPosY <= 328) || (pacPosY >= 618 && pacPosY <= 668)))
        applyWallConstraint(true, true, true, false);    // cancel left

    if (pacPosX == 828 && ((pacPosY >= 276 && pacPosY <= 328) || (pacPosY >= 618 && pacPosY <= 668)))
        applyWallConstraint(true, true, false, true);    // cancel right

    // === M4 Walls ===
    // m4R inside
    if (pacPosX == 894 && pacPosY != 380 && pacPosY != 620 && !(pacPosY >= 190 && pacPosY <= 300)
        && !(pacPosY >= 670 && pacPosY <= 760) && !(pacPosY >= 330 && pacPosY <= 346)
        && !(pacPosY >= 600 && pacPosY <= 624))
        applyWallConstraint(true, true, false, true);    // block right

    // m4L inside
    if (pacPosX == 750 && pacPosY != 380 && pacPosY != 620 && !(pacPosY >= 190 && pacPosY <= 300)
        && !(pacPosY >= 670 && pacPosY <= 760) && !(pacPosY >= 330 && pacPosY <= 346)
        && !(pacPosY >= 600 && pacPosY <= 624))
        applyWallConstraint(true, true, true, false);    // block left

    // m4L outside
    if (pacPosX == 694 && pacPosY != 380 && pacPosY != 620 && !(pacPosY >= 190 && pacPosY <= 300)
        && !(pacPosY >= 670 && pacPosY <= 760) && !(pacPosY >= 330 && pacPosY <= 346)
        && !(pacPosY >= 600 && pacPosY <= 624))
        applyWallConstraint(true, true, false, true);    // block right

    // m4R outside
    if (pacPosX == 950 && pacPosY != 380 && pacPosY != 620 && !(pacPosY >= 190 && pacPosY <= 300)
        && !(pacPosY >= 670 && pacPosY <= 760) && !(pacPosY >= 330 && pacPosY <= 346)
        && !(pacPosY >= 600 && pacPosY <= 624))
        applyWallConstraint(true, true, true, false);    // block left

    // m4D
    if (pacPosY == 400 && !(pacPosX >= 540 && pacPosX <= 630) && !(pacPosX >= 1020 && pacPosX <= 1110)
        && !(pacPosX >= 680 && pacPosX <= 696) && !(pacPosX >= 950 && pacPosX <= 966))
        applyWallConstraint(true, false, true, true);    // block down

    // M4 corners
    if (pacPosX == 750 && pacPosY == 400)
        applyWallConstraint(true, false, true, false);   // c1: block down+left

    if (pacPosX == 894 && pacPosY == 400)
        applyWallConstraint(true, false, false, true);   // c2: block down+right

    // m4D outside
    if (pacPosY == 344 && !(pacPosX >= 540 && pacPosX <= 630) && !(pacPosX >= 1020 && pacPosX <= 1110)
        && !(pacPosX >= 680 && pacPosX <= 696) && !(pacPosX >= 950 && pacPosX <= 966))
        applyWallConstraint(false, true, true, true);    // block up

    // Cancel down through m4 border area
    if (pacPosY == 600 && ((pacPosX >= 698 && pacPosX <= 748) || (pacPosX >= 898 && pacPosX <= 948)))
        applyWallConstraint(true, false, true, true);    // block down

    // === Inner Box Walls ===
    // Inner box bottom
    if (pacPosY == 600 && (pacPosX >= 768 && pacPosX <= 878))
        applyWallConstraint(true, false, true, true);    // block down

    // Inner box top
    if (pacPosY == 544 && (pacPosX >= 768 && pacPosX <= 878))
        applyWallConstraint(false, true, true, true);    // block up

    // Inner box right (top section)
    if (pacPosX == 880 && (pacPosY >= 548 && pacPosY <= 598))
        applyWallConstraint(true, true, true, false);    // block left

    // Inner box left (top section)
    if (pacPosX == 760 && (pacPosY >= 548 && pacPosY <= 598))
        applyWallConstraint(true, true, false, true);    // block right

    // Center block top
    if (pacPosY == 530 && pacPosX > 762 && pacPosX < 880)
        applyWallConstraint(true, false, true, true);    // block down

    // Center block left
    if (pacPosX == 762 && pacPosY > 416 && pacPosY < 530)
        applyWallConstraint(true, true, false, true);    // block right

    // Center block right
    if (pacPosX == 880 && pacPosY > 416 && pacPosY < 530)
        applyWallConstraint(true, true, true, false);    // block left

    // Center block bottom
    if (pacPosY == 414 && pacPosX > 762 && pacPosX < 880)
        applyWallConstraint(false, true, true, true);    // block up

    // === Apply Final Movement ===
    if (moveDirX == 1)
        pacPosX += moveSpeed;
    else if (moveDirX == -1)
        pacPosX -= moveSpeed;
    else if (moveDirY == 1)
        pacPosY += moveSpeed;
    else if (moveDirY == -1)
        pacPosY -= moveSpeed;
}

// --- Mouse move callback ---
void iMouseMove(int mx, int my) {
    printf("x = %d, y= %d\n", mx, my);
}

// --- Mouse click callback ---
void iMouse(int button, int state, int mx, int my) {
    if (button == GLUT_LEFT_BUTTON && showIntro == 0 && showMenu == 1
        && mx >= 765 && mx <= 880 && my >= 570 && my <= 600) {
        showMenu = 0;
        isPlaying = 1;
    }
}

// --- Keyboard callback ---
void iKeyboard(unsigned char key) {
    if (key == 's') {
        showIntro = 0;
        showMenu = 1;
    }
}

// --- Special keyboard callback (arrow keys) ---
void iSpecialKeyboard(unsigned char key) {
    if (key == GLUT_KEY_UP) {
        lastKeyPress = DIR_UP;
        moveDirY = 1;
        moveDirX = 0;
        pacSpriteIndex = 2;
        needsFoodInit = 0;
    }
    if (key == GLUT_KEY_DOWN) {
        lastKeyPress = DIR_DOWN;
        moveDirY = -1;
        moveDirX = 0;
        pacSpriteIndex = 3;
    }
    if (key == GLUT_KEY_LEFT) {
        lastKeyPress = DIR_LEFT;
        moveDirX = -1;
        moveDirY = 0;
        pacSpriteIndex = 1;
    }
    if (key == GLUT_KEY_RIGHT) {
        lastKeyPress = DIR_RIGHT;
        moveDirX = 1;
        moveDirY = 0;
        pacSpriteIndex = 0;
        needsFoodInit = 0;
    }
}

// --- Main ---
int main() {
    foodpos();
    iSetTimer(1000, timer);
    iSetTimer(PAC_MOVE_DELAY, foodfunc);
    iSetTimer(PAC_MOVE_DELAY, pacmove);

    if (musicEnabled) {
        PlaySound("intro.wav", NULL, SND_LOOP | SND_ASYNC);
    }

    iInitialize(SCREEN_WIDTH, SCREEN_HEIGHT, "PacMan");
    return 0;
}
