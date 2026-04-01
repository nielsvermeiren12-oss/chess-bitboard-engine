#include "boardevaluator.h"
#include <domain/position.h>
#include <qdebug.h>

std::vector<int> pawnWhiteTable = {
    100,100,100,100,100,100,100,100,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5,  5, 10, 25, 25, 10,  5,  5,
    0,  0,  0, 50, 50,  0,  0,  0,
    5, -5,-10,  0,  0,-10, -5,  5,
    5, 10, 10,-20,-20, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0
};

std::vector<int> pawnBlackTable = {
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10,-20,-20, 10, 10,  5,
    5, -5,  0,  0,  0,-10, -5,  5,
    0,  0,  0, 50, 80,  0,  0,  0,
    5,  5, 10, 55, 55, 10,  5,  5,
    10, 10, 20, 30, 30, 20, 10, 10,
    50, 50, 50, 50, 50, 50, 50, 50,
    100,100,100,100,100,100,100,100
};


std::vector<int> knightTable = {
    -50,-100,-30,-30,-30,-30,-50,-50,
    -40,-20,-20,-30,-30,-20,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-50,-30,-30,-30,-30,-50,-50
};

std::vector<int> bishopWhiteTable = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

std::vector<int> bishopBlackTable = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};


std::vector<int> rookWhiteTable = {
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10, 10, 10, 10, 10,  5,
    5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    0,  0,  0,  5,  5,  0,  0,  0
};

std::vector<int> rookBlackTable = {
    0,  0,  0,  5,  5,  0,  0,  0,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    5, 10, 10, 10, 10, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0
};

std::vector<int> queenTable = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0, 25, 25, 25, 25,  0,-10,
    -5,  0, 25, 25, 25, 25,  0, -5,
    0,  0, 25, 25, 25, 25,  0, -5,
    -10, 25, 25, 25, 25, 25,  0,-10,
    -10,  0, 25,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};


std::vector<int> kingWhiteTable = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
    20, 20,  0,  0,  0,  0, 20, 20,
    20, 30, 10,  0,  0, 10, 30, 20
};

std::vector<int> kingBlackTable = {
    20, 30, 10,  0,  0, 10, 30, 20,
    20, 20,  0,  0,  0,  0, 20, 20,
    -10,-20,-20,-20,-20,-20,-20,-10,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30
};


int BoardEvaluator::evaluateBoard(const Position position)
{
    int score = 0;

    for (int sq = 0; sq < 64; sq++)
    {
        Piece piece = position.getPieceAt(sq);
        if (piece.type == PIECE_NB) {
            continue;
        }
        int value = 0;

        switch (piece.type) {
            case PAWN:
                value = pieceValue(piece.type);
                value += piece.colour == WHITE ? pawnWhiteTable[sq] : pawnBlackTable[sq];
                break;

            case KNIGHT:
                value = pieceValue(piece.type);
                value += knightTable[sq];
                break;

            case BISHOP:
                value = pieceValue(piece.type);
                value += piece.colour == WHITE ? bishopWhiteTable[sq] : bishopBlackTable[sq];
                break;

            case ROOK:
                value = pieceValue(piece.type);
                value += piece.colour == WHITE ? rookWhiteTable[sq] : rookBlackTable[sq];
                break;

            case QUEEN:
                value = pieceValue(piece.type);
                value += queenTable[sq];
                break;

            case KING:
                value = pieceValue(piece.type);
                value += piece.colour == WHITE ? kingWhiteTable[sq] : kingBlackTable[sq];
                break;

            default:
                break;
        }

        score += (piece.colour == WHITE ? value : -value);
    }

    if (std::_Popcount(position.piecesOf(WHITE, BISHOP)) >= 2) {
        score += 30;
    }
    if (std::_Popcount(position.piecesOf(BLACK, BISHOP)) >= 2) {
        score -= 30;
    }

    return (position.sideToMove == WHITE ? score : -score);
}

int BoardEvaluator::pieceValue(PieceType pieceType)
{
    switch(pieceType) {
        case PAWN:   return 100;
        case KNIGHT: return 320;
        case BISHOP: return 330;
        case ROOK:   return 500;
        case QUEEN:  return 900;
        case KING:   return 20000;
        default:     return 0;
    }
}
