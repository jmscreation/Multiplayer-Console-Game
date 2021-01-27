#include "multiplayer.h"
#include <iostream>

using std::string, std::vector, std::cout;

Multiplayer::Multiplayer(Multiplayer::Player* me): Me(me) {
    socket.setBlocking(false);
    if(Player::context == nullptr) Player::context = this; // auto-set context

    if(me != nullptr && Player::context == this) me->listAppend(); // auto-add outside player into list

    for(int i=0; i < MAX_PORT_BIND; i++){
        if(socket.bind(PORT + i) == sf::Socket::Done){
            currentPort = PORT + i;
            break;
        }
    }
}

Multiplayer::~Multiplayer() {
    socket.unbind();
    if(Player::context == this) Player::context = nullptr;
    for(Player* p : players) delete p;
}

void Multiplayer::broadcast(sf::Packet& data){
    for(int i=0; i < MAX_PORT_BIND; i++){
        socket.send(data, sf::IpAddress::Broadcast, PORT + i);
    }
}

bool Multiplayer::update() {

    // player index management
    if(refreshTimer.getElapsedTime().asSeconds() > 4){
        for(size_t i=0;i<players.size(); ++i){
            Player* p = players[i];
            if(p == Me) continue; // skip local player

            if(p->alive.getElapsedTime().asSeconds() > 4) {
                delete p;
                players.erase(players.begin() + i--);
            }
        }
        assignIndex(Me);
        if(Me->index != -1)
            writeCommand(Me->index, CMD_PLAYER_INDEX, {});
        refreshTimer.restart();
    }

    sf::Socket::Status status;
    // send data
    broadcast(output);
    output.clear();
    input.clear();
    tmp.clear();

    do { // receive data
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

void Multiplayer::writeCommand(short cmd, const std::vector<short>& args){
    short ind = Me == nullptr ? -1 : Me->index;
    writeCommand(ind, cmd, args);
}

void Multiplayer::writeCommand(short index, short cmd, const std::vector<short>& args){
    output << sf::Int16(index) << sf::Int16(cmd) << sf::Uint16(args.size());
    for(size_t i=0; i<args.size(); ++i) output << sf::Int16(args[i]);
}

Multiplayer::Player* Multiplayer::findPlayer(short ind){
    for(Player* p : players){
        if(p->index == ind) return p;
    }
    return nullptr;
}

void Multiplayer::assignIndex(Multiplayer::Player* me) {
    if(me == nullptr || me->index >= 0) return; // check if index already assigned
    short ind = 0;
    for(Player* p : players){
        if(p->index < 0 || p == me) continue;
        ind = std::max(ind, short(p->index + 1));
    } // get the highest index
    me->index = ind;
}

Multiplayer* Multiplayer::Player::context = nullptr;

Multiplayer::Player::Player(): index(-1) {
    listAppend();
}
Multiplayer::Player::~Player() {}

void Multiplayer::Player::listAppend() {
    if(context == nullptr) return;
    context->players.push_back(this);
}

