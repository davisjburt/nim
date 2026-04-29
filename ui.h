#pragma once
#ifndef UI_H
#define UI_H

#include <string>
#include "game.h"

// Menu
int uiGetMainMenuChoice();

// Input
std::string uiGetPlayerMove();
std::string uiGetChatMessage();

// Display
void uiShowMessage(const std::string& msg);
void uiShowBoard(Nim& game);
void uiShowOpponentMove(int pile, int amt);
void uiShowTurn(bool myTurn);

#endif