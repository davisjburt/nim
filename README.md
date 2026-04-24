# Program 4 - Nim (Client/Server)

## Team Members
- Jackson Bennett
- Davis Burt
- Riley Hupp
- Stephen Smith

## Project Overview
This project is a networked version of the game Nim.
It uses C++ and the Winsock2 library to allow two players to play over UDP.

## What the Program Does
- Lets a user host a game or challenge another host
- Uses UDP datagrams for negotiation and gameplay
- Supports moves, chat messages, and forfeiting
- Ends the game when a player wins, loses, forfeits, or times out

## How to Build
1. Open the project in Visual Studio.
2. Build the solution in the correct configuration.
3. Make sure Winsock2 is available on the system.

## How to Run
1. Start the program.
2. Enter your player name.
3. Choose whether to host a game or challenge another host.
4. Follow the prompts to play.

## Files / Structure
- `main.cpp` - Program entry point
- `network.cpp` / `network.h` - UDP and Winsock2 communication
- `game.cpp` / `game.h` - Nim game logic and validation
- `ui.cpp` / `ui.h` - Menus, prompts, and display

## Team Responsibilities
- Name 1 - Networking / sockets
- Name 2 - Negotiation / lobby flow
- Stephen Smith - Game logic / validation
- Name 4 - UI / testing / documentation

## Known Bugs
- None known at this time

## Notes
- Port used: `29333`
- All datagrams are null-terminated C-style strings
- Client/challenger goes first
- Server/host chooses the starting piles

## Special Instructions
Add any special setup or run instructions here.

## Submission Checklist
- Source code included
- Build instructions included
- Team member responsibilities included
- Known bugs listed
- Team evaluation form completed
