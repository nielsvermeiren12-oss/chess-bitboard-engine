#ifndef MOVE_H
#define MOVE_H

#include "domain/types.h"
#include <cstdint>

#ifndef Q_MOC_RUN
enum class MoveFlag : uint8_t {
    NONE        = 0,
    CAPTURE     = 1 << 0,
    DOUBLE_PUSH = 1 << 1,
    EN_PASSANT  = 1 << 2,
    CASTLING    = 1 << 3,
    PROMOTION   = 1 << 4
};

inline MoveFlag operator|(MoveFlag a, MoveFlag b) {
    return static_cast<MoveFlag>(
        static_cast<uint8_t>(a) | static_cast<uint8_t>(b)
        );
}

inline MoveFlag operator&(MoveFlag a, MoveFlag b) {
    return static_cast<MoveFlag>(
        static_cast<uint8_t>(a) & static_cast<uint8_t>(b)
        );
}



struct Move {
    uint8_t from = 0;
    uint8_t to = 0;
    MoveFlag flags = MoveFlag::NONE;                // MoveFlag bitmask
    PieceType piece = PIECE_NB;              // moving piece
    PieceType captured = PIECE_NB; // captured piece
    PieceType promotion = PIECE_NB; // only valid if PROMOTION flag set


    // ---- helpers ----
    bool isCapture() const {
        return (flags & MoveFlag::CAPTURE) != MoveFlag::NONE;
    }

    bool isPromotion() const {
        return (flags & MoveFlag::PROMOTION) != MoveFlag::NONE;
    }

    bool isCastling() const {
        return (flags & MoveFlag::CASTLING) != MoveFlag::NONE;
    }

    bool isEnPassant() const {
        return (flags & MoveFlag::EN_PASSANT) != MoveFlag::NONE;
    }

    bool isKingSideCastle() const {
        return isCastling() && to > from;
    }

};
#endif // Q_MOC_RUN


#endif // MOVE_H
