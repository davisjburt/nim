#include <vector>
#include <string>
#include <iostream>
#include "game.h"

Nim::Nim(const std::string& gamestate)
{
    //minimum check. i.e. 3010101
    if (gamestate.length() < 7) {
        std::cout << "Invalid board state entered.\n";
        return;
    }
    else {
        //minus zero so that we get actual number and not a char/string.
        int numPiles = gamestate.at(0) - '0';
        piles.resize(numPiles);

        //ensure correct number of piles
        if (gamestate.length() != 1 + numPiles * 2) {
            std::cout << "Invalid board state entered.\n";
            return;
        }

        for(int i = 0; i < numPiles; ++i){
            //similar logic to earlier, locates the correct number to be processed from the string.
            int numOne = gamestate.at(1 + i*2) - '0';
            int numTwo = gamestate.at(2 + i*2) - '0';
            int pileSize = numOne * 10 + numTwo;

            piles[i] = pileSize;
        }
    }
}

void Nim::printGame(){
    for (int i = 0; i < piles.size(); ++i){
        if (i == piles.size() - 1) {
            std::cout << piles[i];
        }
        else {
            std::cout << piles[i] << " ";
        }
    }
    std::cout << "\n";
}

bool Nim::isGameOver()
{
    for (int pile : piles) {
        if (pile > 0) return false;
    }
    return true;
}

bool Nim::tryMove(int pile, int amt) {
    if (pile < 1 || pile > piles.size()) return false;
    if (amt < 1 || amt > piles[pile - 1]) return false;

    piles[pile - 1] -= amt;
    return true;
}