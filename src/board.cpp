#include "board.hpp"
#include <sstream>

Board::Board() { setFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"); }

void Board::clear() {
    pieces.fill(PieceNone);
    colors.fill(ColorNone);
    for (int c = 0; c < 2; c++) for (int p = 0; p < 7; p++) pieceBB[c][p] = 0;
    occupancy[0] = occupancy[1] = allOccupancy = 0;
    side = White;
    epSquare = SquareNone;
    castleRights = 0;
    halfmove = fullmove = 0;
}

void Board::putPiece(int sq, Color c, Piece p) {
    pieces[sq] = p;
    colors[sq] = c;
    pieceBB[c][p] |= bit(sq);
    occupancy[c] |= bit(sq);
    allOccupancy |= bit(sq);
}

void Board::removePiece(int sq) {
    if (pieces[sq] == PieceNone) return;
    int c = colors[sq], p = pieces[sq];
    pieceBB[c][p] &= ~bit(sq);
    occupancy[c] &= ~bit(sq);
    allOccupancy &= ~bit(sq);
    pieces[sq] = PieceNone;
    colors[sq] = ColorNone;
}

void Board::updateBitboards() {
    for (int c = 0; c < 2; c++) {
        occupancy[c] = 0;
        for (int p = 0; p < 6; p++) {
            pieceBB[c][p] = 0;
        }
        pieceBB[c][6] = 0;
    }
    allOccupancy = 0;
    for (int sq = 0; sq < 64; sq++) {
        if (pieces[sq] != PieceNone) {
            int c = colors[sq], p = pieces[sq];
            pieceBB[c][p] |= bit(sq);
            occupancy[c] |= bit(sq);
            allOccupancy |= bit(sq);
        }
    }
}

void Board::setFromFen(const std::string& fen) {
    clear();
    std::istringstream ss(fen);
    std::string board, sideStr, castleStr, epStr;
    ss >> board >> sideStr >> castleStr >> epStr >> halfmove >> fullmove;

    int sq = 56;
    for (char ch : board) {
        if (ch == '/') { sq -= 16; continue; }
        if (ch >= '1' && ch <= '8') { sq += ch - '0'; continue; }
        Color c = (ch >= 'A' && ch <= 'Z') ? White : Black;
        char lower = ch | 32;
        Piece p = PieceNone;
        switch (lower) {
            case 'p': p = Pawn; break;
            case 'n': p = Knight; break;
            case 'b': p = Bishop; break;
            case 'r': p = Rook; break;
            case 'q': p = Queen; break;
            case 'k': p = King; break;
        }
        if (p != PieceNone) putPiece(sq, c, p);
        sq++;
    }

    side = (sideStr == "w") ? White : Black;

    castleRights = 0;
    if (castleStr != "-") {
        for (char ch : castleStr) {
            if (ch == 'K') castleRights |= 1;
            if (ch == 'Q') castleRights |= 2;
            if (ch == 'k') castleRights |= 4;
            if (ch == 'q') castleRights |= 8;
        }
    }

    epSquare = (epStr != "-") ? strToSquare(epStr) : SquareNone;
}

std::string Board::toFen() const {
    std::string fen;
    const char pieceChars[] = "pnbrqk";
    for (int r = 7; r >= 0; r--) {
        int empty = 0;
        for (int f = 0; f < 8; f++) {
            int sq = makeSquare(f, r);
            if (pieces[sq] == PieceNone) { empty++; continue; }
            if (empty) { fen += std::to_string(empty); empty = 0; }
            char ch = pieceChars[pieces[sq]];
            if (colors[sq] == White) ch &= ~32;
            fen += ch;
        }
        if (empty) fen += std::to_string(empty);
        if (r > 0) fen += '/';
    }
    fen += (side == White) ? " w " : " b ";
    std::string cs;
    if (castleRights & 1) cs += 'K';
    if (castleRights & 2) cs += 'Q';
    if (castleRights & 4) cs += 'k';
    if (castleRights & 8) cs += 'q';
    fen += cs.empty() ? "-" : cs;
    fen += " ";
    fen += (epSquare != SquareNone) ? squareToStr(epSquare) : "-";
    fen += " " + std::to_string(halfmove) + " " + std::to_string(fullmove);
    return fen;
}

void Board::makeMove(const Move& m) {
    int piece = pieces[m.from];
    int col = colors[m.from];

    removePiece(m.to);
    Piece movingPiece = static_cast<Piece>(piece);

    if (m.flag == FlagPromoN) movingPiece = Knight;
    else if (m.flag == FlagPromoB) movingPiece = Bishop;
    else if (m.flag == FlagPromoR) movingPiece = Rook;
    else if (m.flag == FlagPromoQ) movingPiece = Queen;

    removePiece(m.from);
    putPiece(m.to, static_cast<Color>(col), movingPiece);

    if (m.flag == FlagEP) {
        int capSq = (col == White) ? m.to - 8 : m.to + 8;
        removePiece(capSq);
    }

    if (m.flag == FlagCastle) {
        if (m.to == G1) { removePiece(H1); putPiece(F1, White, Rook); }
        else if (m.to == C1) { removePiece(A1); putPiece(D1, White, Rook); }
        else if (m.to == G8) { removePiece(H8); putPiece(F8, Black, Rook); }
        else if (m.to == C8) { removePiece(A8); putPiece(D8, Black, Rook); }
    }

    epSquare = SquareNone;
    if (m.flag == FlagDoublePawn)
        epSquare = (col == White) ? m.from + 8 : m.from - 8;

    if (m.from == E1 || m.to == E1) castleRights &= ~3;
    if (m.from == E8 || m.to == E8) castleRights &= ~12;
    if (m.from == H1 || m.to == H1) castleRights &= ~1;
    if (m.from == A1 || m.to == A1) castleRights &= ~2;
    if (m.from == H8 || m.to == H8) castleRights &= ~4;
    if (m.from == A8 || m.to == A8) castleRights &= ~8;

    if (piece == Pawn || pieces[m.to] != PieceNone) halfmove = 0;
    else halfmove++;

    if (col == Black) fullmove++;
    side = (side == White) ? Black : White;
}

void Board::unmakeMove(const Move& m, int capPiece, int capColor, int prevEp, int prevCastle, int prevHalf) {
    side = (side == White) ? Black : White;
    int col = static_cast<int>(side);

    Piece movingPiece = static_cast<Piece>(pieces[m.to]);
    if (m.flag >= FlagPromoN && m.flag <= FlagPromoQ)
        movingPiece = Pawn;

    removePiece(m.to);
    putPiece(m.from, static_cast<Color>(col), movingPiece);

    if (capPiece != PieceNone && m.flag != FlagEP)
        putPiece(m.to, static_cast<Color>(capColor), static_cast<Piece>(capPiece));

    if (m.flag == FlagEP) {
        int capSq = (col == White) ? m.to - 8 : m.to + 8;
        putPiece(capSq, (col == White) ? Black : White, Pawn);
    }

    if (m.flag == FlagCastle) {
        if (m.to == G1) { removePiece(F1); putPiece(H1, White, Rook); }
        else if (m.to == C1) { removePiece(D1); putPiece(A1, White, Rook); }
        else if (m.to == G8) { removePiece(F8); putPiece(H8, Black, Rook); }
        else if (m.to == C8) { removePiece(D8); putPiece(A8, Black, Rook); }
    }

    epSquare = prevEp;
    castleRights = prevCastle;
    halfmove = prevHalf;
    if (col == Black) fullmove--;
}

static const int knightOffsets[] = {-17, -15, -10, -6, 6, 10, 15, 17};
static const int bishopDirs[] = {-9, -7, 7, 9};
static const int rookDirs[] = {-8, -1, 1, 8};

bool Board::isSquareAttacked(int sq, Color by) const {
    int pawnDir = (by == White) ? -1 : 1;
    int pr = rankOf(sq) + pawnDir;
    if (pr >= 0 && pr < 8) {
        int pf = fileOf(sq);
        if (pf > 0) {
            int ps = makeSquare(pf - 1, pr);
            if (pieces[ps] == Pawn && colors[ps] == by) return true;
        }
        if (pf < 7) {
            int ps = makeSquare(pf + 1, pr);
            if (pieces[ps] == Pawn && colors[ps] == by) return true;
        }
    }

    for (int off : knightOffsets) {
        int to = sq + off;
        if (to < 0 || to > 63) continue;
        int fd = abs(fileOf(to) - fileOf(sq));
        int rd = abs(rankOf(to) - rankOf(sq));
        if (!((fd == 1 && rd == 2) || (fd == 2 && rd == 1))) continue;
        if (pieces[to] == Knight && colors[to] == by) return true;
    }

    for (int dir : bishopDirs) {
        int cur = sq;
        while (true) {
            int nf = fileOf(cur) + (dir == -9 || dir == 7 ? -1 : 1);
            int nr = rankOf(cur) + (dir < 0 ? -1 : 1);
            if (nf < 0 || nf > 7 || nr < 0 || nr > 7) break;
            cur = makeSquare(nf, nr);
            if (pieces[cur] != PieceNone) {
                if (colors[cur] == by && (pieces[cur] == Bishop || pieces[cur] == Queen))
                    return true;
                break;
            }
        }
    }

    for (int dir : rookDirs) {
        int cur = sq;
        while (true) {
            int nf = fileOf(cur) + (dir == -1 ? -1 : dir == 1 ? 1 : 0);
            int nr = rankOf(cur) + (dir == -8 ? -1 : dir == 8 ? 1 : 0);
            if (nf < 0 || nf > 7 || nr < 0 || nr > 7) break;
            cur = makeSquare(nf, nr);
            if (pieces[cur] != PieceNone) {
                if (colors[cur] == by && (pieces[cur] == Rook || pieces[cur] == Queen))
                    return true;
                break;
            }
        }
    }

    for (int dr = -1; dr <= 1; dr++) {
        for (int df = -1; df <= 1; df++) {
            if (dr == 0 && df == 0) continue;
            int nf = fileOf(sq) + df, nr = rankOf(sq) + dr;
            if (nf < 0 || nf > 7 || nr < 0 || nr > 7) continue;
            int to = makeSquare(nf, nr);
            if (pieces[to] == King && colors[to] == by) return true;
        }
    }

    return false;
}

bool Board::inCheck(Color c) const {
    return isSquareAttacked(kingSquare(c), c == White ? Black : White);
}

int Board::kingSquare(Color c) const {
    for (int sq = 0; sq < 64; sq++)
        if (pieces[sq] == King && colors[sq] == c) return sq;
    return SquareNone;
}
