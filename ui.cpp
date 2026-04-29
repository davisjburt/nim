#include "ui.h"
#include <iostream>

using namespace std;

// ===================== MENU =====================
int uiGetMainMenuChoice() {
    cout << "\n========================\n";
    cout << "        NIM GAME\n";
    cout << "========================\n";
    cout << "1. Host a game\n";
    cout << "2. Challenge a host\n";
    cout << "3. Quit\n";
    cout << "> ";

    string input;
    getline(cin, input);
    return atoi(input.c_str());
}

// ===================== INPUT =====================
string uiGetPlayerMove() {
    cout << "Enter move (e.g. 305 = pile 3 remove 5)\n";
    cout << "C = Chat | F = Forfeit\n";
    cout << "> ";

    string input;
    getline(cin, input);
    return input;
}

string uiGetChatMessage() {
    cout << "Enter message: ";
    string msg;
    getline(cin, msg);
    return msg;
}

// ===================== DISPLAY =====================
void uiShowMessage(const string& msg) {
    cout << msg << endl;
}

void uiShowBoard(Nim& game) {
    cout << "\n===== CURRENT BOARD =====\n";
    game.printGame();
    cout << "=========================\n";
}

void uiShowOpponentMove(int pile, int amt) {
    cout << "\nOpponent removed " << amt
        << " rocks from pile " << pile << endl;
}

void uiShowTurn(bool myTurn) {
    if (myTurn)
        cout << "\n--- YOUR TURN ---\n";
    else
        cout << "\n--- OPPONENT TURN ---\n";
}