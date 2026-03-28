#pragma once
#include "board.hpp"

MoveList generatePseudoLegal(const Board& b);
MoveList generateLegal(Board& b);
