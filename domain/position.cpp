#include "position.h"
#include "domain/bitboard.h"
#include <cstdint>
#include <intrin.h>
#include <QDebug>
#include <cmath>

Position::Position() {
    for(int c=0;c<2;++c)
        for(int p=0;p<PIECE_NB;++p)
            pieces[c][p] = 0ULL;

    // White pieces
    pieces[WHITE][PAWN]   = 0x000000000000FF00ULL;
    pieces[WHITE][ROOK]   = 0x0000000000000081ULL;
    pieces[WHITE][KNIGHT] = 0x0000000000000042ULL;
    pieces[WHITE][BISHOP] = 0x0000000000000024ULL;
    pieces[WHITE][QUEEN]  = 0x0000000000000008ULL;
    pieces[WHITE][KING]   = 0x0000000000000010ULL;

    // Black pieces
    pieces[BLACK][PAWN]   = 0x00FF000000000000ULL;
    pieces[BLACK][ROOK]   = 0x8100000000000000ULL;
    pieces[BLACK][KNIGHT] = 0x4200000000000000ULL;
    pieces[BLACK][BISHOP] = 0x2400000000000000ULL;
    pieces[BLACK][QUEEN]  = 0x0800000000000000ULL;
    pieces[BLACK][KING]   = 0x1000000000000000ULL;

    sideToMove = WHITE;
    enPassantSquare = -1;
    castleWK = castleWQ = castleBK = castleBQ = true;

    updateOccupancy();
}

// --------------------- Occupancy ---------------------
void Position::updateOccupancy() {
    occupancy[WHITE] = occupancy[BLACK] = 0ULL;
    for(int p=0;p<PIECE_NB;++p) {
        occupancy[WHITE] |= pieces[WHITE][p];
        occupancy[BLACK] |= pieces[BLACK][p];
    }
    all = occupancy[WHITE] | occupancy[BLACK];
}

// --------------------- Sliding Attacks ---------------------
Bitboard Position::bishopAttacks(int sq, Bitboard blockers) const {
    Bitboard attacks = 0ULL;

    const int directions[4][2] = {
        { 1,  1},  // NE
        { 1, -1},  // NW
        {-1,  1},  // SE
        {-1, -1}   // SW
    };

    int rank = sq / 8;
    int file = sq % 8;

    for (auto& d : directions) {
        int r = rank + d[0];
        int f = file + d[1];

        while (r >= 0 && r < 8 && f >= 0 && f < 8) {
            int s = r * 8 + f;
            assert(s >= 0 && s < 64);
            attacks |= (1ULL << s);

            if (blockers & (1ULL << s))
                break;

            r += d[0];
            f += d[1];
        }
    }

    return attacks;
}


Bitboard Position::rookAttacks(int sq, Bitboard blockers) const {
    Bitboard attacks = 0ULL;

    const int directions[4][2] = {
        { 1,  0},  // north
        {-1,  0},  // south
        { 0,  1},  // east
        { 0, -1}   // west
    };

    int rank = sq / 8;
    int file = sq % 8;

    for (auto& d : directions) {
        int r = rank + d[0];
        int f = file + d[1];

        while (r >= 0 && r < 8 && f >= 0 && f < 8) {
            int s = r * 8 + f;
            assert(s >= 0 && s < 64);
            attacks |= (1ULL << s);

            if (blockers & (1ULL << s))
                break;

            r += d[0];
            f += d[1];
        }
    }

    return attacks;
}


// --------------------- Knight / King Attacks ---------------------
Bitboard Position::knightAttacksFor(int sq) const {
    Bitboard attacks = 0ULL;

    const int moves[8] = {17, 15, 10, 6, -17, -15, -10, -6};
    int rank = sq / 8, file = sq % 8;

    for(auto m : moves) {
        int newSq = sq + m;
        int newRank = newSq / 8, newFile = newSq % 8;
        if(newSq >=0 && newSq<64 && std::abs(newRank-rank)<=2 && std::abs(newFile-file)<=2) {
            assert( newSq >= 0 &&  newSq < 64);
            attacks |= (1ULL << newSq);
        }
    }
    return attacks;
}

Bitboard Position::kingAttacksFor(int sq) const {
    Bitboard attacks = 0ULL;
    int rank = sq/8, file = sq%8;
    for(int dr=-1;dr<=1;dr++)
        for(int df=-1;df<=1;df++) {
            if(dr==0 && df==0) continue;
            int nr = rank+dr, nf=file+df;
            if(nr>=0 && nr<8 && nf>=0 && nf<8)
                //assert( (nr*8+nf) >= 0 &&  (nr*8+nf) < 64);
                attacks |= (1ULL << (nr*8+nf));
        }
    return attacks;
}

