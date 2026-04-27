#include <chrono>
#include <cctype>
#include <iostream>
#include <string>
#include "game.h"
#include "game_session.h"

using namespace std;

namespace {
const int GAME_BUF = 512;

bool isValidBoardState(const string& state) {
    if (state.length() < 7) {
        return false;
    }

    if (!isdigit((unsigned char)state[0])) {
        return false;
    }

    int pileCount = state[0] - '0';
    if (pileCount < 3 || pileCount > 9) {
        return false;
    }

    if (state.length() != 1 + pileCount * 2) {
        return false;
    }

    for (int i = 0; i < pileCount; ++i) {
        char tens = state[1 + i * 2];
        char ones = state[2 + i * 2];
        if (!isdigit((unsigned char)tens) || !isdigit((unsigned char)ones)) {
            return false;
        }

        int rocks = (tens - '0') * 10 + (ones - '0');
        if (rocks < 1 || rocks > 20) {
            return false;
        }
    }

    return true;
}

string promptBoardState() {
    string state;
    while (true) {
        cout << "Enter board state (example 3050505): ";
        getline(cin, state);
        if (isValidBoardState(state)) {
            return state;
        }

        cout << "Invalid board state. Try again." << endl;
    }
}

bool receiveFromOpponent(SOCKET s, const sockaddr_in& opponent, char buf[], int bufSize, int timeoutSeconds) {
    auto deadline = chrono::steady_clock::now() + chrono::seconds(timeoutSeconds);

    while (true) {
        auto now = chrono::steady_clock::now();
        if (now >= deadline) {
            return false;
        }

        int remaining = (int)chrono::duration_cast<chrono::seconds>(deadline - now).count();
        if (remaining < 1) {
            remaining = 1;
        }

        sockaddr_in from{};
        if (!receiveMessage(s, buf, bufSize, from, remaining)) {
            return false;
        }

        if (sameServer(from, opponent)) {
            return true;
        }
    }
}

void playGame(SOCKET s, const sockaddr_in& opponent, bool localStarts, const string& boardState) {
    if (!isValidBoardState(boardState)) {
        cout << "Game over. You won by default." << endl;
        return;
    }

    Nim game(boardState);
    bool localTurn = localStarts;

    cout << "Piles: ";
    game.printGame();

    while (true) {
        if (localTurn) {
            while (true) {
                cout << "Enter move, C for chat, or F to forfeit: ";
                string action;
                getline(cin, action);

                if (action.empty()) {
                    continue;
                }

                if (action[0] == 'C') {
                    string chatText;
                    if (action.length() > 1) {
                        chatText = action.substr(1);
                    }
                    else {
                        cout << "Enter chat message: ";
                        getline(cin, chatText);
                    }

                    string chatMsg = "C" + chatText;
                    sendMessage(s, opponent, chatMsg.c_str());
                    continue;
                }

                if (action[0] == 'F') {
                    sendMessage(s, opponent, "F");
                    cout << "You forfeited. Game over." << endl;
                    return;
                }

                int pile = 0;
                int amt = 0;
                if (!game.verifyAction(action, pile, amt)) {
                    cout << "Invalid move. Try again." << endl;
                    continue;
                }

                sendMessage(s, opponent, action.c_str());
                game.tryMove(pile, amt);
                cout << "Piles: ";
                game.printGame();

                if (game.isGameOver()) {
                    cout << "You removed the last rock. You win." << endl;
                    return;
                }

                localTurn = false;
                break;
            }
        }
        else {
            char buf[GAME_BUF];
            if (!receiveFromOpponent(s, opponent, buf, GAME_BUF, 30)) {
                cout << "No message received. You won by default." << endl;
                return;
            }

            string msg = buf;
            if (msg.empty()) {
                continue;
            }

            if (msg[0] == 'C') {
                cout << "Opponent: " << msg.substr(1) << endl;
                continue;
            }

            if (msg[0] == 'F') {
                cout << "Opponent forfeited. You win." << endl;
                return;
            }

            int pile = 0;
            int amt = 0;
            if (!game.verifyAction(msg, pile, amt)) {
                cout << "Opponent sent an invalid move. You won by default." << endl;
                return;
            }

            game.tryMove(pile, amt);
            cout << "Piles: ";
            game.printGame();

            if (game.isGameOver()) {
                cout << "Opponent removed the last rock. You lose." << endl;
                return;
            }
            localTurn = true;
        }
    }
}
} // namespace

void runHostGameSession(SOCKET s, const sockaddr_in& opponent) {
    string boardState = promptBoardState();
    sendMessage(s, opponent, boardState.c_str());
    playGame(s, opponent, false, boardState);
}

void runClientGameSession(SOCKET s, const sockaddr_in& opponent) {
    char boardReply[GAME_BUF];
    if (!receiveFromOpponent(s, opponent, boardReply, GAME_BUF, 30)) {
        cout << "Did not receive game state." << endl;
        return;
    }

    string boardState = boardReply;
    if (!isValidBoardState(boardState)) {
        cout << "Invalid board state received. You won by default." << endl;
        return;
    }

    playGame(s, opponent, true, boardState);
}