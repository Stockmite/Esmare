#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <array>
#include <iterator>

#include "LookupData\MagicNumbers.hpp"
#include "LookupData\SlideAttackLookup.hpp"

#ifndef USEFUL_DEFINITIONS
    #include "UsefulDefs.hpp"
#endif

using Square = short unsigned int;
using BitBoard = unsigned long long int;
using color = bool;

enum PieceTypes {
    KNIGHT,
    PAWN,
    KING,
    QUEEN,
    BISHOP,
    ROOK,
    null
};

enum Dir {
    Nor = 8, Wes = 1,
    Sou = -8, Eas = -1,
    NorWes = 9, NorEas = 7,
    SouWes = -7, SouEas = -9
};

using PieceType = enum PieceTypes;
using Direction = enum Dir;
class Move {
    public:
        Square OgSquare;
        Square NewSquare;
        PieceType MovedPiece;
        
        Move(Square os, Square ns, PieceType mp) {
            OgSquare = os; NewSquare = ns; MovedPiece = mp;
        }
};
using MoveVector = std::vector<Move>;
using MaskVector = std::vector<BitBoard>;
using Lookup = std::array<MaskVector , 64>;
//TODO: Examine the possibility of later replacing BitBoard* with a vector
class Piece {
    public:
        Square PieceSquare;
        const PieceType Type;
        bool Has_Moved;

        BitBoard Attacks;
        MoveVector* PossibleMoves;

        BitBoard ValidSquares;
        //For absolute pins and (presumably) other stuff
};

class Side {
    public:
        std::vector<Piece> PieceList;
        BitBoard PieceLocations;
        BitBoard GlobalValidSquares;
};

typedef struct {
    BitBoard Pawns;
    BitBoard Knights;
    BitBoard Bishops;
    BitBoard Rooks;
    BitBoard Queens;
    BitBoard Kings;

    Side White;
    Side Black;
    unsigned char EnPassantSquares;
} Position;

namespace SquareFuncs{
    bool DoesSquareExist(Square TheSquare) {return 1 <= TheSquare && TheSquare <= 64;}

    BitBoard GetBBSpot(Square TheSquare) {

        BitBoard base = 1UL;
        return base << (TheSquare-1);
    }

    BitBoard GetLimitingBB(Position CurPosition, Piece ThePiece) {
        Square PieceSquare = ThePiece.PieceSquare;

        Side OpposingSide = (CurPosition.White.PieceLocations & GetBBSpot(PieceSquare))
         ? CurPosition.White : CurPosition.Black;
        //Note to self: if the piece locations are imprecise this function gets screwed up
        
        return OpposingSide.PieceLocations & ThePiece.ValidSquares & OpposingSide.GlobalValidSquares;
    }
}

namespace BitFuncs {

    BitBoard GetLSB(BitBoard Mask) {return Mask & -Mask;}

    BitBoard RemoveLSB(BitBoard Mask) {return Mask & (Mask - 1);}

    int getPopCount(BitBoard Mask){

        if (Mask == 0) {return 0;}
        else if (BitFuncs::RemoveLSB(Mask) == 0) {return 1;}

        int count = 0;
        while (Mask) {
            count++;
            Mask = BitFuncs::RemoveLSB(Mask);
        }
        return count;

    }

    BitBoard GetMSB(BitBoard Mask) {

        if (Mask == 0xFFFFFFFFFFFFFFFF) {return (1UL << 63);}

        BitBoard MSB = 0;

        for (int shift = 1; shift <= 32; shift <<= 1) {
            Mask |= (Mask >> shift);
        }

        Mask += 1;
        return Mask >> 1;

    }

}

namespace BBFuncs{

    BitBoard GetBlockerMask(int index, BitBoard AimMask) {

        BitBoard BlockerMask = 0;

        for (int shift = 0; AimMask ; shift++) {

            BitBoard LSB = BitFuncs::GetLSB(AimMask);

            bool InsertBit = (index >> shift) & 1;
            if (InsertBit) {BlockerMask |= LSB;}

            AimMask = BitFuncs::RemoveLSB(AimMask);
        }

        return BlockerMask;
    }

    BitBoard GetRookMask(Square OgSquare) {
        int PieceFile = ((OgSquare-1) % 8);
        int PieceLine = (OgSquare - (PieceFile+1)) / 8;

        return (RankLookup[PieceLine] | FileLookup[PieceFile]) ^ SquareFuncs::GetBBSpot(OgSquare);
    }

    BitBoard GetKnightMask(Square OgSquare) { 
        int PieceFile = OgSquare % 8;
        int PieceLine = (OgSquare - PieceFile) / 8;  

        BitBoard Mask = 0x50880110A;

        short int dif = (OgSquare - 19);
        //The mask is initially situated at c3, then shifted accordingly

        Mask = (dif >= 0) ? Mask << OgSquare - 1 : Mask >> OgSquare - 1;

        //return Mask ^ HorizontalOmmiter[PieceFile] ^ VerticalOmmiter[PieceLine];
    }

    BitBoard GetKingMask(Square OgSquare) {
        int PieceFile = OgSquare % 8;
        int PieceLine = (OgSquare - PieceFile) / 8;

        BitBoard Mask = 0x70507;

        short int dif = (OgSquare - 10);
        Mask = (dif >= 0) ? Mask << OgSquare - 1 : Mask >> OgSquare - 1;

        //return Mask ^ HorizontalOmmiter[PieceFile] ^ VerticalOmmiter[PieceLine];
    }

    BitBoard GetBishopMask(Square OgSquare) {

        return (DiagonalLookup[OgSquare-1] | AntiDLookup[OgSquare-1]);

    }