// --------------------- Square Attacks ---------------------
bool Position::isSquareAttacked(int sq, Color byColor) const {
    Bitboard attackers;

    // Pawns
    attackers = piecesOf(byColor, PAWN);
    Bitboard pawnAttacksBB = (byColor == WHITE) ?
                                 ((attackers << 7) & ~0x8080808080808080ULL) |
                                     ((attackers << 9) & ~0x0101010101010101ULL) :
                                 ((attackers >> 9) & ~0x8080808080808080ULL) |
                                     ((attackers >> 7) & ~0x0101010101010101ULL);
    if(pawnAttacksBB & (1ULL << sq)) return true;

    // Knights
    attackers = piecesOf(byColor, KNIGHT);
    while(attackers) {
        int ksq = popLSB(attackers);
        if(knightAttacksFor(ksq) & (1ULL << sq)) return true;
    }

    // King
    attackers = piecesOf(byColor, KING);
    while(attackers) {
        int ksq = popLSB(attackers);
        if(kingAttacksFor(ksq) & (1ULL << sq)) return true;
    }

    Bitboard allBlockers = all;

    // Bishops & Queens
    attackers = piecesOf(byColor, BISHOP) | piecesOf(byColor, QUEEN);
    while(attackers) {
        int bqSq = popLSB(attackers);
        if(bishopAttacks(bqSq, allBlockers) & (1ULL << sq)) return true;
    }

    // Rooks & Queens
    attackers = piecesOf(byColor, ROOK) | piecesOf(byColor, QUEEN);
    while(attackers) {
        int rqSq = popLSB(attackers);
        if(rookAttacks(rqSq, allBlockers) & (1ULL << sq)) return true;
    }

    return false;
}

// --------------------- Check ---------------------
bool Position::inCheck(Color color) const {
    Bitboard kingBB = piecesOf(color, KING);
    if(!kingBB) return false;
    int kingSq = popLSB(kingBB);
    return isSquareAttacked(kingSq, opposite(color));
}

// --------------------- Helpers ---------------------
Piece Position::getPieceAt(int square) const {
    for(int c=0;c<2;++c) {
        for(int p=0;p<PIECE_NB;++p) {
            assert(square >= 0 && square < 64);
            if(pieces[c][p] & (1ULL << square)) {
                return {static_cast<PieceType>(p), (c == 1 ? WHITE: BLACK)};
            }
        }
    }
    return {PIECE_NB, Color(-1)};
}

void Position::removePieceAt(Color c, uint8_t square) {
    assert(square >= 0 && square < 64);
    Bitboard mask = 1ULL << square;
    for(int p=0;p<PIECE_NB;++p) {
        if(pieces[c][p] & mask) {
            pieces[c][p] &= ~mask;

            return;
        }
    }
}

bool Position::canCastleKingSide(Color us) const
{
    Color them = opposite(us);

    if (us == WHITE) {
        if (!castleWK) return false;

        // Squares between king and rook must be empty
        if (all & ((1ULL << 5) | (1ULL << 6))) return false;

        // King cannot be in check or pass through attacked squares
        if (isSquareAttacked(4, them)) return false; // e1
        if (isSquareAttacked(5, them)) return false; // f1
        if (isSquareAttacked(6, them)) return false; // g1

        return true;
    } else {
        if (!castleBK) return false;

        if (all & ((1ULL << 61) | (1ULL << 62))) return false;

        if (isSquareAttacked(60, them)) return false; // e8
        if (isSquareAttacked(61, them)) return false; // f8
        if (isSquareAttacked(62, them)) return false; // g8

        return true;
    }
}

bool Position::canCastleQueenSide(Color us) const
{
    Color them = opposite(us);

    if (us == WHITE) {
        if (!castleWQ) return false;

        if (all & ((1ULL << 1) | (1ULL << 2) | (1ULL << 3))) return false;

        if (isSquareAttacked(4, them)) return false; // e1
        if (isSquareAttacked(3, them)) return false; // d1
        if (isSquareAttacked(2, them)) return false; // c1

        return true;
    } else {
        if (!castleBQ) return false;

        if (all & ((1ULL << 57) | (1ULL << 58) | (1ULL << 59))) return false;

        if (isSquareAttacked(60, them)) return false; // e8
        if (isSquareAttacked(59, them)) return false; // d8
        if (isSquareAttacked(58, them)) return false; // c8

        return true;
    }
}

void Position::movePiece(Color c, PieceType p, int from, int to)
{

    Bitboard fromMask = 1ULL << from;
    Bitboard toMask   = 1ULL << to;

    // Capture if needed
    if (piecesOf(opposite(c)) & toMask) {
        removePieceAt(opposite(c), to);
    }

    // Remove piece from source square
    pieces[c][p] &= ~fromMask;

    // Place piece on destination
    pieces[c][p] |= toMask;

    // ---- Castling rights updates ----
    if (p == KING) {
        if (c == WHITE) {
            castleWK = false;
            castleWQ = false;
        } else {
            castleBK = false;
            castleBQ = false;
        }
    }

    if (p == ROOK) {
        if (c == WHITE) {
            if (from == 0) castleWQ = false;
            if (from == 7) castleWK = false;
        } else {
            if (from == 56) castleBQ = false;
            if (from == 63) castleBK = false;
        }
    }
}


