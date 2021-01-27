
#include "SFML/Network.hpp"
#include "windows.h"
#include <string>
#include <vector>
#include <functional>


#define PORT        9980
#define CMD_PLAYER_INDEX    -1024

class Multiplayer {
    short currentPort;
    sf::Packet tmp, input, output;
    sf::UdpSocket socket;
    sf::Clock refreshTimer;
public:
    class Player {
        static Multiplayer* context;
    public:
        short index;
        sf::Clock alive;

        Player();
        ~Player();

        void listAppend();

        friend class Multiplayer;
    };

    Player* Me;
    std::vector<Player*> players;

    Multiplayer(Player* me = nullptr);
    ~Multiplayer();

    void assignIndex(Player* me);
    Player* findPlayer(short ind);
    inline const std::vector<Player*>& getPlayers() { return players; }
    inline void setPlayerContext() { Player::context = this; }

    inline short myIndex() { return Me == nullptr ? -1 : Me->index; }

    void broadcast(sf::Packet& data);
    bool update();
    bool readData(std::function<void (short, short, unsigned short, const std::vector<short>&)> callback);
    void writeCommand(short index, short cmd, const std::vector<short>& args);

};
