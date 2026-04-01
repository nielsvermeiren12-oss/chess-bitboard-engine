#ifndef UISQUARE_H
#define UISQUARE_H

#include "domain/types.h"
struct UISquare {
    PieceType piece;
    Color color;
    bool hasMoved = false;
};

#endif // UISQUARE_H
