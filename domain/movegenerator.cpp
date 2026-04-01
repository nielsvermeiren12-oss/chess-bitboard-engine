#include "movegenerator.h"
#include "domain/attack_tables.h"
#include "domain/bitboard.h"
#include "domain/undo.h"
#include <qdebug.h>



MoveGenerator::MoveGenerator() {
    initAttackTables();
}

void MoveGenerator::generateKnightMoves(Position &position, std::vector<Move> &moves)
{
    const Color us   = position.sideToMove;
    const Color them = opposite(us);
    Bitboard knights = position.piecesOf(us, KNIGHT);
    const Bitboard own = position.piecesOf(us);
    const Bitboard enemy = position.piecesOf(them);

    while(knights) {
        uint8_t square = popLSB(knights);
        Bitboard quiet = knightAttacks[square] & ~enemy & ~own;
        Bitboard captures = knightAttacks[square] & enemy & ~own;

        while(quiet) {
            uint8_t to = popLSB(quiet);
            moves.push_back({square, to, MoveFlag::NONE, KNIGHT});
        }

        while(captures) {
            uint8_t to = popLSB(captures);
            moves.push_back({square, to, MoveFlag::CAPTURE, KNIGHT});
        }
    }
}


void MoveGenerator::generatePawnMoves(Position &position, std::vector<Move> &moves)
{
    const Color us   = position.sideToMove;
    const Color them = opposite(us);

    const Bitboard pawns = position.piecesOf(us, PAWN);
    const Bitboard enemy = position.piecesOf(them);
    const Bitboard empty = ~position.all;

    // Directional constants
    const int pushDir   = (us == WHITE) ? 8 : -8;
    const int doubleDir = (us == WHITE) ? 16 : -16;

    const Bitboard startRank = (us == WHITE)
                                   ? 0x000000000000FF00ULL   // rank 2
                                   : 0x00FF000000000000ULL;  // rank 7

    const int promotionRankMin = (us == WHITE) ? 56 : 0;
    const int promotionRankMax = (us == WHITE) ? 63 : 7;

    // ---- Bulk push bitboards ----
    Bitboard singlePushes = (us == WHITE)
                                ? (pawns << 8) & empty
                                : (pawns >> 8) & empty;

    Bitboard pawnsOnStart = pawns & startRank;
    Bitboard oneStepFromStart = (us == WHITE)
                                    ? (pawnsOnStart << 8) & empty
                                    : (pawnsOnStart >> 8) & empty;

    Bitboard doublePushes = (us == WHITE)
                                ? (oneStepFromStart << 8) & empty
                                : (oneStepFromStart >> 8) & empty;

    auto isPromotionSquare = [&](int sq) {
        return sq >= promotionRankMin && sq <= promotionRankMax;
    };

    auto addPromotions = [&](uint8_t from, uint8_t to, bool capture) {
        MoveFlag flags = capture ? MoveFlag::CAPTURE | MoveFlag::PROMOTION : MoveFlag::PROMOTION;
        moves.push_back({from, to, flags, PAWN, QUEEN});
        moves.push_back({from, to, flags, PAWN, ROOK});
        moves.push_back({from, to, flags, PAWN, BISHOP});
        moves.push_back({from, to, flags, PAWN, KNIGHT});
    };

    Bitboard pawnsCopy = pawns;

    while (pawnsCopy) {
        uint8_t from = popLSB(pawnsCopy);

        // ---- Captures ----
        Bitboard captures = pawnAttacks[us][from] & enemy;
        while (captures) {
            uint8_t to = popLSB(captures);
            if (isPromotionSquare(to))
                addPromotions(from, to, true);
            else
                moves.push_back({from, to, MoveFlag::CAPTURE, PAWN});
        }

        // ---- Single push ----
        int to = from + pushDir;
        if (to >= 0 && to < 64 && (singlePushes & (1ULL << to))) {
            if (isPromotionSquare(to))
                addPromotions(from, to, false);
            else
                moves.push_back({from, (uint8_t)to, MoveFlag::NONE, PAWN});
        }

        // ---- Double push ----
        int to2 = from + doubleDir;
        if (to2 >= 0 && to2 < 64 && (doublePushes & (1ULL << to2))) {
            moves.push_back({from, (uint8_t)to2, MoveFlag::DOUBLE_PUSH, PAWN});
        }

        // ---- En passant ----
        if (position.enPassantSquare != -1) {
            Bitboard epBB = 1ULL << position.enPassantSquare;
            if (pawnAttacks[us][from] & epBB) {
                moves.push_back({from, static_cast<uint8_t>(position.enPassantSquare), MoveFlag::EN_PASSANT, PAWN});
            }
        }
    }
}

