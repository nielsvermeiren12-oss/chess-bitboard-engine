#ifndef ATTACK_TABLES_H
#define ATTACK_TABLES_H

#include "domain/types.h"
#pragma once
#include <cstdint>


extern Bitboard pawnAttacks[2][64];
extern Bitboard knightAttacks[64];
extern Bitboard kingAttacks[64];

inline constexpr int rookDirs[4]   = { 8, -8, 1, -1 };
inline constexpr int bishopDirs[4] = { 9, 7, -7, -9 };
inline constexpr uint64_t H_FILE = 0x8080808080808080ULL;
inline constexpr uint64_t A_FILE = 0x0101010101010101ULL;

inline bool onBoard(int sq) {
    return sq >= 0 && sq < 64;
}

inline int file(int sq) {
    return sq & 7;
}

inline int rankOf(int sq) {
    return sq >> 3;
}

inline int fileOf(int sq) {
    return sq & 7;
}

inline bool sameRank(int a, int b) {
    return rankOf(a) == rankOf(b);
}

inline bool sameFile(int a, int b) {
    return fileOf(a) == fileOf(b);
}

static void initPawnAttacks();
static void initKnightAttacks();
static void initKingAttacks();
void initAttackTables();

Bitboard slidingAttacks(int square, const int* directions, int dirCount, Bitboard blockers);
Bitboard rookAttacks(int square, Bitboard blockers);
Bitboard bishopAttacks(int square, Bitboard blockers);
Bitboard queenAttacks(int square, Bitboard blockers);


#endif // ATTACK_TABLES_H
