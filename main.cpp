#include "main.h"

using std::string, std::vector, std::to_string, std::min, std::max;

GamePlayer::GamePlayer(short ind, Collidable* ii): instance(ii) { index = ind; }

GamePlayer::~GamePlayer() {
    if(instance != nullptr) delete instance;
}

/// Initialize Game Instance
MyGame::MyGame(short width, short height, GamePlayer* me): Game(width, height, BACK_CHAR, {FONT_WIDTH, FONT_HEIGHT}), mplay(Multiplayer(me)) {
    user = new Collidable(5,5,PLAYER_CHAR); /// player object

    me->instance = user;

    score = 0;
    gameOver = false;

    /// border walls
    for(short i=2; i<curScreen().getScreenSize().Y-1; i++){
        new Collidable(0, i, BORDER_CHAR);
        new Collidable(curScreen().getScreenSize().X-1, i, BORDER_CHAR);
    }
    for(short i=0; i<curScreen().getScreenSize().X; i++){
        new Collidable(i, 1, BORDER_CHAR);
        new Collidable(i, curScreen().getScreenSize().Y-1, BORDER_CHAR);
    }

}

MyGame::~MyGame() {
    delete user;
}


#define CMD_IS_HOST             2
#define CMD_UPDATE_POSITION     12


///Single Game Tick
bool MyGame::update() {
    /// multiplayer update tick
    mplay.update();
    while(mplay.readData([&](short index, short cmd, unsigned short argc, const vector<short>& argv){
        switch(cmd){
            case CMD_PLAYER_INDEX:{
                GamePlayer* p = (GamePlayer*)mplay.findPlayer(index);
                if(p == nullptr){
                    p = new GamePlayer(index, new Collidable(-1, -1, PLAYER_CHAR));
                } else {
                    p->alive.restart();
                }
                break;}
            case CMD_UPDATE_POSITION:{
                if(argc < 2) return;
                GamePlayer* p = (GamePlayer*)mplay.findPlayer(index); //index
                if(p != nullptr && p->instance != nullptr){
                    p->instance->updatePos(argv[0], argv[1]); // x y
                }
                break;}
        }
    }));

    if(refreshTimer.getSeconds() > 2){
        curScreen().drawClear();
        Object::refreshObjects();

        curScreen().drawText(1, 0, (string("Currently ") + (mplay.getPlayers().size() == 1 ? "no" : to_string(mplay.getPlayers().size())) + " players in game         " ).data());

        refreshTimer.restart();
    }


    if(!gameOver && mplay.myIndex() != -1) do { // high priority game tick
        bool stopMovement = false;
        short x = user->X(),
              y = user->Y(),
              _x = x, _y = y;

        /// Get player current moving direction input
        switch(curInput().getDirection()){
            case 1: x += 1; break; case 2: y -= 1; break;
            case 3: x -= 1; break; case 4: y += 1; break;
            default: stopMovement = true; break;
        }

        if(stopMovement){
            keyTimer.restart(); // single step motion reset
            break;
        }

        if(keyTimer.getMilliseconds() < 300 && keyTimer.getMilliseconds() > 100) break; // allow easy single step motion

        /// Collision Event + Checking
        Collidable* cwall = Collidable::checkCollision(x, y);
        if(cwall != nullptr){
            switch(cwall->getChar()){
                case PLAYER_CHAR:
                case BORDER_CHAR:{ // collision with border
                    stopMovement = true;
                    break;
                }
                case POINT_CHAR: { // collision with point
                    score += 50; // increment score by a lot
                    delete cwall; // destroy point
                    break;
                }
            }
            if(stopMovement) break;
        }
        if(_x != x || _y != y){
            user->updatePos(x, y); // move player
            mplay.writeCommand(mplay.myIndex(), CMD_UPDATE_POSITION, {x, y});
        }

        bool stuck = true;
        {
            // check all sides for walls
            Collidable* sides[4] = {
                Collidable::checkCollision(x-1,y),
                Collidable::checkCollision(x+1,y),
                Collidable::checkCollision(x,y-1),
                Collidable::checkCollision(x,y+1)};
            for(int i=0; i<4; ++i){
                if(sides[i] == nullptr){ stuck = false; break;}
                char c = sides[i]->getChar();
                if(c != BORDER_CHAR && c != DWALL_CHAR){
                    stuck = false; break;
                }
            }
        }

        /// Player is stuck and has no unlocks to use - Game Over
        if(stuck){
            gameOver = true;
        }

    } while(0); // just use break
    return true;
}


/// Start The Game - Program Entry Point
int main() {
    srand(time(0));
    SetConsoleTitle("Game");

    { /// Game and game loop
        MyGame game(GAME_SIZE_W, GAME_SIZE_H, new GamePlayer); // new game instance with given map size
        game.runGame(); //blocking until game is over
    }
    {   /// End title curScreen
        Draw curScreen;
        Input keyboard;
        curScreen.drawClear();
        string message("Thank You For Playing!");
        curScreen.drawText(curScreen.getScreenSize().X / 2 - message.size() / 2, curScreen.getScreenSize().Y / 2, message.data());
        keyboard.waitAnykey(2);
    }
    return 0;
}
