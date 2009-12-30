#include <iostream>
#include "pyconsole.h"

using namespace std;

int main() {

    string input, output, outerr;
    pyconsole::PyConsole * console = pyconsole::getConsole();
    while (console->alive()) {
        cout << ">>> ";
        getline(cin, input);
        if (cin.eof())
            break;
        if (!input.size())
            continue;
        if (input == "restart") {
            cout << "Ready to restart py console" << endl;
            console->restart();
            continue;
        }

        console->loop_once(input, output, outerr);
        cout << output;
        cerr << outerr;
    }
    
    return 0;
}