void MoveGenerator::generateKingMoves(Position &position, std::vector<Move> &moves)
{
    const Color us = position.sideToMove;
    const Color them = opposite(us);

    Bitboard kingBB = position.piecesOf(us, KING);
    uint8_t from = popLSB(kingBB);

    const Bitboard own   = position.piecesOf(us);
    const Bitboard enemy = position.piecesOf(them);

    Bitboard quiet    = kingAttacks[from] & ~own & ~enemy;
    Bitboard captures = kingAttacks[from] & enemy;

    // Quiet moves
    while (quiet) {
        uint8_t to = popLSB(quiet);
        Position p = position;

        Bitboard& k = p.pieces[us][KING];
        clearBit(k, from);
        setBit(k, to);

        if (!p.inCheck(us)) {
            moves.push_back({from, to, MoveFlag::NONE, KING});
        }
    }

    // Captures
    while (captures) {
        uint8_t to = popLSB(captures);
        Position p = position;

        p.removePieceAt(them, to);

        Bitboard& k = p.pieces[us][KING];
        clearBit(k, from);
        setBit(k, to);

        if (!p.inCheck(us)) {
            moves.push_back({from, to, MoveFlag::CAPTURE, KING});
        }
    }

    if (!position.inCheck(us)) {
        if (position.canCastleKingSide(us)) {
            moves.push_back({
                from,
                static_cast<uint8_t>(us == WHITE ? 6 : 62),
                MoveFlag::CASTLING,
                KING
            });
        }

        if (position.canCastleQueenSide(us)) {
            moves.push_back({
                from,
                static_cast<uint8_t>(us == WHITE ? 2 : 58),
                MoveFlag::CASTLING,
                KING
            });
        }
    }
}

void MoveGenerator::generateSlidingMoves(Position &position, std::vector<Move> &moves, PieceType piece, AttackFn attacksFn)
{
    Color us   = position.sideToMove;
    Color them = opposite(us);

    Bitboard sliders = position.piecesOf(us, piece);
    Bitboard own     = position.piecesOf(us);
    Bitboard enemy   = position.piecesOf(them);
    Bitboard blockers = own | enemy;

    while (sliders) {
        uint8_t from = popLSB(sliders);

        Bitboard attacks = attacksFn(from, blockers);

        Bitboard quiets   = attacks & ~blockers;
        Bitboard captures = attacks & enemy;

        while (quiets) {
            uint8_t to = popLSB(quiets);
            moves.push_back({from, to, MoveFlag::NONE, piece});
        }

        while (captures) {
            uint8_t to = popLSB(captures);
            moves.push_back({from, to, MoveFlag::CAPTURE, piece});
        }
    }
}

void MoveGenerator::generateLegalMoves(Position &position, std::vector<Move> &moves)
{
    std::vector<Move> pseudo;
    pseudo.reserve(256);

    generatePawnMoves(position, pseudo);
    generateKnightMoves(position, pseudo);
    generateKingMoves(position, pseudo);
    generateRookMoves(position, pseudo);
    generateBishopMoves(position, pseudo);
    generateQueenMoves(position, pseudo);

    Color us = position.sideToMove;

    for (const Move& m : pseudo)
    {
        Undo undo;
        position.makeMove(m, undo);

        bool illegal = position.inCheck(us);

        position.undoMove(m, undo);

        if (!illegal) {
            moves.push_back(m);
        }
    }
}


