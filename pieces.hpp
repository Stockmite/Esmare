#include <iostream>
#include <vector>

typedef uint8_t Square;
typedef uint64_t BitBoard;

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
    private:
        Square OgSquare;
        Square NewSquare;
        PieceType MovedPiece;
};
typedef std::vector<Move> MoveList;
class Piece {
    public:
        Square PieceSquare;
        bool Has_Moved;

        BitBoard Attacks;
        MoveList* PossibleMoves;

        BitBoard ValidSquares;
        //For absolute pins and (presumably) other stuff

    private:
        char type;
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

        BitBoard OpposingBB = (CurPosition.White.PieceLocations & GetBBSpot(PieceSquare))
         ? CurPosition.White.PieceLocations : CurPosition.Black.PieceLocations;
        //Note to self: if the piece locations are imprecise this function gets screwed up
        
        return OpposingBB & ThePiece.ValidSquares;
    }
}
namespace LegalMoves
{
    void AssignMoveList(MoveList* ptr, MoveList* NewMoves) {
        free(ptr); ptr = NewMoves;
    }

    void GetKnightMoves(Piece ThePiece, Position CurPosition) {
        
    }
}