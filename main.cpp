#include "main.h"

using std::string, std::vector, std::to_string, std::min, std::max;

struct Player {
    short index;
    Collidable* instance;
    Clock alive;
};

vector<Player*> players;

Player* Me = nullptr;

void assignIndex() {
    if(Me == nullptr || Me->index > 0) return;
    short ind = 0;
    for(Player* p : players){
        if(p->index < 0 || p == Me) continue;
        ind = max(ind, short(p->index + 1));
    } // get the highest index
    Me->index = ind;
}

Player* findPlayer(short ind){
    for(Player* p : players){
        if(p->index == ind) return p;
    }
    return nullptr;
}


/// Initialize Game Instance
MyGame::MyGame(short width, short height): Game(width, height, BACK_CHAR) {
    user = new Collidable(5,5,PLAYER_CHAR); /// player object

    Me->instance = user;

    score = 0;
    gameOver = false;

    //assert(curScreen().setFont(8, 8));

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
#define CMD_MY_INDEX            3
#define CMD_UPDATE_POSITION     12


///Single Game Tick
bool MyGame::update() {
    /// multiplayer update tick
    mplay.update();
    while(mplay.readData([&](short index, short cmd, unsigned short argc, const vector<short>& argv){
        switch(cmd){
            case CMD_MY_INDEX:{
                Player* p = findPlayer(index);
                if(p == nullptr){
                    players.push_back( new Player({index, new Collidable(-1, -1, PLAYER_CHAR), Clock()}) );
                } else {
                    p->alive.restart();
                }
                break;}
            case CMD_UPDATE_POSITION:{
                //curScreen().drawText(1, 0, (string("Update Pos: ") + to_string(argv[0]) + " " + to_string(argv[1]) ).data());
                if(argc < 2) return;
                Player* p = findPlayer(index); //index
                if(p != nullptr && p->instance != nullptr){
                    p->instance->updatePos(argv[0], argv[1]); // x y
                }
                break;}
        }
    }));

    if(refreshTimer.getSeconds() > 4){
        curScreen().drawClear();
        Object::refreshObjects();

        for(size_t i=0;i<players.size(); ++i){
            Player* p = players[i];
            if(p == Me) continue;

            if(p->alive.getSeconds() > 5) {
                if(p->instance != nullptr) delete p->instance;
                delete p;
                players.erase(players.begin() + i--);
            }
        }
        curScreen().drawText(1, 0, (string("Currently ") + (players.size() == 1 ? "no" : to_string(players.size())) + " players in game         " ).data());

        refreshTimer.restart();
    }

    // claim new index - send my index
    if(indexTimer.getSeconds() > (Me->index == -1 ? 3 : 1)){
        //curScreen().drawText(1, 0, (string("My Index: ") + to_string(Me->index) + "       " ).data());
        assignIndex();

        mplay.writeCommand(Me->index, CMD_MY_INDEX, {});
        indexTimer.restart();
    }

    if(!gameOver && Me->index != -1) do { // high priority game tick
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
            mplay.writeCommand(Me->index, CMD_UPDATE_POSITION, {x, y});
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

    Me = new Player;
    Me->index = -1;
    Me->instance = nullptr;
    players.push_back(Me);

    { /// Game and game loop
        MyGame game(GAME_SIZE_W, GAME_SIZE_H); // new game instance with given map size
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