void Position::makeCastle(Color us, bool kingSide)
{
    if (us == WHITE) {
        if (kingSide) {
            movePiece(us, KING, 4, 6);
            movePiece(us, ROOK, 7, 5);
        } else {
            movePiece(us, KING, 4, 2);
            movePiece(us, ROOK, 0, 3);
        }
        castleWK = castleWQ = false;
    } else {
        if (kingSide) {
            movePiece(us, KING, 60, 62);
            movePiece(us, ROOK, 63, 61);
        } else {
            movePiece(us, KING, 60, 58);
            movePiece(us, ROOK, 56, 59);
        }
        castleBK = castleBQ = false;
    }
}
void Position::makeMove(const Move& m, Undo& u)
{
    Color us   = sideToMove;
    Color them = opposite(us);

    // ---- Save undo ----
    u.enPassantSquare = enPassantSquare;
    u.castleWK = castleWK;
    u.castleWQ = castleWQ;
    u.castleBK = castleBK;
    u.castleBQ = castleBQ;
    u.movedPiece = getPieceAt(m.from).type;
    u.capturedPiece   = getPieceAt(m.to).type;
    u.promotionPiece = m.promotion;

    // ---- Reset EP ----
    enPassantSquare = -1;

    // ---- Castling ----
    if (m.isCastling()) {
        makeCastle(us, m.isKingSideCastle());
        sideToMove = them;
        updateOccupancy();
        return;
    }

    // ---- Captures (normal + promotion) ----
    if (m.captured != PIECE_NB && !m.isEnPassant()) {
        removePieceAt(them, m.to);
    }

    // ---- En passant ----
    if (m.isEnPassant()) {
        int capSq = (us == WHITE) ? m.to - 8 : m.to + 8;
        removePieceAt(them, capSq);
    }

    // ---- Pawn moves ----
    if (u.movedPiece == PAWN) {

        // Double push
        if (abs(m.to - m.from) == 16)
            enPassantSquare = (m.from + m.to) / 2;

        // Promotion
        if (m.isPromotion()) {
            removePieceAt(us, m.from);
            assert(m.to >= 0 && m.to < 64);
            pieces[us][m.promotion] |= (1ULL << m.to);
        }
        else {
            movePiece(us, PAWN, m.from, m.to);
        }
    }
    else {
        // ---- Normal piece ----
        movePiece(us, u.movedPiece, m.from, m.to);
    }
    updateOccupancy();
    sideToMove = them;
}



void Position::undoMove(const Move& m, const Undo& u)
{
    Color them = sideToMove;
    Color us   = opposite(them);

    sideToMove = us;

    // ---- Castling ----
    if (m.isCastling()) {
        if (us == WHITE) {
            if (m.isKingSideCastle()) {
                movePiece(us, KING, 6, 4);
                movePiece(us, ROOK, 5, 7);
            } else {
                movePiece(us, KING, 2, 4);
                movePiece(us, ROOK, 3, 0);
            }
        } else {
            if (m.isKingSideCastle()) {
                movePiece(us, KING, 62, 60);
                movePiece(us, ROOK, 61, 63);
            } else {
                movePiece(us, KING, 58, 60);
                movePiece(us, ROOK, 59, 56);
            }
        }

        // Restore state
        enPassantSquare = u.enPassantSquare;
        castleWK = u.castleWK;
        castleWQ = u.castleWQ;
        castleBK = u.castleBK;
        castleBQ = u.castleBQ;

        updateOccupancy();
        return;
    }

    // ---- Undo promotion ----
    if (m.isPromotion()) {
        removePieceAt(us, m.to);                 // remove promoted piece
        assert(m.from >= 0 && m.from < 64);
        pieces[us][PAWN] |= (1ULL << m.from);    // restore pawn
    }
    else {
        movePiece(us, u.movedPiece, m.to, m.from);
    }

    // ---- Restore captured piece ----
    if (u.capturedPiece != PIECE_NB) {
        if (m.isEnPassant()) {
            int capSq = (us == WHITE) ? m.to - 8 : m.to + 8;
            assert(capSq >= 0 && capSq < 64);
            pieces[them][PAWN] |= (1ULL << capSq);
        }
        else {
            assert(u.capturedPiece >= 0 && u.capturedPiece < PIECE_NB);
            pieces[them][u.capturedPiece] |= (1ULL << m.to);
        }
    }

    // Restore state
    enPassantSquare = u.enPassantSquare;
    castleWK = u.castleWK;
    castleWQ = u.castleWQ;
    castleBK = u.castleBK;
    castleBQ = u.castleBQ;


    updateOccupancy();
}

