
#include "console_library.h"
#include "multiplayer.h"
#include "windows.h"
#include <iostream>

using namespace std;

int main(){
    cout << "Testing..." << endl;

    Input input;

    Multiplayer mplay;
    do {
        mplay.writeCommand(24, 3, {});
        mplay.writeCommand(24, 12, {3 + rand() % 4, 3 + rand() % 4});
        mplay.update();
        mplay.readData([](short index, short cmd, unsigned short argc, const vector<short>& argv){
            cout << "index: " << index << " cmd: " << cmd << " argc: " << argc << " argv: \n";
            for(size_t i=0; i<argc; ++i) cout << argv[i] << ", ";
            cout << endl;
        });
        Sleep(100);
    } while(!input.anyKey());

    return 0;
}
