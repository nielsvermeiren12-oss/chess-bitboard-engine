#ifndef BOARDVIEWMODEL_H
#define BOARDVIEWMODEL_H


#include <QObject>
#include <QVariantList>
#include <QFuture>
#include <QFutureWatcher>
#include <domain/position.h>
#include <domain/move.h>

class BoardViewModel : public QObject
{

    Q_OBJECT
    Q_PROPERTY(QVariantList board READ board NOTIFY boardChanged)
    Q_PROPERTY(int sideToMove READ sideToMove NOTIFY sideToMoveChanged)
    public:
        explicit BoardViewModel(QObject* parent = nullptr);
        QVariantList board() const;
        void syncFromPosition();
        Q_INVOKABLE QString pieceToUnicode(int piece) const;
        Q_INVOKABLE QString getPieceColor(int color) const;
        Q_INVOKABLE QVariantList possibleMovesFrom(int square);
        Q_INVOKABLE void makeMove(int square);
        Q_INVOKABLE void makeMove(int square, int piece);
        Q_INVOKABLE void resetGame();
        Q_INVOKABLE void makeComputerMove();
        int sideToMove() const;

    private:
        QVariantList  m_board;
        std::vector<Move> activeMoves;
        Position m_position;
        QFuture<Move> m_computerFuture;
        QFutureWatcher<Move> m_computerWatcher;
    signals:
        void boardChanged();
        void sideToMoveChanged();
};

#endif // BOARDVIEWMODEL_H
