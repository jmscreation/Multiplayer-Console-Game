#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <string>
#include <vector>
#include <assert.h>
#include <chrono>
#include "windows.h"

typedef std::chrono::time_point<std::chrono::high_resolution_clock> timepoint;

class Clock {
    timepoint start;
public:
    Clock();
    virtual ~Clock();
    double getSeconds();
    double getMilliseconds();
    inline void restart() { start = std::chrono::high_resolution_clock::now(); }
};

class Draw {
    HANDLE console;
    CONSOLE_FONT_INFOEX font;
    COORD fontSize;
    short width, height;
    char backChar;
public:
    Draw(const char backChar=' ');
    virtual ~Draw();

    inline COORD getScreenSize() { updateWindowSize(); return (COORD){width, height}; }
    inline COORD getFontSize(){ updateWindowFont(); return fontSize; }

    void drawText(short x, short y, const char* str);
    void drawChar(short x, short y, char c);
    void drawRemove(short x, short y);
    bool setFont(short w, short h);
    bool setSize(short x, short y);
    void setWindowResizeable(bool allowed);
    void drawClear();
private:
    void updateWindowSize();
    void updateWindowFont();

};

class Input {
    static std::vector<int> keys;
public:
    Input();
    virtual ~Input();

    bool hasFocus();

    int getDirection();
    bool getSpace();
    bool anyKey();
    void waitAnykey(float seconds=1.);
};


class Object {
protected:
    Draw* screen;
    short x, y;
    char myChar;
    size_t index;
    static std::vector<Object*> objects;
public:
    Object(short x, short y, char myChar);
    virtual ~Object();

    inline short X() { return x; }
    inline short Y() { return y; }
    inline char getChar() { return myChar; }
    inline void setChar(char c) { myChar = c; screen->drawChar(x, y, myChar); }
    inline void refresh() { screen->drawChar(x, y, myChar); }

    static void clearObjects();
    static void refreshObjects();

    void updatePos(short _x, short _y);
};

class Collidable : public Object {
protected:
    static std::vector<Collidable*> collidables;
    size_t cindex;
public:
    Collidable(short x, short y, char myChar);
    virtual ~Collidable();

    static Collidable* checkCollision(short x, short y);
    static void clearCollidables();
    static inline const std::vector<Collidable*>& getCollidables() { return collidables; }
};


class Game {
    short width, height;
    COORD fontSize;
    bool _gameRunning;
    static Game* curGame;
    static Draw* screen;
    static Input* input;
public:

    const bool& gameRunning;

    inline Draw& curScreen() { return *screen; }
    inline Input& curInput() { return *input; }

    static inline Game& currentGame() { assert(curGame != nullptr); return *curGame; }
    Game(short width, short height, char backChar, COORD fontsz);
    virtual ~Game();

    virtual bool update()=0;
    void runGame(unsigned int msPerTick=60);
};

#endif // __CONSOLE_H__
