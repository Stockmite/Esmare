#include <iostream>
#include <vector>
#include <cmath>

typedef int8_t Square;
typedef int64_t BitBoard;

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

    BitBoard GetRookMask(Piece ThePiece) {
        int PieceFile = ThePiece.PieceSquare % 8;
        int PieceLine = (ThePiece.PieceSquare - PieceFile) / 8;

        //The mask is initialized to the horizontal squares the rook can move to
        BitBoard Mask = (255 - (int)pow(2, 8 - PieceFile)) << (PieceLine * 8);
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

    void GetKnightMoves(Piece ThePiece, Position CurPosition) {
        Square OgSquare = ThePiece.PieceSquare;
        
        for (int a = -1; a<2; a=a+2) {
            for (int b = -1; b<2; b=b+2) {
                Square PotentialSquare1 = (Square)((16 * a) + b + OgSquare);
                Square PotentialSquare2 = (Square)((16 * b) + a + OgSquare);

                if (SquareFuncs::DoesSquareExist(PotentialSquare1)) {
                    RegisterMove(ThePiece, PotentialSquare1, ThePiece.PossibleMoves);
                }
                if (SquareFuncs::DoesSquareExist(PotentialSquare2)) {
                    RegisterMove(ThePiece, PotentialSquare2, ThePiece.PossibleMoves);
                }
            }
        }
    }

    void GetBishopMoves(Piece ThePiece, Position CurPosition) {
        
        

    }
}