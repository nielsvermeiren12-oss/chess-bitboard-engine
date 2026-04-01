#include "attack_tables.h"
#include "domain/types.h"
#include <cmath>

Bitboard pawnAttacks[2][64];
Bitboard knightAttacks[64];
Bitboard kingAttacks[64];

void initAttackTables() {
    initKnightAttacks();
    initPawnAttacks();
    initKingAttacks();
}

static void initKnightAttacks() {
    for (int sq = 0; sq < 64; sq++) {
        Bitboard attacks = 0;
        int r = sq / 8;
        int f = sq % 8;

        auto add = [&](int dr, int df) {
            int rr = r + dr;
            int ff = f + df;
            if (rr >= 0 && rr < 8 && ff >= 0 && ff < 8)
                attacks |= 1ULL << (rr * 8 + ff);
        };

        add( 2,  1); add( 2, -1);
        add(-2,  1); add(-2, -1);
        add( 1,  2); add( 1, -2);
        add(-1,  2); add(-1, -2);

        knightAttacks[sq] = attacks;
    }
}

static void initPawnAttacks() {



    for (int sq = 0; sq < 64; sq++) {
        int square = (7 - std::floor(sq / 8)) * 8 + (sq % 8);
        uint64_t bb = 1ULL << square;

        // White pawns attack up-left and up-right
        uint64_t white = 0;
        if ((bb << 7) & ~H_FILE) white |= (bb << 7); // not from h-file
        if ((bb << 9) & ~A_FILE) white |= (bb << 9); // not from a-file

        // Black pawns attack down-left and down-right
        uint64_t black = 0;
        if ((bb >> 7) & ~A_FILE) black |= (bb >> 7); // not from a-file
        if ((bb >> 9) & ~H_FILE) black |= (bb >> 9); // not from h-file

        pawnAttacks[WHITE][square] = white;
        pawnAttacks[BLACK][square] = black;
    }
}

static void initKingAttacks() {
    for (int sq = 0; sq < 64; sq++) {
        Bitboard attacks = 0;
        int r = sq / 8;
        int f = sq % 8;

        auto add = [&](int dr, int df) {
            int rr = r + dr;
            int ff = f + df;
            if (rr >= 0 && rr < 8 && ff >= 0 && ff < 8)
                attacks |= 1ULL << (rr * 8 + ff);
        };

        add(-1, -1); add(-1, 0); add(-1, 1);
        add( 0, -1);             add( 0, 1);
        add( 1, -1); add( 1, 0); add( 1, 1);

        kingAttacks[sq] = attacks;
    }
}

Bitboard slidingAttacks(int square, const int *directions, int dirCount, Bitboard blockers)
{
    Bitboard attacks = 0;

    for (int i = 0; i < dirCount; i++) {
        int dir = directions[i];
        int s = square;

        while (true) {
            int next = s + dir;

            // File wrap prevention
            if (!onBoard(next)) break;

            int f0 = file(s);
            int f1 = file(next);
            if (std::abs(f1 - f0) > 1)
                break;

            attacks |= (1ULL << next);

            if (blockers & (1ULL << next))
                break;

            s = next;
        }
    }

    return attacks;
}

Bitboard rookAttacks(int sq, Bitboard blockers) {
    return slidingAttacks(sq, rookDirs, 4, blockers);
}

Bitboard bishopAttacks(int sq, Bitboard blockers) {
    return slidingAttacks(sq, bishopDirs, 4, blockers);
}

Bitboard queenAttacks(int sq, Bitboard blockers) {
    return rookAttacks(sq, blockers) | bishopAttacks(sq, blockers);
}

