#include <iostream>
#include <cstring>
#include <string>
#include <cctype>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include "game_session.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const int NIM_PORT = 29333;
const int MAX_BUF = 512;
const int MAX_NAME = 81;
const int MAX_SERVERS = 50;

const char MSG_WHO[] = "Who?";
const char MSG_NAME[] = "Name=";
const char MSG_PLAYER[] = "Player=";
const char MSG_YES[] = "YES";
const char MSG_NO[] = "NO";
const char MSG_GREAT[] = "GREAT!";

struct HostInfo {
    sockaddr_in addr;
    char name[MAX_NAME];
};

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

int waitForSocket(SOCKET s, int seconds) {
    fd_set set;
    FD_ZERO(&set);
    FD_SET(s, &set);

    timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;

    return select(0, &set, nullptr, nullptr, &tv);
}

// Send one text message to a specific computer.
bool sendMessage(SOCKET s, const sockaddr_in& addr, const char msg[]) {
    int sent = sendto(s, msg, (int)strlen(msg) + 1, 0,
        (sockaddr*)&addr, sizeof(addr));
    return sent != SOCKET_ERROR;
}

// Wait a little while for a message to arrive.
bool receiveMessage(SOCKET s, char buf[], int bufSize,
                    sockaddr_in& fromAddr, int timeoutSeconds) {
    int ready = waitForSocket(s, timeoutSeconds);
    if (ready <= 0) return false;

    int fromSize = sizeof(fromAddr);
    int len = recvfrom(s, buf, bufSize - 1, 0,
        (sockaddr*)&fromAddr, &fromSize);

    if (len == SOCKET_ERROR) return false;
    buf[len] = '\0';
    return true;
}

// Check that the board string has the right shape.
int discoverHosts(SOCKET s, HostInfo hosts[]) {
    BOOL canBroadcast = TRUE;
    setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char*)&canBroadcast, sizeof(canBroadcast));

    sockaddr_in bcast{};
    bcast.sin_family = AF_INET;
    bcast.sin_port = htons(NIM_PORT);
    bcast.sin_addr.s_addr = INADDR_BROADCAST;

    sendMessage(s, bcast, MSG_WHO);

    int count = 0;
    char buf[MAX_BUF];

    while (true) {
        sockaddr_in from{};
        if (!receiveMessage(s, buf, MAX_BUF, from, 2)) break;

        if (startsWithIgnoreCase(buf, MSG_NAME)) {
            bool alreadySeen = false;
            for (int i = 0; i < count; i++) {
                if (sameServer(hosts[i].addr, from)) {
                    alreadySeen = true;
                    break;
                }
            }

            if (!alreadySeen && count < MAX_SERVERS) {
                hosts[count].addr = from;
                strcpy_s(hosts[count].name, MAX_NAME, buf + strlen(MSG_NAME));
                count++;
            }
        }
    }

    return count;
}

bool hostMode(const char myName[]) {
    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == INVALID_SOCKET) {
        cout << "socket() failed" << endl;
        return false;
    }

    sockaddr_in myAddr{};
    myAddr.sin_family = AF_INET;
    myAddr.sin_port = htons(NIM_PORT);
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s, (sockaddr*)&myAddr, sizeof(myAddr)) == SOCKET_ERROR) {
        cout << "bind() failed" << endl;
        closesocket(s);
        return false;
    }

    cout << "Hosting on port " << NIM_PORT << endl;

    char buf[MAX_BUF];
    bool gameStarted = false;

    while (true) {
        sockaddr_in from{};
        if (!receiveMessage(s, buf, MAX_BUF, from, 1)) {
            continue;
        }

        cout << "[RECV <- ";
        printAddress(from);
        cout << "] " << buf << endl;

        if (sameTextIgnoreCase(buf, MSG_WHO)) {
            char reply[MAX_BUF] = "";
            strcpy_s(reply, MAX_BUF, MSG_NAME);
            strcat_s(reply, MAX_BUF, myName);

            cout << "[SEND -> ";
            printAddress(from);
            cout << "] " << reply << endl;
            sendMessage(s, from, reply);
        }
        else if (startsWithIgnoreCase(buf, MSG_PLAYER)) {
            const char* challenger = buf + strlen(MSG_PLAYER);

            cout << challenger << " wants to play Nim." << endl;
            cout << "1. Accept" << endl;
            cout << "2. Refuse" << endl;
            int choice = getChoice(1, 2);

            if (choice == 1) {
                sendMessage(s, from, MSG_YES);

                sockaddr_in again{};
                char reply[MAX_BUF];
                if (receiveMessage(s, reply, MAX_BUF, again, 2) &&
                    sameServer(from, again) &&
                    sameTextIgnoreCase(reply, MSG_GREAT)) {
                    cout << "Handshake complete. Game can start." << endl;
                    runHostGameSession(s, from);
                    gameStarted = true;
                    break;
                }
                else {
                    cout << "Did not receive GREAT! in time." << endl;
                }
            }
            else {
                sendMessage(s, from, MSG_NO);
            }
        }
    }

    closesocket(s);
    return gameStarted;
}

bool clientMode(const char myName[]) {
    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == INVALID_SOCKET) {
        cout << "socket() failed" << endl;
        return false;
    }

    HostInfo hosts[MAX_SERVERS]{};
    int count = discoverHosts(s, hosts);

    if (count <= 0) {
        cout << "No hosts found." << endl;
        closesocket(s);
        return false;
    }

    cout << "Available hosts:" << endl;
    for (int i = 0; i < count; i++) {
        cout << i + 1 << ". " << hosts[i].name << " (";
        printAddress(hosts[i].addr);
        cout << ")" << endl;
    }

    cout << "Pick a host:" << endl;
    int picked = getChoice(1, count) - 1;

    char msg[MAX_BUF] = "";
    strcpy_s(msg, MAX_BUF, MSG_PLAYER);
    strcat_s(msg, MAX_BUF, myName);

    cout << "[SEND -> ";
    printAddress(hosts[picked].addr);
    cout << "] " << msg << endl;
    sendMessage(s, hosts[picked].addr, msg);

    char reply[MAX_BUF];
    sockaddr_in from{};
    bool gotReply = receiveMessage(s, reply, MAX_BUF, from, 10);

    if (!gotReply || !sameServer(from, hosts[picked].addr)) {
        cout << "No valid reply. Treat as NO." << endl;
        closesocket(s);
        return false;
    }

    cout << "[RECV <- ";
    printAddress(from);
    cout << "] " << reply << endl;

    if (sameTextIgnoreCase(reply, MSG_YES)) {
        sendMessage(s, hosts[picked].addr, MSG_GREAT);
        cout << "Handshake successful" << endl;
        runClientGameSession(s, hosts[picked].addr);
        closesocket(s);
        return true;
    }

    cout << "Challenge refused." << endl;
    closesocket(s);
    return false;
}

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

    cout << "1. Host a game" << endl;
    cout << "2. Challenge a host" << endl;
    cout << "3. Quit" << endl;

    while (true) {
        int choice = getChoice(1, 3);
        if (choice == 3) {
            break;
        }

        if (choice == 1) {
            // Host waits for a challenge and then starts the game.
            hostMode(myName);
        }
        else {
            // Client looks for hosts, picks one, and starts play after setup.
            clientMode(myName);
        }

        cout << "1. Host a game" << endl;
        cout << "2. Challenge a host" << endl;
        cout << "3. Quit" << endl;
    }

    WSACleanup();
    return 0;
}