#ifndef BOARDEVALUATOR_H
#define BOARDEVALUATOR_H

#include <domain/position.h>



class BoardEvaluator {
    public:
        BoardEvaluator();
        static int evaluateBoard(const Position position);
        static int pieceValue(PieceType piece);
        static bool isHanging(const Position pos, int sq, Piece piece);
};

#endif // BOARDEVALUATOR_H
