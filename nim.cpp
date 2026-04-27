#include <iostream>
#include <WinSock2.h>
#include <string>
#include "game.h"

using namespace std;

extern void hostMode(const char myName[]);
extern bool clientMode(const char myName[]);
extern int getChoice(int low, int high);