#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include <string>
#include "game.h"
#include "nim.h"
#include "ui.h"


using namespace std;

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "WSAStartup failed" << endl;
        return 1;
    }

    char myName[MAX_NAME] = "";
    cout << "Enter your name: ";
    cin.getline(myName, MAX_NAME);


    if (strlen(myName) == 0) {
        strcpy_s(myName, MAX_NAME, "Player");
    }

    while (true) {
        int choice = uiGetMainMenuChoice();

        if (choice == 1) {
            hostMode(myName);
        } else if (choice == 2) {
            clientMode(myName);
        } else {
            break;
        }
    }

    WSACleanup();
    return 0;
}