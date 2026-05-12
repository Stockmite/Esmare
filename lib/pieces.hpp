#include <iostream>
#include <vector>
#include <cmath>

#include "MiscellaneousData\MaskOmmiter.hpp"

typedef short signed int Square;
typedef long long BitBoard;

enum PieceTypes {
    KNIGHT,
    PAWN,
    KING,
    QUEEN,
    BISHOP,
    ROOK,
    null
};
typedef enum PieceTypes PieceType;
class Move {
    public:
        Square OgSquare;
        Square NewSquare;
        PieceType MovedPiece;
        
        Move(Square os, Square ns, PieceType mp) {
            OgSquare = os; NewSquare = ns; MovedPiece = mp;
        }
};
typedef std::vector<Move> MoveList;
class Piece {
    public:
        Square PieceSquare;
        const PieceType Type;
        bool Has_Moved;

        BitBoard Attacks;
        MoveList* PossibleMoves;

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
    bool DoesSquareExist(Square TheSquare) {return 1 <= TheSquare <= 64;}

    BitBoard GetBBSpot(Square TheSquare) {return (1 << TheSquare);}

    BitBoard GetLimitingBB(Position CurPosition, Piece ThePiece) {
        Square PieceSquare = ThePiece.PieceSquare;

        Side OpposingSide = (CurPosition.White.PieceLocations & GetBBSpot(PieceSquare))
         ? CurPosition.White : CurPosition.Black;
        //Note to self: if the piece locations are imprecise this function gets screwed up
        
        return OpposingSide.PieceLocations & ThePiece.ValidSquares & OpposingSide.GlobalValidSquares;
    }
}

namespace BBFuncs{

    BitBoard GetAllFileSquares(short unsigned int file) {

        BitBoard FileSquares = SquareFuncs::GetBBSpot(file);

        for (int line = 0; line < 7; line++) {
            BitBoard LineSquare = FileSquares << 8;
            FileSquares |= LineSquare;
        }

        return FileSquares;

    }

    BitBoard GetRookMask(Square OgSquare) {
        int PieceFile = OgSquare % 8;
        int PieceLine = (OgSquare - PieceFile) / 8;

        //The mask is initialized to the horizontal squares the rook can move to
        BitBoard Mask = 255 << PieceLine * 8;
        Mask |= GetAllFileSquares(PieceFile);

        return Mask ^ SquareFuncs::GetBBSpot(OgSquare);
    }

    BitBoard GetKnightMask(Square OgSquare) { 
        int PieceFile = OgSquare % 8;
        int PieceLine = (OgSquare - PieceFile) / 8;  

        BitBoard Mask = 0x50880110A;

        short int dif = (OgSquare - 19);
        //The mask is initially situated at c3, then shifted accordingly

        Mask = (dif >= 0) ? Mask << OgSquare - 1 : Mask >> OgSquare - 1;

        return Mask ^ HorizontalOmmiter[PieceFile] ^ VerticalOmmiter[PieceLine];
    }

    BitBoard GetKingMask(Square OgSquare) {
        int PieceFile = OgSquare % 8;
        int PieceLine = (OgSquare - PieceFile) / 8;

        BitBoard Mask = 0x70507;

        short int dif = (OgSquare - 10);
        Mask = (dif >= 0) ? Mask << OgSquare - 1 : Mask >> OgSquare - 1;

        return Mask ^ HorizontalOmmiter[PieceFile] ^ VerticalOmmiter[PieceLine];
    }

    BitBoard GetBishopMask(Square OgSquare) {
        int PieceFile = OgSquare % 8;

        BitBoard Diagonal1, Diagonal2 = SquareFuncs::GetBBSpot(OgSquare);

        BitBoard BufVal = Diagonal1;

        for (int ind = 0; ind < (8 - PieceFile); ind++) {
            Diagonal1 |= (BufVal) << ((9 * ind) + (ind * (ind + 1) / 2));
        }

        for (int ind = 0; ind < PieceFile; ind++) {
            Diagonal1 |= (BufVal) >> ((9 * ind) + (ind * (ind + 1) / 2));
        }

        for (int ind = 0; ind < (8 - PieceFile); ind++) {
            Diagonal2 |= (BufVal) << ((7 * ind) + (ind * (ind + 1) / 2));
        }

        for (int ind = 0; ind < PieceFile; ind++) {
            Diagonal2 |= (BufVal) >> ((7 * ind) + (ind * (ind + 1) / 2));
        }

        return (Diagonal2 | Diagonal1) ^ BufVal;

    }

    BitBoard GetQueenMask(Square OgSquare) {
        return GetBishopMask(OgSquare) | GetRookMask(OgSquare);
    }

}
namespace LegalMoves
{
    void AssignMoveList(MoveList* ptr, MoveList* NewMoves) {
        free(ptr); ptr = NewMoves;
    }

    void RegisterMove(Piece ThePiece, Square TargetSquare, MoveList* PossibleMoves) {

        Move NewMove(ThePiece.PieceSquare, TargetSquare, ThePiece.Type);
        PossibleMoves->push_back(NewMove);

    }

}