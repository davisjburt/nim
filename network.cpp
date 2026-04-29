#include <iostream>
#include <cstring>
#include <string>
#include <cctype>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include "nim.h"
#include "game.h"
#include "ui.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;

bool sameTextIgnoreCase(const char a[], const char b[]) {
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i])) {
            return false;
        }
        i++;
    }
    return a[i] == '\0' && b[i] == '\0';
}

bool startsWithIgnoreCase(const char text[], const char front[]) {
    int i = 0;
    while (front[i] != '\0') {
        if (text[i] == '\0') return false;
        if (tolower((unsigned char)text[i]) != tolower((unsigned char)front[i])) {
            return false;
        }
        i++;
    }
    return true;
}

bool sameServer(sockaddr_in a, sockaddr_in b) {
    return a.sin_family == b.sin_family &&
        a.sin_port == b.sin_port &&
        a.sin_addr.s_addr == b.sin_addr.s_addr;
}

void printAddress(sockaddr_in addr) {
    char ip[INET_ADDRSTRLEN] = "";
    inet_ntop(AF_INET, (void*)&addr.sin_addr, ip, INET_ADDRSTRLEN);
    cout << ip << ":" << ntohs(addr.sin_port);
}

int getChoice(int low, int high) {
    string line;
    int choice;
    while (true) {
        cout << "> ";
        getline(cin, line);
        choice = atoi(line.c_str());
        if (choice >= low && choice <= high) return choice;
        cout << "Try again." << endl;
    }
}

int wait(SOCKET s, int seconds, int msec) {
    fd_set set;
    FD_ZERO(&set);
    FD_SET(s, &set);

    timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = msec * 1000;

    return select(0, &set, nullptr, nullptr, &tv);
}

bool sendMessage(SOCKET s, const sockaddr_in& addr, const char msg[]) {
    int sent = sendto(s, msg, (int)strlen(msg) + 1, 0,
        (sockaddr*)&addr, sizeof(addr));
    return sent != SOCKET_ERROR;
}

bool receiveMessage(SOCKET s, char buf[], int bufSize,
    sockaddr_in& fromAddr, int timeoutSeconds) {
    int ready = wait(s, timeoutSeconds, 0);
    if (ready <= 0) return false;

    int fromSize = (int)sizeof(fromAddr);
    int len = recvfrom(s, buf, bufSize - 1, 0,
        (sockaddr*)&fromAddr, &fromSize);

    if (len == SOCKET_ERROR) return false;
    buf[len] = '\0';
    return true;
}

int getServers(SOCKET s, ServerEntry servers[]) {
    BOOL canBroadcast = TRUE;
    setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char*)&canBroadcast, sizeof(canBroadcast));

    sockaddr_in bcast{};
    bcast.sin_family = AF_INET;
    bcast.sin_port = htons(NIM_PORT);
    bcast.sin_addr.s_addr = INADDR_BROADCAST;

    sendMessage(s, bcast, NIM_QUERY);

    int count = 0;
    char buf[DEFAULT_BUFLEN];

    while (true) {
        sockaddr_in from{};
        if (!receiveMessage(s, buf, DEFAULT_BUFLEN, from, 2)) break;

        if (startsWithIgnoreCase(buf, NIM_NAME_PFX)) {
            bool alreadySeen = false;
            for (int i = 0; i < count; i++) {
                if (sameServer(servers[i].addr, from)) {
                    alreadySeen = true;
                    break;
                }
            }

            if (!alreadySeen && count < MAX_SERVERS) {
                servers[count].addr = from;
                strcpy_s(servers[count].name, MAX_NAME, buf + strlen(NIM_NAME_PFX));
                count++;
            }
        }
    }

    return count;
}

static bool validateBoard(const char* s) {
    if (!s || !isdigit((unsigned char)s[0])) return false;
    int n = s[0] - '0';
    if (n < 3 || n > 9) return false;
    if ((int)strlen(s) != 1 + n * 2) return false;
    for (int i = 0; i < n; i++) {
        char c1 = s[1 + i * 2], c2 = s[2 + i * 2];
        if (!isdigit((unsigned char)c1) || !isdigit((unsigned char)c2)) return false;
        int rocks = (c1 - '0') * 10 + (c2 - '0');
        if (rocks < 1 || rocks > 20) return false;
    }
    return true;
}

