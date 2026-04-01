#ifndef UNDO_H
#define UNDO_H

#include "domain/move.h"


struct Undo {
    int from;
    int to;

    PieceType movedPiece;
    PieceType capturedPiece;
    PieceType promotionPiece;

    uint64_t capturedBitboard; // for EP
    int capturedSquare;

    bool wasPromotion;
    bool castleWK, castleWQ, castleBK, castleBQ;
    int enPassantSquare;
    int halfmoveClock;

};


#endif // UNDO_H
