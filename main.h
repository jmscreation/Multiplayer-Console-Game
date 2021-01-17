#ifndef __MAIN_H__
#define __MAIN_H__

#include "multiplayer.h"
#include "console_library.h"

#define GAME_SIZE_W     40
#define GAME_SIZE_H     40

#define FONT_WIDTH      8
#define FONT_HEIGHT     8

#define BACK_CHAR       ' '
#define PLAYER_CHAR     '0'
#define PLAYER_SCHAR    '@'
#define BORDER_CHAR     'X'
#define DWALL_CHAR      'x'
#define POINT_CHAR      '$'

class MyGame : public Game {

    Collidable* user;
    int score;
    bool gameOver;

    Multiplayer mplay;

    Clock keyTimer, indexTimer, refreshTimer, sendClock;
public:
    MyGame(short width, short height);
    virtual ~MyGame();

    virtual bool update();
};

#endif // __MAIN_H__