static string buildBoardString() {
    cout << "Enter board string. Example: 3030405  =  3 piles with 3, 4, and 5 rocks" << endl;
    string board;
    while (true) {
        cout << "> ";
        getline(cin, board);
        if (validateBoard(board.c_str())) break;
        cout << "Invalid. Must be 3-9 piles, each pile 01-20 rocks. Try again." << endl;
    }
    return board;
}

void playGame(SOCKET s, sockaddr_in opponent, bool isClient) {
    char buf[DEFAULT_BUFLEN];
    string boardStr;

    if (!isClient) {
        boardStr = buildBoardString();
        sendMessage(s, opponent, boardStr.c_str());
    }
    else {
        sockaddr_in from{};
        if (!receiveMessage(s, buf, DEFAULT_BUFLEN, from, 30)) {
            uiShowMessage("No board received in time. You win by default!");
            return;
        }
        if (!validateBoard(buf)) {
            uiShowMessage("Invalid board received. You win by default!");
            return;
        }
        boardStr = buf;
    }

    Nim game(boardStr);
    uiShowBoard(game);

    bool myTurn = isClient;

    while (true) {
        if (myTurn) {
            uiShowTurn(true);
            while (true) {
                string line = uiGetPlayerMove();
                if (line.empty()) continue;
                char first = toupper((unsigned char)line[0]);

                if (first == 'F') {
                    sendMessage(s, opponent, "F");
                    uiShowMessage("You forfeited. Game over.");
                    return;
                }
                else if (first == 'C') {
                    string chat = uiGetChatMessage();
                    string dg = string(1, NIM_CHAT_FLAG) + chat;
                    sendMessage(s, opponent, dg.c_str());
                }
                else if (first >= '1' && first <= '9') {
                    int pile = 0, amt = 0;
                    if (!game.decodeMove(line, pile, amt)) {
                        uiShowMessage("Invalid move. Format: pile(1-9) + rocks(01-20), e.g. 305. Try again.");
                        continue;
                    }
                    game.tryMove(pile, amt);
                    sendMessage(s, opponent, line.c_str());
                    uiShowBoard(game);
                    if (game.isGameOver()) {
                        uiShowMessage("You removed the last rock. You win!");
                        return;
                    }
                    break;
                }
                else {
                    uiShowMessage("Enter a move code (e.g. 305), C, or F.");
                }
            }
            myTurn = false;
        }
        else {
            uiShowTurn(false);
            while (true) {
                sockaddr_in from{};
                if (!receiveMessage(s, buf, DEFAULT_BUFLEN, from, 30)) {
                    uiShowMessage("Opponent timed out. You win by default!");
                    return;
                }
                if (!sameServer(from, opponent)) continue;

                int pile = 0, amt = 0;
                if (!game.verifyAction(string(buf), pile, amt)) {
                    uiShowMessage("Opponent sent invalid move. You win by default!");
                    return;
                }

                char fc = buf[0];
                if (fc == NIM_CHAT_FLAG) {
                    uiShowMessage(string("Opponent: ") + (buf + 1));
                }
                else if (fc == NIM_FORFEIT) {
                    uiShowMessage("Opponent forfeited. You win!");
                    return;
                }
                else {
                    game.tryMove(pile, amt);
                    uiShowOpponentMove(pile, amt);
                    uiShowBoard(game);
                    if (game.isGameOver()) {
                        uiShowMessage("Opponent removed the last rock. You lose.");
                        return;
                    }
                    break;
                }
            }
            myTurn = true;
        }
    }
}

