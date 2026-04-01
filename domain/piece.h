#ifndef PIECE_H
#define PIECE_H
#include "domain/types.h"
#include <QString>

struct Piece {
    PieceType type;    // "♟", "♞", "♜", etc.
    Color colour;  // "white" or "black"
};

#endif // PIECE_H
