#pragma once
#include <winsock2.h>

#define MAX_NAME        81   //<= 80 chars + null
#define MAX_SERVERS     20
#define DEFAULT_BUFLEN  512
#define NIM_PORT        29333

// Negotiation strings 
#define NIM_QUERY       "Who?"
#define NIM_NAME_PFX    "Name="
#define NIM_PLAYER_PFX  "Player="
#define NIM_YES         "YES"
#define NIM_NO          "NO"
#define NIM_GREAT       "GREAT!"

// Game-phase prefixes
#define NIM_CHAT_FLAG   'C'
#define NIM_FORFEIT     'F'

struct ServerEntry {
    char name[MAX_NAME];
    sockaddr_in addr;       // IP + port of the responding server
};


int  wait(SOCKET s, int seconds, int msec);
int  getServers(SOCKET s, ServerEntry servers[]);      
sockaddr_in GetBroadcastAddress(char* ip, char* mask);
sockaddr_in GetBroadcastAddressAlternate(char* ip);