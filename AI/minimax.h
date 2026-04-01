#ifndef MINIMAX_H
#define MINIMAX_H

#include <QHash>
#include <domain/movegenerator.h>
#include <domain/position.h>

class Minimax {
    public:
        Minimax(Position& position);
        Move minimaxRoot(int depth);
        int minimax(int depth, int alpha, int beta);
        std::vector<Move> getNextMoves(){
            std::vector<Move> moves;
            moves.reserve(256);
            MoveGenerator generator;
            generator.generateLegalMoves(m_position, moves);
            return moves;
        }
        int quiescence(int alpha, int beta, int depth);
    private:
        Position m_position;
};

#endif // MINIMAX_H
