#pragma once
#include "types.hpp"
#include <array>

struct Board {
    std::array<int, 64> pieces;
    std::array<int, 64> colors;
    Color side;
    int epSquare;
    int castleRights; // bits: 0=wK, 1=wQ, 2=bK, 3=bQ
    int halfmove;
    int fullmove;

    U64 pieceBB[2][7]; // [color][piece]
    U64 occupancy[2];
    U64 allOccupancy;

    Board();
    void clear();
    void setFromFen(const std::string& fen);
    std::string toFen() const;
    void putPiece(int sq, Color c, Piece p);
    void removePiece(int sq);
    void makeMove(const Move& m);
    void unmakeMove(const Move& m, int capPiece, int capColor, int prevEp, int prevCastle, int prevHalf);
    bool isSquareAttacked(int sq, Color by) const;
    bool inCheck(Color c) const;
    int kingSquare(Color c) const;
    void updateBitboards();
};

inline U64 bit(int sq) { return 1ULL << sq; }
