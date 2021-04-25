#include "console_library.h"

using std::max, std::min,
      std::string, std::vector;


Clock::Clock(){
    restart();
}
Clock::~Clock(){}

double Clock::getSeconds() {
    return getMilliseconds() / 1000.0;
}

double Clock::getMilliseconds() {
    return double(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000.0;
}



Draw::Draw(const char backChar): backChar(backChar) {
    console = GetStdHandle(STD_OUTPUT_HANDLE);

    updateWindowFont();
    updateWindowSize();

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(console, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(console, &cursorInfo); // no blinking cursor

    DWORD dwMode;
    GetConsoleMode(console, &dwMode);
    dwMode |= DISABLE_NEWLINE_AUTO_RETURN; // no new line at end
    SetConsoleMode(console, dwMode);

}


Draw::~Draw() {}

void Draw::updateWindowSize(){
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(console, &info);
    width = info.dwSize.X; // update console buffer size
    height = info.dwSize.Y;
}

void Draw::updateWindowFont(){
    GetCurrentConsoleFontEx(console, FALSE, &font); // get font info
    fontSize = font.dwFontSize; // update console font size
}

bool Draw::setFont(short w, short h) {
    fontSize = {w, h};
    font.cbSize = sizeof(font);
    font.nFont = 0;
    wcscpy(font.FaceName, L"Raster Font");
    font.FontFamily = FF_DONTCARE;
    font.FontWeight = FW_NORMAL;
    font.dwFontSize = fontSize;
    if(!SetCurrentConsoleFontEx(console, false, &font)) return false;
    return true;
}

bool Draw::setSize(short x, short y) {
    COORD mxsize = GetLargestConsoleWindowSize(console),
          mnsize = { short(GetSystemMetrics(SM_CXMIN) / fontSize.X),
                     short(GetSystemMetrics(SM_CYMIN) / fontSize.Y) };
    x = max(min(x, short(mxsize.X-1)), short(mnsize.X+1));
    y = max(min(y, short(mxsize.Y-1)), short(mnsize.Y+1));

    if((x < width) ^ (y < height)){
        SetConsoleScreenBufferSize(console, mxsize);
        width = mxsize.X;
        height = mxsize.Y;
    }

    COORD size = {x, y};
    SMALL_RECT win = { 0, 0, short(size.X-1), short(size.Y-1) };
    if(x < width && y < height) SetConsoleWindowInfo(console, TRUE, (const SMALL_RECT*) &win);
    SetConsoleScreenBufferSize(console, size);
    if(x >= width || y >= height) SetConsoleWindowInfo(console, TRUE, (const SMALL_RECT*) &win);

    if(GetLastError() == 0){
        width = x;
        height = y;
        return true;
    } else {
        return false;
    }
}

void Draw::drawClear() {
    DWORD out;
    FillConsoleOutputCharacter(console, backChar, width * height, {0,0}, &out);
}

void Draw::setWindowResizeable(bool allowed) {
    HWND consoleWindow = GetConsoleWindow();
    LONG flags = GetWindowLong(consoleWindow, GWL_STYLE);
    if(allowed){
        flags |= WS_MAXIMIZEBOX;
        flags |= WS_SIZEBOX;
    } else {
         flags &= ~WS_MAXIMIZEBOX;
         flags &= ~WS_SIZEBOX;
    }
    SetWindowLong(consoleWindow, GWL_STYLE, flags);
}

void Draw::drawChar(short x, short y, char c) {
    if(x >= width || x < 0 || y >= height || y < 0) return;
    DWORD out;
    WriteConsoleOutputCharacter(console, (const char*)&c, 1, {x, y}, &out);
}

void Draw::drawRemove(short x, short y) {
    if(x >= width || x < 0 || y >= height || y < 0) return;
    DWORD out;
    WriteConsoleOutputCharacter(console, (const char*)&backChar, 1, {x, y}, &out);
}

void Draw::drawText(short x, short y, const char* str) {
    if(x >= width || x < 0 || y >= height || y < 0) return;
    DWORD out;
    WriteConsoleOutputCharacter(console, str, strlen(str), {x, y}, &out);
}


Input::Input() {}
Input::~Input() {}

bool Input::hasFocus() {
    return GetForegroundWindow() == GetConsoleWindow();
}

int Input::getDirection() {
    if(!hasFocus()) return 0;
    if(GetAsyncKeyState(VK_RIGHT) & 0x8000)  return 1;
    if(GetAsyncKeyState(VK_UP) & 0x8000)     return 2;
    if(GetAsyncKeyState(VK_LEFT) & 0x8000)   return 3;
    if(GetAsyncKeyState(VK_DOWN) & 0x8000)   return 4;
    return 0;
}

bool Input::getSpace() {
    if(!hasFocus()) return false;
    return GetAsyncKeyState(VK_SPACE) & 0x8000;
}

bool Input::anyKey() {
    if(!hasFocus()) return false;
    for(int i=0x00; i < 0xFF; ++i) if(i != VK_RBUTTON && i != VK_LBUTTON && i != VK_MBUTTON && GetAsyncKeyState(i) & 0x8000) return true;
    return false;
}

void Input::waitAnykey(float seconds) {
    Sleep(DWORD(1000 * seconds));
    do {
        if(anyKey()) return;
    } while(1);
}


std::vector<Object*> Object::objects;

Object::Object(short x, short y, char myChar): screen(&Game::currentGame().curScreen()), x(x), y(y), myChar(myChar) {
    index = objects.size();
    objects.push_back(this);
    screen->drawChar(x, y, myChar);
}
Object::~Object() {
    objects[index] = nullptr;
    screen->drawRemove(x, y);
}

void Object::updatePos(short _x, short _y) {
    screen->drawRemove(x, y);
    x = _x;
    y = _y;
    screen->drawChar(x, y, myChar);
}

void Object::clearObjects() {
    for(Object* obj : objects) if(obj != nullptr) delete obj;
    objects.clear();
}

void Object::refreshObjects() {
    for(Object* obj : objects) if(obj != nullptr) obj->refresh();
}


vector<Collidable*> Collidable::collidables;

Collidable::Collidable(short x, short y, char myChar) : Object(x, y, myChar) {
    cindex = collidables.size();
    collidables.push_back(this);
}

Collidable::~Collidable() {
    collidables[cindex] = nullptr;
}

Collidable* Collidable::checkCollision(short x, short y){
    for(Collidable* obj : collidables){
        if(obj == nullptr) continue;
        if(x == obj->x && y == obj->y) return obj;
    }
    return nullptr;
}

void Collidable::clearCollidables() {
    for(Collidable* obj : collidables) if(obj != nullptr) delete obj;
    collidables.clear();
}


Game* Game::curGame = nullptr;
Draw* Game::screen = nullptr;
Input* Game::input = nullptr;

Game::Game(short width, short height, char backChar, COORD fontsz): width(width), height(height), fontSize(fontsz), _gameRunning(false), gameRunning(_gameRunning) {
    curGame = this;
    screen = new Draw(backChar);
    input = new Input;

    screen->setWindowResizeable(false);
    screen->setFont(fontSize.X, fontSize.Y); // use raster font
    screen->setSize(width, height);
    screen->drawClear();

    if(screen->getFontSize().X != fontSize.X || screen->getFontSize().Y != fontSize.Y){
        fontSize = screen->getFontSize(); // truncate invalid font size to correct size
    }
}

Game::~Game() {
    Collidable::clearCollidables();
    Object::clearObjects();

    delete screen;
    delete input;

    curGame = nullptr;
    screen = nullptr;
    input = nullptr;
}

void Game::runGame(unsigned int msPerTick) {
    Clock tick;
    _gameRunning = true;
    while(1){
        if(tick.getMilliseconds() > msPerTick){
            if(!update()) break;

            COORD curFontSize = screen->getFontSize();

            if(screen->getScreenSize().X != width || screen->getScreenSize().Y != height ||
                           curFontSize.X != fontSize.X || curFontSize.Y != fontSize.Y) {
                screen->setFont(fontSize.X, fontSize.Y);
                screen->setSize(width, height);
                screen->drawClear();
                Object::refreshObjects();
            }

            tick.restart();
        } else {
            Sleep(msPerTick);
        }
    }
    _gameRunning = false;
}
