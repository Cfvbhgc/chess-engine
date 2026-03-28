#pragma once
#include "board.hpp"

struct SearchResult {
    Move bestMove;
    int score;
};

SearchResult search(Board& b, int depth);
