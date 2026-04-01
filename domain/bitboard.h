#ifndef BITBOARD_H
#define BITBOARD_H

#include <QDebug>
#include <cstdint>

// Set bit at square
inline void setBit(uint64_t &bb, int sq) {
    bb |= (1ULL << sq);
}

// Clear bit at square
inline void clearBit(uint64_t &bb, int sq) {
    bb &= ~(1ULL << sq);
}

// Test bit at square
inline bool getBit(uint64_t bb, int square) {
    if (square < 0 || square >= 64) return false;
    return (bb >> square) & 1ULL;
}

// Pop least significant bit (for iterating pieces)
inline int popLSB(uint64_t &bb) {
    if(bb == 0) return -1;

    int sq = 0;
    uint64_t b = bb;
    while ((b & 1ULL) == 0) { // check LSB
        b >>= 1;
        ++sq;
    }

    bb &= bb - 1; // clear LSB
    return sq;
}


// Count bits
inline int countBits(uint64_t bb) {
    //return __builtin_popcountll(bb);
    return 0;
}

// Print bitboard for debugging purposes
inline void printBitboard(uint64_t bb) {
    QString bitboard;
    qDebug() << "";
    for (int rank = 7; rank >= 0; --rank) {
        bitboard = "";
        // 8th rank to 1st rank
        for (int file = 0; file < 8; ++file) {    // a to h
            int sq = rank * 8 + file;
            if ((bb >> sq) & 1ULL)
                bitboard += "1 ";
            else
                bitboard += ". ";
        }
        bitboard += "  " ;
        bitboard += std::to_string(rank + 1);
        qDebug() << bitboard;
    }

    qDebug() << "  a b c d e f g h";
    qDebug() << "";
}


#endif // BITBOARD_H
