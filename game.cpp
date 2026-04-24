#include <vector>
#include <string>
#include <iostream>
#include "game.h"

Nim::Nim(const std::string& gamestate)
{
    if (gamestate.length() < 7){
        std::cout << "Invalid board state entered.\n";
        return;
    }
    else{
        int numPiles = gamestate.at(0) - '0';
        piles.resize(numPiles);

        for(int i = 0; i < numPiles; ++i){
            int numOne = gamestate.at(1 + i*2) - '0';
            int numTwo = gamestate.at(2 + i*2) - '0';
            int pileSize = numOne * 10 + numTwo;

            piles[i] = pileSize;
        }
    }
}

void Nim::printGame(){
    for (int i = 0; i < piles.size(); ++i){
        std::cout << piles[i] << " ";
    }
    std::cout << "\n";
}

bool Nim::isGameOver()
{
    return gameOver;
}