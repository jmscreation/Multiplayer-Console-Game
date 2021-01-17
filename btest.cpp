/**

    This application is used for strictly low-level debugging purpose only

*/

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
