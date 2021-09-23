#include "game.h"

int main(int argc, char *argv[])
{
    playGame(argc >= 2 ? argv[1] : "game1.txt");
}
