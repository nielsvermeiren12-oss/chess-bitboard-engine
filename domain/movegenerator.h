#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include "domain/attack_tables.h"
#include "domain/bitboard.h"
#include "domain/move.h"
#include "domain/position.h"

#include <cstdint>
#include <vector>
using Bitboard = uint64_t;
using AttackFn = Bitboard (*)(int, Bitboard);

class MoveGenerator
{

public:
    MoveGenerator();
    void generateKnightMoves(Position& position, std::vector<Move>& moves);
    void generatePawnMoves(Position& position, std::vector<Move>& moves);
    void generateKingMoves(Position& position, std::vector<Move>& moves);
    void generateSlidingMoves(Position& position, std::vector<Move>& moves, PieceType piece, AttackFn attacksFn);
    void generateRookMoves(Position& position, std::vector<Move>& moves) {
        generateSlidingMoves(position, moves, ROOK, rookAttacks);
    };
    void generateBishopMoves(Position& position, std::vector<Move>& moves) {
        generateSlidingMoves(position, moves, BISHOP, bishopAttacks);
    };
    void generateQueenMoves(Position& position, std::vector<Move>& moves){
        generateSlidingMoves(position, moves, QUEEN, queenAttacks);
    };
    void generateLegalMoves(Position& position, std::vector<Move>& moves);
    std::vector<Move> generateLegalMovesFromSquare(Position& pos, int fromSquare);


    std::vector<Move> generatePseudoMovesFromSquare(const Position& pos, uint8_t from);

    PieceType getCapturedPiece(const Position& pos, uint8_t sq, Color them) {
        for (int pt = PAWN; pt < PIECE_NB; ++pt) {
            if (getBit(pos.pieces[them][pt], sq))
                return static_cast<PieceType>(pt);
        }
        return PIECE_NB; // should never happen
    }

};

#endif // MOVEGENERATOR_H
