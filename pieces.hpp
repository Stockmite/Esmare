#include <iostream>
#include <vector>
#include <list>

typedef unsigned __int8 Square;
typedef unsigned __int64 BitBoard;

enum PieceTypes {
    KNIGHT,
    PAWN,
    KING,
    QUEEN,
    BISHOP,
    ROOK
};
typedef enum PieceTypes PieceType;
class Move {
    private:
        Square OgSquare;
        Square NewSquare;
        PieceType MovedPiece;
};
class Piece {
    public:
        Square PieceSquare;
        bool Has_Moved;

        BitBoard Attacks;
        std::list<Move> MoveList;
        
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