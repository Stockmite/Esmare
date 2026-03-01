#include <iostream>
#include <vector>

using namespace std;

typedef unsigned short int Square;
typedef unsigned long long BitBoard;
class Piece {
    public:
        Square PieceSquare;
        bool Has_Moved;
    private:
        char type;
};

class Side {
    public:
        vector<Piece> PieceList;
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
    unsigned short int EnPassantSquares;
} Position;