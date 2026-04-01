#ifndef POSITION_H
#define POSITION_H
#include "domain/move.h"
#include "domain/types.h"
#include "domain/piece.h"
#include "domain/undo.h"
#include <cstdint>

using Bitboard = uint64_t;

class Position {
public:
    Bitboard pieces[2][PIECE_NB];
    Bitboard occupancy[2];
    Bitboard all;
    Color sideToMove;
    int enPassantSquare;
    bool castleWK, castleWQ, castleBK, castleBQ;

    Position();

    Bitboard piecesOf(Color c) const { return occupancy[c]; }
    Bitboard piecesOf(Color c, PieceType p) const { return pieces[c][p]; }
    bool squareOccupied(int sq) const { return all & (1ULL << sq); }

    void updateOccupancy();
    bool isSquareAttacked(int sq, Color byColor) const;
    bool inCheck(Color c) const;

    void removePieceAt(Color c, uint8_t square);
    bool canCastleKingSide(Color us) const;
    bool canCastleQueenSide(Color us) const;

    void movePiece(Color c, PieceType p, int from, int to);
    void makeCastle(Color us, bool kingSide);

    void makeMove(const Move& m, Undo& u);
    void undoMove(const Move& m, const Undo& u);

    Piece getPieceAt(int sq) const;

    Bitboard knightAttacksFor(int sq) const;
    Bitboard kingAttacksFor(int sq) const;
private:

    Bitboard bishopAttacks(int sq, Bitboard blockers) const;
    Bitboard rookAttacks(int sq, Bitboard blockers) const;
};


#endif // POSITION_H
