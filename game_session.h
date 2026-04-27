#pragma once

#include <WinSock2.h>
#include <string>

bool sameServer(sockaddr_in a, sockaddr_in b);
bool sendMessage(SOCKET s, const sockaddr_in& addr, const char msg[]);
bool receiveMessage(SOCKET s, char buf[], int bufSize,
                    sockaddr_in& fromAddr, int timeoutSeconds);

void runHostGameSession(SOCKET s, const sockaddr_in& opponent);
void runClientGameSession(SOCKET s, const sockaddr_in& opponent);