void hostMode(const char myName[]) {
    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == INVALID_SOCKET) {
        cout << "socket() failed" << endl;
        return;
    }

    BOOL reuse = TRUE;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));

    sockaddr_in myAddr{};
    myAddr.sin_family = AF_INET;
    myAddr.sin_port = htons(NIM_PORT);
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s, (sockaddr*)&myAddr, sizeof(myAddr)) == SOCKET_ERROR) {
        cout << "bind() failed: error " << WSAGetLastError() << endl;
        closesocket(s);
        return;
    }

    cout << "Hosting on port " << NIM_PORT << endl;

    char buf[DEFAULT_BUFLEN];
    sockaddr_in clientAddr{};

    while (true) {
        sockaddr_in from{};
        if (!receiveMessage(s, buf, DEFAULT_BUFLEN, from, 1)) {
            continue;
        }

        cout << "[RECV <- ";
        printAddress(from);
        cout << "] " << buf << endl;

        if (sameTextIgnoreCase(buf, NIM_QUERY)) {
            char reply[DEFAULT_BUFLEN] = "";
            strcpy_s(reply, DEFAULT_BUFLEN, NIM_NAME_PFX);
            strcat_s(reply, DEFAULT_BUFLEN, myName);

            cout << "[SEND -> ";
            printAddress(from);
            cout << "] " << reply << endl;
            sendMessage(s, from, reply);
        }
        else if (startsWithIgnoreCase(buf, NIM_PLAYER_PFX)) {
            const char* challenger = buf + strlen(NIM_PLAYER_PFX);

            cout << challenger << " wants to play Nim." << endl;
            cout << "1. Accept" << endl;
            cout << "2. Refuse" << endl;
            int choice = getChoice(1, 2);

            if (choice == 1) {
                sendMessage(s, from, NIM_YES);

                sockaddr_in again{};
                char reply[DEFAULT_BUFLEN];
                if (receiveMessage(s, reply, DEFAULT_BUFLEN, again, 2) &&
                    sameServer(from, again) &&
                    sameTextIgnoreCase(reply, NIM_GREAT)) {
                    cout << "Handshake complete. Game can start." << endl;
                    clientAddr = from;
                    break;
                }
                else {
                    cout << "Did not receive GREAT! in time." << endl;
                }
            }
            else {
                sendMessage(s, from, NIM_NO);
            }
        }
    }

    playGame(s, clientAddr, false);
    closesocket(s);
}

bool clientMode(const char myName[]) {
    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == INVALID_SOCKET) {
        cout << "socket() failed" << endl;
        return false;
    }

    ServerEntry servers[MAX_SERVERS]{};
    int count = getServers(s, servers);

    if (count <= 0) {
        cout << "No hosts found." << endl;
        closesocket(s);
        return false;
    }

    cout << "Available hosts:" << endl;
    for (int i = 0; i < count; i++) {
        cout << i + 1 << ". " << servers[i].name << " (";
        printAddress(servers[i].addr);
        cout << ")" << endl;
    }

    cout << "Pick a host:" << endl;
    int picked = getChoice(1, count) - 1;

    char msg[DEFAULT_BUFLEN] = "";
    strcpy_s(msg, DEFAULT_BUFLEN, NIM_PLAYER_PFX);
    strcat_s(msg, DEFAULT_BUFLEN, myName);

    cout << "[SEND -> ";
    printAddress(servers[picked].addr);
    cout << "] " << msg << endl;
    sendMessage(s, servers[picked].addr, msg);

    char reply[DEFAULT_BUFLEN];
    sockaddr_in from{};
    bool gotReply = receiveMessage(s, reply, DEFAULT_BUFLEN, from, 10);

    if (!gotReply || !sameServer(from, servers[picked].addr)) {
        cout << "No valid reply. Treat as NO." << endl;
        closesocket(s);
        return false;
    }

    cout << "[RECV <- ";
    printAddress(from);
    cout << "] " << reply << endl;

    if (sameTextIgnoreCase(reply, NIM_YES)) {
        sendMessage(s, servers[picked].addr, NIM_GREAT);
        cout << "Handshake complete. Game can start." << endl;
        playGame(s, servers[picked].addr, true);
        closesocket(s);
        return true;
    }

    cout << "Challenge refused." << endl;
    closesocket(s);
    return false;
}