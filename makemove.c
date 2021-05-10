#include <stdio.h>
#include "defs.h"

#define HASH_PCE(pce,sq) (pos->posKey ^= (PieceKeys[(pce)][(sq)]))
#define HASH_CA (pos->posKey ^= (CastleKeys[(pos->castlePerm)]))
#define HASH_SIDE (pos->posKey ^= (SideKey))
#define HASH_EP (pos->posKey ^= (PieceKeys[EMPTY][(pos->enPas)]))


const int CastlePerm[120] = {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 13, 15, 15, 15, 12, 15, 15, 14, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15,  7, 15, 15, 15,  3, 15, 15, 11, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15
};

static void ClearPiece(const int sq, S_BOARD *pos) {
	ASSERT(sqOnBoard(sq));
	
    int pce = pos->pieces[sq];
	
    ASSERT(pieceValid(pce));
	
	int col = PieceCol[pce];
	int index = 0;
	int t_pceNum = -1;
	
    HASH_PCE(pce,sq);
	
	pos->pieces[sq] = EMPTY;
    pos->material[col] -= PieceVal[pce];
	
	if(PieceBig[pce]) {
			pos->bigPce[col]--;
		if(PieceMaj[pce]) {
			pos->majPce[col]--;
		} else {
			pos->minPce[col]--;
		}
	} else {
		CLRBIT(pos->pawns[col], Sq120ToSq64[sq]);
		CLRBIT(pos->pawns[BOTH], Sq120ToSq64[sq]);
	}
	
	for (index = 0; index < pos->pceNum[pce]; ++index) {
		if (pos->pList[pce][index] == sq) {
			t_pceNum = index;
			break;
		}
	}
	
	ASSERT(t_pceNum != -1);
	pos->pceNum[pce]--;	
	
	pos->pList[pce][t_pceNum] = pos->pList[pce][pos->pceNum[pce]];
}

static void AddPiece(const int sq, S_BOARD *pos, const int pce) {

    ASSERT(pieceValid(pce));
    ASSERT(sqOnBoard(sq));
	
	int col = PieceCol[pce];

    HASH_PCE(pce,sq);
	
	pos->pieces[sq] = pce;

    if(PieceBig[pce]) {
			pos->bigPce[col]++;
		if(PieceMaj[pce]) {
			pos->majPce[col]++;
		} else {
			pos->minPce[col]++;
		}
	} else {
		SETBIT(pos->pawns[col], Sq120ToSq64[sq]);
		SETBIT(pos->pawns[BOTH], Sq120ToSq64[sq]);
	}
	
	pos->material[col] += PieceVal[pce];
	pos->pList[pce][pos->pceNum[pce]++] = sq;
}

static void MovePiece(const int from, const int to, S_BOARD *pos) {
    ASSERT(sqOnBoard(from));
    ASSERT(sqOnBoard(to));
	
	int index = 0;
	int pce = pos->pieces[from];	
	int col = PieceCol[pce];
	
#ifdef DEBUG
	int t_PieceNum = false;
#endif

	HASH_PCE(pce,from);
	pos->pieces[from] = EMPTY;
	
	HASH_PCE(pce,to);
	pos->pieces[to] = pce;
	
	if (!PieceBig[pce]) {
		CLRBIT(pos->pawns[col], Sq120ToSq64[from]);
		CLRBIT(pos->pawns[BOTH], Sq120ToSq64[from]);
		SETBIT(pos->pawns[col], Sq120ToSq64[to]);
		SETBIT(pos->pawns[BOTH], Sq120ToSq64[to]);		
	}    
	
	for (index = 0; index < pos->pceNum[pce]; ++index) {
		if (pos->pList[pce][index] == from) {
			pos->pList[pce][index] = to;
#ifdef DEBUG
			t_PieceNum = true;
#endif
			break;
		}
	}
	ASSERT(t_PieceNum);
}

int MakeMove(S_BOARD *pos, int move) {

	ASSERT(checkBoard(pos));
	
	int from = FROMSQ(move);
    int to = TOSQ(move);
    int side = pos->side;
	
	ASSERT(sqOnBoard(from));
    ASSERT(sqOnBoard(to));
    ASSERT(sideValid(side));
    ASSERT(pieceValid(pos->pieces[from]));
	
	pos->history[pos->hisPly].posKey = pos->posKey;
	
	if (move & MFLAGEP) {
        if (side == WHITE) {
            ClearPiece(to-10, pos);
        } else {
            ClearPiece(to+10, pos);
        }
    } else if (move & MFLAGCA) {
        switch(to) {
            case C1:
                MovePiece(A1, D1, pos);
				break;
            case C8:
                MovePiece(A8, D8, pos);
				break;
            case G1:
                MovePiece(H1, F1, pos);
				break;
            case G8:
                MovePiece(H8, F8, pos);
				break;
            default: ASSERT(false);
				break;
        }
    }	
	
	if (pos->enPas != NO_SQ) HASH_EP;
    HASH_CA;
	
	pos->history[pos->hisPly].move = move;
    pos->history[pos->hisPly].fiftyMove = pos->fiftyMove;
    pos->history[pos->hisPly].enPas = pos->enPas;
    pos->history[pos->hisPly].castlePerm = pos->castlePerm;

    pos->castlePerm &= CastlePerm[from];
    pos->castlePerm &= CastlePerm[to];
    pos->enPas = NO_SQ;

	HASH_CA;
	
	int captured = CAPTURED(move);
    pos->fiftyMove++;
	
	if (captured != EMPTY) {
        ASSERT(pieceValid(captured));
        ClearPiece(to, pos);
        pos->fiftyMove = 0;
    }
	
	pos->hisPly++;
	pos->ply++;
	
	if(PiecePawn[pos->pieces[from]]) {
        pos->fiftyMove = 0;
        if(move & MFLAGPS) {
            if(side==WHITE) {
                pos->enPas=from+10;
                ASSERT(RanksBrd[pos->enPas]==RANK_3);
            } else {
                pos->enPas=from-10;
                ASSERT(RanksBrd[pos->enPas]==RANK_6);
            }
            HASH_EP;
        }
    }
	
	MovePiece(from, to, pos);
	
	int prPce = PROMOTED(move);
    if(prPce != EMPTY)   {
        ASSERT(pieceValid(prPce) && !PiecePawn[prPce]);
        ClearPiece(to, pos);
        AddPiece(to, pos, prPce);
    }
	
	if(PieceKing[pos->pieces[to]]) {
        pos->KingSq[pos->side] = to;
    }
	
	pos->side ^= 1;
    HASH_SIDE;

    ASSERT(checkBoard(pos));
	
		
	if(squareAttacked(pos->KingSq[side],pos->side,pos))  {
        // TakeMove(pos);
        return false;
    }
	
	return true;
	
}