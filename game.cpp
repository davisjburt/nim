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

void Nim::printGame() {
    for (int i = 0; i < (int)piles.size(); ++i) {
        std::cout << "[Pile " << i + 1 << "] : ";

        for (int j = 0; j < piles[i]; ++j) {
            std::cout << "#";
        }

        std::cout << " (" << piles[i] << ")" << std::endl;
    }
}

bool Nim::isGameOver()
{
    for (int pile : piles) {
        if (pile > 0) return false;
    }
    return true;
}

bool Nim::tryMove(int pile, int amt) {
    if (pile < 1 || pile > (int)piles.size()) return false;
    if (amt < 1 || amt > piles[pile - 1]) return false;

    piles[pile - 1] -= amt;
    return true;
}

bool Nim::decodeMove(const std::string msg, int& pile, int& amt){
    if(msg.length() != 3){
        return false;
    }

    if(!isdigit(msg[0]) || !isdigit(msg[1]) || !isdigit(msg[2])){
        return false;
    }

    pile = msg.at(0) - '0';
    amt = (msg.at(1) - '0') * 10 + (msg.at(2) - '0');

    if (pile < 1 || pile > (int)piles.size()) return false;
    if (amt < 1 || amt > piles[pile - 1]) return false;

    return true;
}

bool Nim::verifyAction(const std::string action, int& pile, int& amt){
    if(action.empty()){
        return false;
    }

    if (action.at(0) == 'F'){
        gameOver = true;
        return true;
    }
    else if (action.at(0) == 'C'){
        return true;
    }
    else if (decodeMove(action, pile, amt)){
        return true;
    }

    return false;
}