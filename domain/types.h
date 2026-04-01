#ifndef TYPES_H
#define TYPES_H

#pragma once
#include <cstdint>

using Bitboard = uint64_t;

enum Color : uint8_t {
    WHITE = 1,
    BLACK = 0
};

inline Color opposite(Color c) {
    return c == WHITE ? BLACK : WHITE;
}

enum PieceType : uint8_t {
    PAWN = 0,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    PIECE_NB
};

enum PieceScore : int {
    PAWN_SCORE   = 100,
    KNIGHT_SCORE = 320,
    BISHOP_SCORE = 330,
    ROOK_SCORE   = 500,
    QUEEN_SCORE  = 900,
    KING_SCORE   = 20000
};

#endif // TYPES_H