    BitBoard GetQueenMask(Square OgSquare) {
        return GetBishopMask(OgSquare) | GetRookMask(OgSquare);
    }

}

namespace SlidingPieceMoves {

    constexpr BitBoard SlideAttack(int increment, Square TheSquare, BitBoard BlockerMask) {

        BitBoard Aim = 0;

        auto file = [](Square s) {return ((s-1)%8)+1;};
        auto rank = [](Square s) {return (s - ((s-1)%8)-1)/8;};

        bool AttackBlocked = false;
        bool OutOfBounds = false;
        Square OgSquare = TheSquare;

        while (!AttackBlocked && !OutOfBounds) {

            Square NewS = TheSquare+increment;
            BitBoard Spot = SquareFuncs::GetBBSpot(TheSquare);

            if (SquareFuncs::DoesSquareExist(TheSquare)) {Aim |= Spot;}

            AttackBlocked = Spot & BlockerMask;

            short int file_distance = abs(file(NewS) - file(TheSquare));
            short int rank_distance = abs(rank(NewS) - rank(TheSquare));

            OutOfBounds = (file_distance > 2 || rank_distance > 2);
            TheSquare = NewS;

        }

        return Aim ^ SquareFuncs::GetBBSpot(OgSquare);
    }
    
    BitBoard RookMoves(Square s, BitBoard BlockerMask) {

        BitBoard RookMovesMask = 0;

        int up = 8; int toside = 1;
        RookMovesMask |= SlideAttack(up, s, BlockerMask);
        RookMovesMask |= SlideAttack(-up, s, BlockerMask);
        RookMovesMask |= SlideAttack(toside, s, BlockerMask);
        RookMovesMask |= SlideAttack(-toside, s, BlockerMask);

        return RookMovesMask;
    }

    BitBoard BishopMoves(Square s, BitBoard BlockerMask) {

        BitBoard BishopMovesMask = 0;

        //"D" stands for diagonal
        int MainD = 9; int AntiD = 7;
        BishopMovesMask |= SlideAttack(MainD, s, BlockerMask);
        BishopMovesMask |= SlideAttack(-MainD, s, BlockerMask);
        BishopMovesMask |= SlideAttack(AntiD, s, BlockerMask);
        BishopMovesMask |= SlideAttack(-AntiD, s, BlockerMask);

        return BishopMovesMask;
    }

    BitBoard QueenMoves(Square s, BitBoard BlockerMask) {
        return RookMoves(s, BlockerMask) | BishopMoves(s, BlockerMask);
    }

    BitBoard PawnMoves(Square s, BitBoard BlockerMask) {
        return BlockerMask;
        
    }

};
namespace LegalMoves
{
    void AssignMoveList(MoveVector* ptr, MoveVector* NewMoves) {
        free(ptr); ptr = NewMoves;
    }

    void RegisterMove(Piece ThePiece, Square TargetSquare, MoveVector* PossibleMoves) {

        Move NewMove(ThePiece.PieceSquare, TargetSquare, ThePiece.Type);
        PossibleMoves->push_back(NewMove);

    }

    //TODO: Implement bitboard techniques to deal with varying circumnstances and 
    //edge cases in the chessboard
    void GetMovesFromMask(Piece ThePiece, BitBoard Mask, MoveVector* MoveBuf){

        while (Mask) {

            Square MoveSquare = log2(Mask & -Mask); //LSB of mask

            RegisterMove(ThePiece, MoveSquare, MoveBuf);
            Mask &= Mask - 1;
        }

    }

    short int GetSquareMagicBBIndex(BitBoard SquareMask, BitBoard BlockerMask) {

        short int index = 0;
        short int shift = 0;

        while (SquareMask) {

            BitBoard bit = BitFuncs::GetLSB(SquareMask); //Extract LSB
            index |= ((bool)(bit & BlockerMask) >> shift);
            shift++;

            SquareMask = BitFuncs::RemoveLSB(SquareMask);
        }

        return index;

    }

    void CreateRookMaskLookup(Lookup& RookMovesArray) {

        for (Square CurSquare = 1; CurSquare < 65; CurSquare++) {

            BitBoard AimMask = BBFuncs::GetRookMask(CurSquare);

            int MaskCount = 1 << BitFuncs::getPopCount(AimMask);
            int ArrayIndex = CurSquare - 1;

            MaskVector& MasksOnSquare = RookMovesArray[ArrayIndex];

            for (int index = 0; index < MaskCount; index++) {

                BitBoard BlockerMask = BBFuncs::GetBlockerMask(index, AimMask);
                BitBoard RookMoves = SlidingPieceMoves::RookMoves(CurSquare, BlockerMask);

                MasksOnSquare.push_back(RookMoves);
            }

        }

    }

    void CreateBishopMaskLookup(Lookup& BishopMovesArray) {


        for (Square CurSquare = 1; CurSquare < 65; CurSquare++) {

            BitBoard AimMask = BBFuncs::GetBishopMask(CurSquare);

            int MaskCount = 1 << BitFuncs::getPopCount(AimMask);
            int ArrayIndex = CurSquare - 1;

            MaskVector& MasksOnSquare = BishopMovesArray[ArrayIndex];

            for (int index = 0; index < MaskCount; index++) {

                BitBoard BlockerMask = BBFuncs::GetBlockerMask(index, AimMask);
                BitBoard BishopMoves = SlidingPieceMoves::BishopMoves(CurSquare, BlockerMask);

                MasksOnSquare.push_back(BishopMoves);

            }

        }

    }

    void GetLegalMoves(color Side, Position CurPosition) {

        

    }

}