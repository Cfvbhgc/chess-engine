#include "movegen.hpp"

static void addPawnMoves(const Board& b, MoveList& moves) {
    Color us = b.side;
    int dir = (us == White) ? 8 : -8;
    int promoRank = (us == White) ? 7 : 0;
    int startRank = (us == White) ? 1 : 6;

    for (int sq = 0; sq < 64; sq++) {
        if (b.pieces[sq] != Pawn || b.colors[sq] != us) continue;

        int to = sq + dir;
        if (to >= 0 && to < 64 && b.pieces[to] == PieceNone) {
            if (rankOf(to) == promoRank) {
                moves.emplace_back(sq, to, FlagPromoQ);
                moves.emplace_back(sq, to, FlagPromoR);
                moves.emplace_back(sq, to, FlagPromoB);
                moves.emplace_back(sq, to, FlagPromoN);
            } else {
                moves.emplace_back(sq, to, FlagNone);
            }

            if (rankOf(sq) == startRank) {
                int to2 = sq + dir * 2;
                if (b.pieces[to2] == PieceNone)
                    moves.emplace_back(sq, to2, FlagDoublePawn);
            }
        }

        int capDirs[] = {dir - 1, dir + 1};
        for (int cd : capDirs) {
            int ct = sq + cd;
            if (ct < 0 || ct > 63) continue;
            if (abs(fileOf(ct) - fileOf(sq)) != 1) continue;

            if (ct == b.epSquare) {
                moves.emplace_back(sq, ct, FlagEP);
            } else if (b.pieces[ct] != PieceNone && b.colors[ct] != us) {
                if (rankOf(ct) == promoRank) {
                    moves.emplace_back(sq, ct, FlagPromoQ);
                    moves.emplace_back(sq, ct, FlagPromoR);
                    moves.emplace_back(sq, ct, FlagPromoB);
                    moves.emplace_back(sq, ct, FlagPromoN);
                } else {
                    moves.emplace_back(sq, ct, FlagNone);
                }
            }
        }
    }
}

static const int knightOff[] = {-17, -15, -10, -6, 6, 10, 15, 17};

static void addKnightMoves(const Board& b, MoveList& moves) {
    Color us = b.side;
    for (int sq = 0; sq < 64; sq++) {
        if (b.pieces[sq] != Knight || b.colors[sq] != us) continue;
        for (int off : knightOff) {
            int to = sq + off;
            if (to < 0 || to > 63) continue;
            int fd = abs(fileOf(to) - fileOf(sq));
            int rd = abs(rankOf(to) - rankOf(sq));
            if (!((fd == 1 && rd == 2) || (fd == 2 && rd == 1))) continue;
            if (b.pieces[to] != PieceNone && b.colors[to] == us) continue;
            moves.emplace_back(sq, to);
        }
    }
}

static void addSlidingMoves(const Board& b, MoveList& moves, Piece piece, const int dirs[], int ndirs) {
    Color us = b.side;
    for (int sq = 0; sq < 64; sq++) {
        if (b.pieces[sq] != piece || b.colors[sq] != us) continue;
        for (int i = 0; i < ndirs; i++) {
            int d = dirs[i];
            int cur = sq;
            while (true) {
                int nf = fileOf(cur) + (d == -9 || d == -1 || d == 7 ? -1 : d == -7 || d == 1 || d == 9 ? 1 : 0);
                int nr = rankOf(cur) + (d < -1 ? -1 : d > 1 ? 1 : 0);
                if (nf < 0 || nf > 7 || nr < 0 || nr > 7) break;
                cur = makeSquare(nf, nr);
                if (b.pieces[cur] != PieceNone) {
                    if (b.colors[cur] != us) moves.emplace_back(sq, cur);
                    break;
                }
                moves.emplace_back(sq, cur);
            }
        }
    }
}

static const int bishopDirs[] = {-9, -7, 7, 9};
static const int rookDirs[] = {-8, -1, 1, 8};
static const int queenDirs[] = {-9, -8, -7, -1, 1, 7, 8, 9};

static void addKingMoves(const Board& b, MoveList& moves) {
    Color us = b.side;
    int ksq = b.kingSquare(us);
    if (ksq == SquareNone) return;

    for (int dr = -1; dr <= 1; dr++) {
        for (int df = -1; df <= 1; df++) {
            if (dr == 0 && df == 0) continue;
            int nf = fileOf(ksq) + df, nr = rankOf(ksq) + dr;
            if (nf < 0 || nf > 7 || nr < 0 || nr > 7) continue;
            int to = makeSquare(nf, nr);
            if (b.pieces[to] != PieceNone && b.colors[to] == us) continue;
            moves.emplace_back(ksq, to);
        }
    }

    Color enemy = (us == White) ? Black : White;
    if (us == White) {
        if ((b.castleRights & 1) && b.pieces[F1] == PieceNone && b.pieces[G1] == PieceNone
            && !b.isSquareAttacked(E1, enemy) && !b.isSquareAttacked(F1, enemy) && !b.isSquareAttacked(G1, enemy))
            moves.emplace_back(E1, G1, FlagCastle);
        if ((b.castleRights & 2) && b.pieces[D1] == PieceNone && b.pieces[C1] == PieceNone && b.pieces[B1] == PieceNone
            && !b.isSquareAttacked(E1, enemy) && !b.isSquareAttacked(D1, enemy) && !b.isSquareAttacked(C1, enemy))
            moves.emplace_back(E1, C1, FlagCastle);
    } else {
        if ((b.castleRights & 4) && b.pieces[F8] == PieceNone && b.pieces[G8] == PieceNone
            && !b.isSquareAttacked(E8, enemy) && !b.isSquareAttacked(F8, enemy) && !b.isSquareAttacked(G8, enemy))
            moves.emplace_back(E8, G8, FlagCastle);
        if ((b.castleRights & 8) && b.pieces[D8] == PieceNone && b.pieces[C8] == PieceNone && b.pieces[B8] == PieceNone
            && !b.isSquareAttacked(E8, enemy) && !b.isSquareAttacked(D8, enemy) && !b.isSquareAttacked(C8, enemy))
            moves.emplace_back(E8, C8, FlagCastle);
    }
}

MoveList generatePseudoLegal(const Board& b) {
    MoveList moves;
    moves.reserve(256);
    addPawnMoves(b, moves);
    addKnightMoves(b, moves);
    addSlidingMoves(b, moves, Bishop, bishopDirs, 4);
    addSlidingMoves(b, moves, Rook, rookDirs, 4);
    addSlidingMoves(b, moves, Queen, queenDirs, 8);
    addKingMoves(b, moves);
    return moves;
}

MoveList generateLegal(Board& b) {
    MoveList pseudo = generatePseudoLegal(b);
    MoveList legal;
    legal.reserve(pseudo.size());

    for (auto& m : pseudo) {
        int capPiece = b.pieces[m.to];
        int capColor = b.colors[m.to];
        int prevEp = b.epSquare;
        int prevCastle = b.castleRights;
        int prevHalf = b.halfmove;

        b.makeMove(m);
        if (!b.inCheck(b.side == White ? Black : White))
            legal.push_back(m);
        b.unmakeMove(m, capPiece, capColor, prevEp, prevCastle, prevHalf);
    }

    return legal;
}