std::vector<Move> MoveGenerator::generatePseudoMovesFromSquare(const Position& pos, uint8_t from)
{
    std::vector<Move> moves;
    moves.reserve(256);
    Piece piece = pos.getPieceAt(from);
    if (static_cast<int>(piece.type) == -1) return moves;

    Color them = piece.colour == WHITE ? BLACK : WHITE;

    Bitboard occAll = pos.all;
    Bitboard occThem = pos.piecesOf(them);

    switch (piece.type) {

    // =========================
    // PAWN
    // =========================
    case PAWN: {
        int dir = (piece.colour == WHITE) ? 8 : -8;
        int startRank = (piece.colour == WHITE) ? 1 : 6;
        int promoteRank = (piece.colour == WHITE) ? 6 : 1;

        uint8_t one = from + dir;
        if (!getBit(occAll, one)) {

            if (rankOf(from) == promoteRank) {
                for (PieceType promo : {QUEEN, ROOK, BISHOP, KNIGHT}) {
                    moves.push_back({from, one, MoveFlag::PROMOTION, PAWN, PIECE_NB, promo});

                }
            } else {

                moves.push_back({
                    (uint8_t)from,
                    (uint8_t)one,
                    MoveFlag::NONE,
                    PAWN
                });

                uint8_t two = from + 2 * dir;
                if (rankOf(from) == startRank && !getBit(occAll, two)) {
                    moves.push_back({from, two, MoveFlag::DOUBLE_PUSH, PAWN});
                }
            }
        }

        // Captures
        Bitboard caps = pawnAttacks[piece.colour][from] & occThem;

        while (caps) {


            uint8_t to = popLSB(caps);
            PieceType capturedPiece = getCapturedPiece(pos, to, them);
            if (rankOf(from) == promoteRank) {
                for (PieceType promo : {QUEEN, ROOK, BISHOP, KNIGHT}) {
                    moves.push_back({from, to, MoveFlag::PROMOTION, PAWN, capturedPiece, promo});
                }
            } else {
                moves.push_back({from, to, MoveFlag::CAPTURE, PAWN, capturedPiece});
            }
        }

        // En passant
        if (pos.enPassantSquare != -1) {
            Bitboard epMask = pawnAttacks[piece.colour][from] & (1ULL << pos.enPassantSquare);
            if (epMask) {
                moves.push_back({from, static_cast<uint8_t>(pos.enPassantSquare), MoveFlag::EN_PASSANT, PAWN});
            }
        }
        break;
    }

    // =========================
    // KNIGHT
    // =========================
    case KNIGHT: {
        Bitboard atk = knightAttacks[from];
        Bitboard quiet = atk & ~occAll;
        Bitboard caps = atk & occThem;

        while (quiet) {
            moves.push_back({from, static_cast<uint8_t>(popLSB(quiet)), MoveFlag::NONE, KNIGHT});
        }
        while (caps) {
            uint8_t to = popLSB(caps);
            PieceType capturedPiece = getCapturedPiece(pos, to, them);
            moves.push_back({from, to, MoveFlag::CAPTURE, KNIGHT, capturedPiece});
        }
        break;
    }

    // =========================
    // BISHOP
    // =========================
    case BISHOP: {
        Bitboard atk = bishopAttacks(from, occAll);
        Bitboard quiet = atk & ~occAll;
        Bitboard caps = atk & occThem;

        while (quiet) moves.push_back({from, static_cast<uint8_t>(popLSB(quiet)), MoveFlag::NONE, BISHOP});
        while (caps) {
            uint8_t to = popLSB(caps);
            PieceType capturedPiece = getCapturedPiece(pos, to, them);
            moves.push_back({from, to, MoveFlag::CAPTURE, BISHOP, capturedPiece});
        }
        break;
    }

    // =========================
    // ROOK
    // =========================
    case ROOK: {
        Bitboard atk = rookAttacks(from, occAll);
        Bitboard quiet = atk & ~occAll;
        Bitboard caps = atk & occThem;

        while (quiet) moves.push_back({from, static_cast<uint8_t>(popLSB(quiet)), MoveFlag::NONE, ROOK});
        while (caps) {
            uint8_t to = popLSB(caps);
            PieceType capturedPiece = getCapturedPiece(pos, to, them);
            moves.push_back({from, to, MoveFlag::CAPTURE, ROOK, capturedPiece});
        }
        break;
    }

    // =========================
    // QUEEN
    // =========================
    case QUEEN: {
        Bitboard atk = bishopAttacks(from, occAll) | rookAttacks(from, occAll);
        Bitboard quiet = atk & ~occAll;
        Bitboard caps = atk & occThem;

        while (quiet) moves.push_back({from, static_cast<uint8_t>(popLSB(quiet)), MoveFlag::NONE, QUEEN});
        while (caps) {
            uint8_t to = popLSB(caps);
            PieceType capturedPiece = getCapturedPiece(pos, to, them);
            moves.push_back({from, to, MoveFlag::CAPTURE, QUEEN, capturedPiece});
        }
        break;
    }

    // =========================
    // KING
    // =========================
    case KING: {
        Bitboard atk = kingAttacks[from];
        Bitboard quiet = atk & ~occAll;
        Bitboard caps = atk & occThem;

        while (quiet) moves.push_back({from, static_cast<uint8_t>(popLSB(quiet)), MoveFlag::NONE, KING});
        while (caps) {
            uint8_t to = popLSB(caps);
            PieceType capturedPiece = getCapturedPiece(pos, to, them);
            moves.push_back({from, to, MoveFlag::CAPTURE, KING, capturedPiece});
        }

        // Castling (pseudo)
        if (piece.colour == WHITE) {
            // White king side
            if (pos.castleWK) {
                bool squaresBetweenNotAttacked = !pos.isSquareAttacked(5, them) && !pos.isSquareAttacked(6, them);
                if (!getBit(occAll, 5) && !getBit(occAll, 6) &&  squaresBetweenNotAttacked) { // F1, G1
                    moves.push_back({4, 6, MoveFlag::CASTLING, KING}); // E1 -> G1
                }
            }

            // White queen side
            if (pos.castleWQ) {
                bool squaresBetweenNotAttacked = !pos.isSquareAttacked(1, them) && !pos.isSquareAttacked(2, them) && !pos.isSquareAttacked(3, them);
                if (!getBit(occAll, 3) && !getBit(occAll, 2) && !getBit(occAll, 1) && squaresBetweenNotAttacked) { // D1, C1, B1
                    moves.push_back({4, 2, MoveFlag::CASTLING, KING}); // E1 -> C1
                }
            }

        } else {

            // Black king side
            if (pos.castleBK) {
                bool squaresBetweenNotAttacked = !pos.isSquareAttacked(61, them) && !pos.isSquareAttacked(62, them);
                if (!getBit(occAll, 61) && !getBit(occAll, 62) && squaresBetweenNotAttacked) { // F8, G8
                    moves.push_back({60, 62, MoveFlag::CASTLING, KING}); // E8 -> G8
                }
            }

            // Black queen side
            if (pos.castleBQ) {
                bool squaresBetweenNotAttacked = !pos.isSquareAttacked(57, them) && !pos.isSquareAttacked(58, them) && !pos.isSquareAttacked(59, them);
                if (!getBit(occAll, 59) && !getBit(occAll, 58) && !getBit(occAll, 57) && squaresBetweenNotAttacked) { // D8, C8, B8
                    moves.push_back({60, 58, MoveFlag::CASTLING, KING}); // E8 -> C8
                }
            }
        }
        break;
    }
    case PIECE_NB:
        break;
    }
    return moves;
}

std::vector<Move> MoveGenerator::generateLegalMovesFromSquare(Position &pos, int fromSquare)
{
    std::vector<Move> legalMoves;
    legalMoves.reserve(256);
    Piece piece = pos.getPieceAt(fromSquare);
    if (piece.type == PIECE_NB) {
        return legalMoves;
    }

    Color us = pos.sideToMove;

    //Generate pseudo-legal moves
    std::vector<Move> pseudoMoves = generatePseudoMovesFromSquare(pos, fromSquare);

    //Filter by king safety
    for (const Move& m : pseudoMoves) {
        Undo undo;
        pos.makeMove(m, undo);
        bool inCheck = pos.inCheck(us);

        pos.undoMove(m, undo);

        if (!inCheck) {
            legalMoves.push_back(m);
        }

    }

    return legalMoves;
}
