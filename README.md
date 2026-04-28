# Program 4 - Nim (Client/Server)

## Team Members

* Jackson Bennett
* Davis Burt
* Riley Hupp
* Stephen Smith

## Project Overview

This project is a networked version of the game Nim.
It uses C++ and the Winsock2 library to allow two players to play over UDP.

## What the Program Does

* Lets a user host a game or challenge another host
* Uses UDP datagrams for negotiation and gameplay
* Supports moves, chat messages, and forfeiting
* Ends the game when a player wins, loses, forfeits, or times out

## How to Build

1. Open the project folder in VS Code.
2. Make sure you have a C++ compiler (MSVC or MinGW) installed and on your PATH.
3. Press `Ctrl+Shift+B` to run the build task — this compiles the project and produces `nim.exe`.

## How to Run

1. Start the program.
2. Enter your player name.
3. Choose whether to host a game or challenge another host.
4. Follow the prompts to play.

## Files / Structure

* `nim.cpp` - Program entry point
* `nim.h` - Constants, structs, and function declarations
* `network.cpp` - UDP/Winsock2 communication, game loop, menus, and prompts
* `game.cpp` / `game.h` - Nim game logic and validation

## Team Responsibilities

* Davis Burt - Networking / sockets / game loop
* Riley Hupp - Negotiation / lobby flow
* Stephen Smith - Game logic / validation
* Jackson Bennett - Testing / documentation

## Known Bugs

* None known at this time

## Notes

* Port used: `29333`
* All datagrams are null-terminated C-style strings
* Client/challenger goes first
* Server/host chooses the starting piles

## Submission Checklist

* Source code included
* Build instructions included
* Team member responsibilities included
* Known bugs listed
* Team evaluation form completed
