#pragma once

#ifndef NIM
#define NIM

#include <vector>
#include <string>

class Nim
{
    private:
    std::vector<int> piles;
    int currentPlayer;
    bool gameOver = false;

    public:
    Nim(const std::string& gamestate);
    void printGame();
    bool isGameOver();
    bool tryMove(int pile, int amt);
    bool verifyAction(const std::string action, int& pile, int& amt);
    bool decodeMove(const std::string msg, int& pile, int& amt);
};

#endif