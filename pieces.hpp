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

        

    private:
        char type;
};

class Side {
    public:
        std::vector<Piece> PieceList;
        BitBoard PieceLocations;
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

}
namespace LegalMoves
{
    void GetKnightMoves(Piece ThePiece) {

    }
}