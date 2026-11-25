#include "game.h"

#include <cstring>

int main(int argc, char* argv[])
{
    if (argc > 1 && std::strcmp(argv[1], "--two-player") == 0) {
        tetris::run_two_player_game();
    } else if (argc > 1 && std::strcmp(argv[1], "--debug") == 0) {
        tetris::run_game(true);
    } else {
        tetris::run_main_menu();
    }
    return 0;
}
