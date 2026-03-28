#include "uci.hpp"
#include "movegen.hpp"
#include "search.hpp"
#include <iostream>
#include <sstream>

static Board board;

static Move parseMove(Board& b, const std::string& s) {
    if (s.size() < 4) return Move();
    int from = strToSquare(s.substr(0, 2));
    int to = strToSquare(s.substr(2, 2));
    int flag = FlagNone;

    if (s.size() == 5) {
        switch (s[4]) {
            case 'q': flag = FlagPromoQ; break;
            case 'r': flag = FlagPromoR; break;
            case 'b': flag = FlagPromoB; break;
            case 'n': flag = FlagPromoN; break;
        }
    }

    MoveList legal = generateLegal(b);
    for (auto& m : legal) {
        if (m.from == from && m.to == to) {
            if (flag != FlagNone) {
                if (m.flag == flag) return m;
            } else if (m.flag < FlagPromoN || m.flag > FlagPromoQ) {
                return m;
            }
        }
    }
    return Move();
}

static void handlePosition(std::istringstream& ss) {
    std::string token;
    ss >> token;

    if (token == "startpos") {
        board.setFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        ss >> token;
    } else if (token == "fen") {
        std::string fen;
        for (int i = 0; i < 6 && ss >> token; i++) {
            if (i > 0) fen += " ";
            fen += token;
        }
        board.setFromFen(fen);
        ss >> token;
    }

    if (token == "moves") {
        while (ss >> token) {
            Move m = parseMove(board, token);
            if (!m.isNull()) {
                board.makeMove(m);
            }
        }
    }
}

static void handleGo(std::istringstream& ss) {
    int depth = 5;
    std::string token;
    while (ss >> token) {
        if (token == "depth") ss >> depth;
        else if (token == "movetime") {
            int mt; ss >> mt;
            if (mt < 500) depth = 3;
            else if (mt < 2000) depth = 4;
            else if (mt < 10000) depth = 5;
            else depth = 6;
        } else if (token == "wtime" || token == "btime") {
            int t; ss >> t;
            if (t < 5000) depth = 3;
            else if (t < 30000) depth = 4;
            else depth = 5;
        } else if (token == "winc" || token == "binc" || token == "movestogo") {
            int dummy; ss >> dummy;
        }
    }

    SearchResult result = search(board, depth);
    std::cout << "bestmove " << moveToStr(result.bestMove) << std::endl;
}

void uciLoop() {
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream ss(line);
        std::string cmd;
        ss >> cmd;

        if (cmd == "uci") {
            std::cout << "id name Rook" << std::endl;
            std::cout << "id author ilya" << std::endl;
            std::cout << "uciok" << std::endl;
        } else if (cmd == "isready") {
            std::cout << "readyok" << std::endl;
        } else if (cmd == "ucinewgame") {
            board = Board();
        } else if (cmd == "position") {
            handlePosition(ss);
        } else if (cmd == "go") {
            handleGo(ss);
        } else if (cmd == "quit") {
            break;
        }
    }
}
