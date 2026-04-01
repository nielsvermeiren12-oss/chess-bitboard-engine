#include "minimax.h"

#include "boardevaluator.h"

#include <domain/movegenerator.h>

Minimax::Minimax(Position& position) {
    this->m_position = position;
}



Move Minimax::minimaxRoot(int depth)
{
    std::vector<Move> moves = getNextMoves();

    // No legal moves (checkmate/stalemate) - caller must not apply the returned move
    if (moves.empty()) {
        Move dummyMove;
        return dummyMove;
    }

    // Computer plays Black. Evaluation is from White's perspective (positive = White winning).
    // So Black minimizes the evaluation.
    int bestValue = 9999;
    Move bestNextMove = moves[0];
    int i=0;
    for (Move& m : moves) {
        i++;
        Undo undo;
        m_position.makeMove(m, undo);
        if (m.isPromotion()) {
            printBitboard(m_position.pieces[0][QUEEN]);
        }
        int value = minimax(depth - 1, -10000, 10000);  // true = next is White (maximizer)
        m_position.undoMove(m, undo);
        if (m.isPromotion()) {
            printBitboard(m_position.pieces[0][QUEEN]);
        }
        if (value < bestValue) {
            bestValue = value;
            bestNextMove = m;
        }
    }
    return bestNextMove;
}

int Minimax::quiescence(int alpha, int beta, int depth)
{
    int stand_pat = BoardEvaluator::evaluateBoard(m_position);



    if (stand_pat >= beta) {
        return beta;
    }

    if (stand_pat > alpha) {
        alpha = stand_pat;
    }

    if (depth == 0) {
        return stand_pat;
    }

    std::vector<Move> moves = getNextMoves();

    for (Move& m : moves) {
        if (!m.isCapture())
            continue;

        Undo undo;
        m_position.makeMove(m, undo);

        int score = -quiescence(-beta, -alpha, depth -1);

        m_position.undoMove(m, undo);

        if (score >= beta) {
            return beta;
        }

        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}



int Minimax::minimax(int depth, int alpha, int beta)
{
    if(depth == 0) {
        return quiescence(alpha, beta, 1);
    }

    std::vector<Move> moves = getNextMoves();
    // Move ordering: captures/promotions first
    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
        int scoreA = 0;
        int scoreB = 0;

        if (a.isCapture()) {
            scoreA = BoardEvaluator::pieceValue(a.captured) - BoardEvaluator::pieceValue(a.piece);
        }

        if (b.isCapture()) {
            scoreB = BoardEvaluator::pieceValue(b.captured) - BoardEvaluator::pieceValue(b.piece);
        }

        return scoreA > scoreB;
    });

    if(moves.empty()) {
        Color stm = m_position.sideToMove;
        if(m_position.inCheck(stm)) return (stm == WHITE ? -10000 : 10000);
        return 0; // stalemate
    }

    bool isMax = (m_position.sideToMove == WHITE);
    if(isMax) {
        int best = -100000;
        for(Move& m : moves) {
            Undo undo;
            m_position.makeMove(m, undo);
            int eval = minimax(depth - 1, alpha, beta);
            m_position.undoMove(m, undo);
            best = std::max(best, eval);
            alpha = std::max(alpha, eval);
            if(beta <= alpha) break;
        }
        return best;
    } else {
        int best = 100000;
        for(Move& m : moves) {
            Undo undo;
            m_position.makeMove(m, undo);
            int eval = minimax(depth - 1, alpha, beta);
            m_position.undoMove(m, undo);
            best = std::min(best, eval);
            beta = std::min(beta, eval);
            if(beta <= alpha) break;
        }
        return best;
    }
}
