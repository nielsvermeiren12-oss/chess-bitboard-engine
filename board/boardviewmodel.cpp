#include "boardviewmodel.h"
#include <domain/movegenerator.h>
#include <domain/position.h>
#include "domain/bitboard.h"
#include <QtConcurrent>
#include <AI/minimax.h>


BoardViewModel::BoardViewModel(QObject *parent)
    : QObject(parent),
    m_position()   // Default constructor sets sideToMove = WHITE
{
    m_board.resize(64);
    activeMoves.clear();
    syncFromPosition();
}

QVariantList BoardViewModel::board() const
{
    return m_board;
}

void BoardViewModel::syncFromPosition()
{
    m_board.clear();

    for (int sq = 0; sq < 64; ++sq) {
        QVariantMap squareMap;
        squareMap["piece"] = -1;
        squareMap["color"] = -1;

        int square = (7 - std::floor(sq / 8)) * 8 + (sq % 8);

        for (int pt = 0; pt < PIECE_NB; ++pt) {
            if (getBit(m_position.pieces[WHITE][pt], square)) {
                squareMap["piece"] = pt;
                squareMap["color"] = static_cast<int>(WHITE);
            } else if (getBit(m_position.pieces[BLACK][pt], square)) {
                squareMap["piece"] = pt;
                squareMap["color"] = static_cast<int>(BLACK);
            }
        }

        m_board.append(squareMap);
    }

    emit boardChanged();
}


QString BoardViewModel::pieceToUnicode(int piece) const
{
    static const QMap<int, QString> map = {
        {PAWN, "♟"},
        {KNIGHT, "♞"},
        {BISHOP, "♝"},
        {ROOK, "♜"},
        {QUEEN, "♛"},
        {KING, "♚"}
    };

    return map.value({piece}, "");
}

QString BoardViewModel::getPieceColor(int color) const
{
    return color == WHITE ? "white" : "black";
}

int BoardViewModel::sideToMove() const {
    return static_cast<int>(m_position.sideToMove);
}

QVariantList BoardViewModel::possibleMovesFrom(int qmlIndex)
{
    QVariantList result;

    // Map QML index back to bitboard square
    int square = 63 - qmlIndex;

    Color stm = m_position.sideToMove;

    // Check if clicked square belongs to sideToMove
    bool ownsPiece = false;
    for (int pt = 0; pt < PIECE_NB; ++pt) {
        if (getBit(m_position.pieces[stm][pt], square)) {
            ownsPiece = true;
            break;
        }
    }

    if (!ownsPiece) {
        activeMoves.clear();
        qDebug() << "Clicked piece does not belong to sideToMove!";
        return result;
    }

    // Generate legal moves from this square
    MoveGenerator movGen;
    activeMoves = movGen.generateLegalMovesFromSquare(m_position, square);

    for (const Move& m : activeMoves) {
        QVariantMap move;
        move["index"] = 63 - m.to;
        move["isPromotion"] = m.isPromotion();
        // Map bitboard target square to QML index
        result.append(move);
    }

    return result;
}
void BoardViewModel::makeMove(int qmlIndex)
{
    int square = 63 - qmlIndex;

    for (const Move& m : activeMoves) {
        if (m.to == square) {
            Undo u;
            m_position.makeMove(m, u);
            activeMoves.clear();
            syncFromPosition();

            emit sideToMoveChanged();  // <--- update QML
            break;
        }
    }
    makeComputerMove();
}

void BoardViewModel::makeMove(int qmlIndex, int piece)
{
    int square = 63 - qmlIndex;

    for(const Move& m : activeMoves) {
        if(m.to == square && m.promotion == piece) {
            Undo u;
            m_position.makeMove(m, u);
            activeMoves.clear();
            syncFromPosition();

            emit sideToMoveChanged();  // <--- update QML
            break;
        }
    }
    makeComputerMove();
}

void BoardViewModel::resetGame()
{
    Position p;
    m_position = p;
    syncFromPosition();
}

void BoardViewModel::makeComputerMove()
{
    Position posCopy = m_position;
    // m_computerFuture = QtConcurrent::run([posCopy]() {
        Position pos = posCopy;  // non-const copy for Minimax(Position&)
        Minimax minimax(pos);
        Move m = minimax.minimaxRoot(3);
        Undo u;
        m_position.makeMove(m, u);
    // });
    // m_computerWatcher.setFuture(m_computerFuture);

        syncFromPosition();
        emit sideToMoveChanged();
}
