#include "GameManager.h"

#include <cstring>

using namespace std;

int main(int argc, char* argv[])
{
    GameManager game;
    if (argc > 1 && strcmp(argv[1], "--two-player") == 0) game.runTwoPlayer();
    else if (argc > 1 && strcmp(argv[1], "--debug") == 0) game.runStory(true);
    else game.run();
    return 0;
}
