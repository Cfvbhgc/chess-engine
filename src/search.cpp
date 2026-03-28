#include "search.hpp"
#include "movegen.hpp"
#include "eval.hpp"
#include <algorithm>

static const int mvvLva[6] = {100, 320, 330, 500, 900, 20000};

static int quiesce(Board& b, int alpha, int beta) {
    int stand = evaluate(b);
    if (stand >= beta) return beta;
    if (stand > alpha) alpha = stand;

    MoveList moves = generateLegal(b);
    for (auto& m : moves) {
        bool isCapture = b.pieces[m.to] != PieceNone || m.flag == FlagEP;
        if (!isCapture) continue;

        int capP = b.pieces[m.to];
        int capC = b.colors[m.to];
        int prevEp = b.epSquare;
        int prevCastle = b.castleRights;
        int prevHalf = b.halfmove;

        b.makeMove(m);
        int score = -quiesce(b, -beta, -alpha);
        b.unmakeMove(m, capP, capC, prevEp, prevCastle, prevHalf);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

static int alphaBeta(Board& b, int depth, int alpha, int beta) {
    if (depth <= 0) return quiesce(b, alpha, beta);

    MoveList moves = generateLegal(b);

    if (moves.empty()) {
        if (b.inCheck(b.side)) return -MATE_SCORE + (100 - depth);
        return 0;
    }

    std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& c) {
        auto sc = [&](const Move& m) {
            int s = 0;
            if (b.pieces[m.to] != PieceNone)
                s += 10 * mvvLva[b.pieces[m.to]] - mvvLva[b.pieces[m.from]];
            if (m.flag >= FlagPromoN && m.flag <= FlagPromoQ) s += 9000;
            return s;
        };
        return sc(a) > sc(c);
    });

    for (auto& m : moves) {
        int capP = b.pieces[m.to];
        int capC = b.colors[m.to];
        int prevEp = b.epSquare;
        int prevCastle = b.castleRights;
        int prevHalf = b.halfmove;

        b.makeMove(m);
        int score = -alphaBeta(b, depth - 1, -beta, -alpha);
        b.unmakeMove(m, capP, capC, prevEp, prevCastle, prevHalf);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

SearchResult search(Board& b, int depth) {
    MoveList moves = generateLegal(b);
    if (moves.empty()) return {Move(), 0};

    Move best = moves[0];
    int bestScore = -INF_SCORE;

    for (auto& m : moves) {
        int capP = b.pieces[m.to];
        int capC = b.colors[m.to];
        int prevEp = b.epSquare;
        int prevCastle = b.castleRights;
        int prevHalf = b.halfmove;

        b.makeMove(m);
        int score = -alphaBeta(b, depth - 1, -INF_SCORE, -bestScore);
        b.unmakeMove(m, capP, capC, prevEp, prevCastle, prevHalf);

        if (score > bestScore) {
            bestScore = score;
            best = m;
        }
    }
    return {best, bestScore};
}
