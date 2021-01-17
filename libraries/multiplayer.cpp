#include "multiplayer.h"
#include <iostream>

#define PORT 9980

using std::string, std::vector, std::cout;

Multiplayer::Multiplayer() {
    socket.setBlocking(false);

    for(int i=0; i < 4; i++){
        if(socket.bind(PORT + i) == sf::Socket::Done){
            currentPort = PORT + i;
            break;
        }
    }
}

Multiplayer::~Multiplayer() {

}

void Multiplayer::broadcast(sf::Packet& data){
    for(int i=0; i < 4; i++){
        socket.send(data, sf::IpAddress::Broadcast, PORT + i);
    }
}

bool Multiplayer::update() {
    sf::Socket::Status status;
    broadcast(output);
    output.clear();
    input.clear();
    tmp.clear();

    do {
        sf::IpAddress from;
        unsigned short from_port;
        status = socket.receive(tmp, from, from_port);

        if(from == sf::IpAddress::getLocalAddress() && from_port == currentPort){
            tmp.clear();
            continue;
        }

        switch(status){
            case sf::Socket::Done:
                input.append(tmp.getData(), tmp.getDataSize());
                tmp.clear();
                continue;
            case sf::Socket::NotReady: return false;
            case sf::Socket::Error: return false;
            case sf::Socket::Partial: Sleep(50); continue;

            case sf::Socket::Disconnected: return false; // UDP should never return this
        }

    } while(1);
}

bool Multiplayer::readData(std::function<void (short, short, unsigned short, const std::vector<short>&)> callback){

    sf::Int16 ind, cmd;
    sf::Uint16 argc;
    vector<short> argv;

    if(!(input >> ind >> cmd >> argc)) return false;
    for(size_t i=0; i < argc; ++i){
        sf::Int16 arg;
        if(!(input >> arg)) return false;
        argv.push_back(arg);
    }

    callback(ind, cmd, argc, argv);

    return true;
}

void Multiplayer::writeCommand(short index, short cmd, const std::vector<short>& args){
    output << sf::Int16(index) << sf::Int16(cmd) << sf::Uint16(args.size());
    for(size_t i=0; i<args.size(); ++i) output << sf::Int16(args[i]);
}
