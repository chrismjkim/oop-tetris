#pragma once

#include "game_types.h"

namespace tetris {

GimmickRules rules_for(int level);
const CardInfo& card_info(Augment kind);
int augment_count(const GameState& game, Augment kind, int shape = -1);
void select_card(GameState& game, int direction);
void take_card(GameState& game);
Piece landing_projection(const GameState& game);
void start_run(GameState& game, Mode mode, bool debug, std::uint32_t seed);
int difficulty_for(const GameState& game);
int shared_difficulty(const GameState& first, const GameState& second);
bool try_move(GameState& game, int delta, bool rotate);
bool try_hold(GameState& game);
void fall_step(GameState& game);
void hard_drop(GameState& game);
bool advance_tick(GameState& game, double elapsed_ms);

ClearResult resolve_lines(GameState& game);
TheftOutcome resolve_theft(GameState& game, const GimmickRules& rules);
void finish_turn(GameState& game, int level);

} // namespace tetris
