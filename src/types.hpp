#pragma once
#include <cstdint>
#include <string>
#include <vector>

using U64 = uint64_t;

enum Color : int { White, Black, ColorNone };
enum Piece : int { Pawn, Knight, Bishop, Rook, Queen, King, PieceNone };

enum Square : int {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    SquareNone = 64
};

inline int rankOf(int sq) { return sq >> 3; }
inline int fileOf(int sq) { return sq & 7; }
inline int makeSquare(int f, int r) { return r * 8 + f; }

enum MoveFlag : int {
    FlagNone = 0,
    FlagEP = 1,
    FlagCastle = 2,
    FlagPromoN = 3,
    FlagPromoB = 4,
    FlagPromoR = 5,
    FlagPromoQ = 6,
    FlagDoublePawn = 7
};

struct Move {
    int from;
    int to;
    int flag;
    Move() : from(0), to(0), flag(FlagNone) {}
    Move(int f, int t, int fl = FlagNone) : from(f), to(t), flag(fl) {}
    bool operator==(const Move& o) const { return from == o.from && to == o.to && flag == o.flag; }
    bool isNull() const { return from == 0 && to == 0 && flag == FlagNone; }
};

using MoveList = std::vector<Move>;

inline std::string squareToStr(int sq) {
    return std::string(1, 'a' + fileOf(sq)) + std::string(1, '1' + rankOf(sq));
}

inline int strToSquare(const std::string& s) {
    if (s.size() < 2) return SquareNone;
    return makeSquare(s[0] - 'a', s[1] - '1');
}

inline std::string moveToStr(const Move& m) {
    std::string s = squareToStr(m.from) + squareToStr(m.to);
    if (m.flag == FlagPromoN) s += 'n';
    else if (m.flag == FlagPromoB) s += 'b';
    else if (m.flag == FlagPromoR) s += 'r';
    else if (m.flag == FlagPromoQ) s += 'q';
    return s;
}

const int INF_SCORE = 100000;
const int MATE_SCORE = 99000;
