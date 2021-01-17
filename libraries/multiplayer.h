
#include "SFML/Network.hpp"
#include "windows.h"
#include <string>
#include <vector>
#include <functional>

class Multiplayer {
    short currentPort;
    sf::Packet tmp, input, output;
    sf::UdpSocket socket;
public:

    Multiplayer();
    ~Multiplayer();

    void broadcast(sf::Packet& data);
    bool update();
    bool readData(std::function<void (short, short, unsigned short, const std::vector<short>&)> callback);

    void writeCommand(short index, short cmd, const std::vector<short>& args);
};
