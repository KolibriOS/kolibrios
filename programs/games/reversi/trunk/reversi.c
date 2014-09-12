//
//	KReversi.java
//		The Othello Game, based on the algorithm of Muffy Barkocy
//		(muffy@fish.com).
//
//		The strategy is very very simple.  The best move for the computer
//		is the move that flip more pieces (preferring boards line and
//		corners) and give less pieces to move to the opponent.
//
//	Author:	Alex "Kazuma" Garbagnati (kazuma@energy.it)
//	Date:		20 Jan 96
//	L.R.:		26 Jan 96	
//	Note:
//



#include<menuet/os.h>
#include<stdint.h>
#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>
#include<string.h>

typedef unsigned int		u32;

int YSHIFT= 33;

int ENGLISH = 0;					// 
int ITALIAN = 1;					// Languages
int EXTERNAL = 2;				// 

#define BLACK  0x000000;			//
#define BLACK_S  0x333333;		//
#define WHITE  0xffffff;			// Colors
#define WHITE_S 0xaaaaaa;		//


int NOMOVE = 0;					//
int REALMOVE = 1;				// Type of move
int PSEUDOMOVE = 2;				//

int Empty = 0;					//
int User = 1;					// Board's owners
int Computer = 2;				//

#define true 1
#define false 0

int UserMove = true;
int GameOver = false;
int CopyWinOn = false;
int StillInitiated = false;				// This solve a little
										// problem on reinit.

int TheBoard[8][8];							// Board
int Score[8][8];
int OpponentScore[8][8];



	//
	// 	DrawBoard
	//		(paint the Othello Board, a 8X8 green square table)
	//
	//	Input:	graphic (Graphics)
	//	Output:	none
	//	Notes:	
	//
	void DrawBoard() {

		int i;
		for(i=0;i<=8;i++)  
		{
		  	__asm__ __volatile__("int $0x40"::"a"(38),"b"(320),"c"(YSHIFT+(YSHIFT+(40*i))*65536+40*i),"d"(0x555555));
			__menuet__line((40*i),YSHIFT,(40*i),353,0x555555);				// horizontal

		}

	}
	// End of DrawBoard


	//
	//	DrawPiece
	//		(paint a piece, black or white, I'm using an 8x8 array, so
	//		 from the input values for rows and cols must be
	//		 subtracted 1)
	//
	//	Input:	who (int),
	//			column (int),
	//			row (int)
	//	Output:	none
	//	Notes:
	//
	void DrawPiece(int Who, int Col, int Row) {
		int pCol = (40*(Col-1)+1);
		int pRow = YSHIFT+(40*(Row-1)+1);
		u32 pColor,pShadow;

		if (Who == User) {
			pColor = BLACK;
			pShadow = BLACK_S;
		} else {
			pColor = WHITE;
			pShadow = WHITE_S;
		}
		TheBoard[Col-1][Row-1] = Who;

		__menuet__bar(pCol+9,pRow+9,19,19,pColor);
	}
	// End of DrawPiece


	//
	//	MsgWhoMove
	//		(paint the message informing who's move)
	//
	//	Input:	is user ? (int)
	//	Output:	none
	//	Notes:
	//
	void MsgWhoMove(int UM) {
	}
	// End of MsgWhoMove


	//
	//	FlipRow
	//		(calculate number of pieces are flipped by a move
	//		 and return it. Eventually do the complete or pseudo
	//		 move)
	//
	//	Input:	who (int)
	//			which board (int[][])
	//			position col, row (int)
	//			direction col, row (int)
	//			make move ? (int)
	//
	int FlipRow(int Who, int WhichBoard[8][8] , int C, int R, 
	                   int CInc, int RInc, int MakeMove) {
		int NewCol;
		int NewRow;
		int Opponent = User + Computer - Who;
		int CNT = 0;

		NewCol = C - 1;
		NewRow = R - 1;
		while (true) {
			if (((NewCol+CInc) < 0) || ((NewCol+CInc) > 7) ||
			    ((NewRow+RInc) < 0) || ((NewRow+RInc) > 7)) {
				return 0;	
			}
			if (WhichBoard[NewCol+CInc][NewRow+RInc] == Opponent) {
				CNT++;
				NewCol += CInc;
				NewRow += RInc;
			} else if (WhichBoard[NewCol+CInc][NewRow+RInc] == Empty) {
				return 0;
			} else {
				break;
			}
		}
		if (MakeMove != NOMOVE) {
			C--;
			R--;
			int v;
			for(v=0; v<=CNT; v++) {
				if (MakeMove == REALMOVE) {
					DrawPiece(Who, C+1, R+1);
				} else {
					WhichBoard[C][R] = Who;
				}
				C += CInc;
				R += RInc;
			}
		}
		return CNT;
	}
	// End of FlipRow


	//
	//	IsLegalMove
	//		(verify that the move is legal)
	//
	//	Input:	who (int)
	//			board (int[][])
	//			position col, row (int)
	//	Output:	is legal ? (int)
	//	Notes:
	//
	int IsLegalMove(int Who, int WhichBoard[8][8] , int C, int R) {
		if (WhichBoard[C-1][R-1] != Empty) {
			return false;
		}
		int CInc,RInc;
		for (CInc=-1; CInc<2; CInc++) {
			for (RInc=-1; RInc<2; RInc++) {
				if (FlipRow(Who, WhichBoard, C, R, CInc, RInc, NOMOVE) > 0) {
					return true;
				}
			}
		}
		return false;
	}
	// End of IsLegalMove


	//
	//	MakeMove
	//		(make the move)
	//
	//	Input:	who (int)
	//			position col, row (int)
	//	Output:	false=EndGame, true=next player (int)
	//	Notes:
	//
	int MakeMove(int Who, int C, int R) {
		int CInc,RInc;
		for (CInc=-1; CInc<2; CInc++) {
			for (RInc=-1; RInc<2; RInc++) {
				FlipRow(Who, TheBoard, C, R, CInc, RInc, REALMOVE);
			}
		}
		if (IsBoardComplete() || 
                ((!ThereAreMoves(Computer, TheBoard)) && (!ThereAreMoves(User, TheBoard)))) {
			return false;
		}
		int Opponent = (User + Computer) - Who;
		if (ThereAreMoves(Opponent, TheBoard)) {
			UserMove = !UserMove;
		}
		return true;
	}
	// End of MakeMove


	//
	//	EndGame
	//		(shows the winning message)
	//
	//	Input:	none
	//	Output:	none
	//	Notes:
	//
	void EndGame() {
		int CompPieces = 0;
		int UserPieces = 0;
		char *WinMsg_W = "Computer Won";
		char *WinMsg_L = "User Won";
		char *WinMsg_T = "???";
		char *TheMsg;

		int StrWidth;

		int c,r;
		for (c=0; c<8; c++) {
			for (r=0; r<8; r++) {
				if (TheBoard[c][r] == Computer) {
					CompPieces++;
				} else {
					UserPieces++;
				}
			}
		}
		if (CompPieces > UserPieces) {
			TheMsg = WinMsg_W;
		} else if (UserPieces > CompPieces) {
			TheMsg = WinMsg_L;
		} else {
			TheMsg = WinMsg_T;
		}

		__menuet__write_text(100,8,0xff0000,TheMsg,strlen(TheMsg));

	}
	// End of EndGame


	//
	//	IsBoardComplete
	//		(checks if the board is complete)
	//
	//	Input:	none
	//	Output:	the board is complete ? (int)
	//	Notes:
	//
	int IsBoardComplete() {
		int i,j;
		for (i=0; i<8; i++) {
			for (j=0; j<8; j++) {
				if (TheBoard[i][j] == Empty) {
					return false;
				}
			}
		}
		return true;
	}
	// End of IsBoardComplete


	//
	//	ThereAreMoves
	//		(checks if there are more valid moves for the 
	//		 player)
	//
	//	Input:	player (int)
	//			board (int[][])
	//	Output:	there are moves ? (int)
	//	Notes:
	//
	int ThereAreMoves(int Who, int WhichBoard[8][8] ) {
		int i,j;
		for (i=1; i<=8; i++) {
			for (j=1; j<=8; j++) {
				if (IsLegalMove(Who, WhichBoard, i, j)) {
					return true;
				}
			}
		}
		return false;
	}
	// End of ThereAreMoves


	//
	//	CalcOpponentScore
	//		(calculate the totalScore of opponent after
	//		 a move)
	//
	//	Input:	position x, y (int)
	//	Output:	score (int)
	//	Notes:
	//
	int CalcOpponentScore(int CP, int RP) {
		int OpScore = 0;
		int tempBoard[8][8]; // = new int[8][8];
		int c,r;
		for (c=0; c<8; c++) {
			for (r=0; r<8; r++) {
				tempBoard[c][r] = TheBoard[c][r];
			}
		}
		int CInc,RInc;
		for (CInc=-1; CInc<2; CInc++) {
			for (RInc=-1; RInc<2; RInc++) {
				FlipRow(Computer, tempBoard, CP+1, RP+1, CInc, RInc, PSEUDOMOVE);
			}
		}
		if (ThereAreMoves(User, tempBoard)) {
			int C,R;
			for (C=0; C<8; C++) {
				for (R=0; R<8; R++) {
					OpScore += RankMove(User, tempBoard, C, R);
				}
			}
		}
		return OpScore;
	}
	// End of CalcOpponentScore()


	//
	//	RankMoves
	//		(rank all moves for the computer)
	//
	//	Input:	none
	//	Output:	none
	//	Notes:
	//
	void RankMoves() {
		int C,R;
		for (C=0; C<8; C++) {
			for (R=0; R<8; R++) {
				Score[C][R] = RankMove(Computer, TheBoard, C, R);
				if (Score[C][R] != 0) {
					OpponentScore[C][R] = CalcOpponentScore(C, R);
				} else {
					OpponentScore[C][R] = 0;
				}
			}
		}
	}
	// End of RankMoves


	//
	//	RankMove
	//		(rank a move for a player on a board)
	//
	//	Input:	who moves (int)
	//			on which board (int[][])
	//			position col, row (int)
	//	Output:	flipped pieces (int)
	//	Notes:	best are corner, then border lines,
	//			worst are line near to border lines
	//
	int RankMove(int Who, int WhichBoard[8][8], int Col, int Row) {
		int CNT = 0;
		int MV = 0;

		if (WhichBoard[Col][Row] != Empty) {
			return 0;
		}
		int CInc,RInc;
		for (CInc=-1; CInc<2; CInc++) {
			for (RInc=-1; RInc<2; RInc++) {
				MV = FlipRow(Who, WhichBoard, Col+1, Row+1, CInc, RInc, NOMOVE);
				CNT += MV;
			}
		}
		if (CNT > 0) {
			if (((Col == 0) || (Col == 7)) ||
			    ((Row == 0) || (Row == 7))) {
				CNT = 63;
			}
			if (((Col == 0) || (Col == 7)) &&
			    ((Row == 0) || (Row == 7))) {
				CNT = 64;
			}
			if ((((Col == 0) || (Col == 7)) && (Row == 1) || (Row == 6)) &&
			    (((Col == 1) || (Col == 6)) && (Row == 0) || (Row == 7)) &&
			    (((Col == 1) || (Col == 6)) && (Row == 1) || (Row == 6))) {
				CNT = 1;
			}			
		}
		return CNT;
	}
	// End of RankMove


	//
	//	BestMove
	//		(calculate and execute the best move)
	//
	//	Input:	none
	//	Output:	value, col & row (int[3])
	//	Notes:
	//
	void BestMove (int retval[3]) {

		retval[0] = -998;	// move value;
		retval[1] = 0;	// column
		retval[2] = 0;	// row

		RankMoves();
		int C,R;
		for (C=0; C<8; C++) {
			for (R=0; R<8; R++) {
				if ((Score[C][R] == 0) && (OpponentScore[C][R] == 0)) {
					Score[C][R] = -999;
				} else if (Score[C][R] != 64) {
					Score[C][R] = Score[C][R] - OpponentScore[C][R];
				}
			}
		}
		for (C=0; C<8; C++) {
			for (R=0; R<8; R++) {
				if (Score[C][R] > retval[0]) {
					retval[1] = C;
					retval[2] = R;
					retval[0] = Score[C][R]; 
				}
			}
		}
		retval[1]++;
		retval[2]++;
//		return retval;
	}
	// End of BestMove




	//
	// paint
	//
	void paint() {
//		MsgWhoMove(UserMove);
		if (!CopyWinOn) {
			int i,j;
			for (i=0; i<8; i++) {
				for (j=0; j<8; j++) {
					if (TheBoard[i][j] != Empty) {
						DrawPiece(TheBoard[i][j], i+1, j+1);
					}
				}
			}
//		} else {
//			ShowAbout();
		}

                int _user = 0;
                int _computer = 0;
                int c, r;

                for (c=0; c<8; c++)
                    for (r=0; r<8; r++) {
                            if (TheBoard[c][r] == User)
                                       _user++;
                            if (TheBoard[c][r] == Computer)
                                       _computer++;
                            }

                // do not use sprintf function here please! ( sprintf(score, "User: %d - Computer: %d", _user, _computer); )
                char score[64];
                char tmp[8];
                strcpy(score, "User (black): ");
                itoa(_user++, tmp, 10);
                strcat(score, tmp);
                strcat(score, " - Computer (white): ");
                itoa(_computer++, tmp, 10);
                strcat(score, tmp);

                __menuet__bar(58, 8, 250, 16, 0x777777);
                __menuet__write_text(58,8,0x333333,score, strlen(score));


	}
	// End of paint


	//
	// init
	//
	void init() {
			// This is the right size of the applet 321x387
//		resize(321,387);
			// I set twice the language. That's because if anybody
			//	forget a string in an external language file, are
			//	used english strings.
		if (!StillInitiated) {
			StillInitiated = true;
		}
		int i,j;
		for (i=0; i<8; i++) {
			for (j=0; j<8; j++) {
				TheBoard[i][j] = 0;
			}
		}
		TheBoard[3][3] = User;
		TheBoard[3][4] = Computer;
		TheBoard[4][3] = Computer;
		TheBoard[4][4] = User;
		UserMove = true;
//		MsgWhoMove(true);
	//	repaint();
	}
	// End of init





void paint_win(void)
{
 __menuet__window_redraw(1);
 __menuet__define_window(100,100,330,400,0x33777777,0,"Reversi");
 __menuet__make_button(4,4,40,20,3,0xe0e0e0);
 __menuet__write_text(8,8,0x333333,"New",3);
 __menuet__window_redraw(2);
}

void main(void)
{
 int i;
 u32 mouse_coord;
 u32 mouse_butn;
 int X,Y;

 int TheCol, TheRow;
 int BMove[3];
 int BX,  BY;
 int retval = false;


 __menuet__set_bitfield_for_wanted_events(EVENT_REDRAW + EVENT_KEY + EVENT_BUTTON + EVENT_MOUSE_CHANGE);
 paint_win();
 DrawBoard();
 init();
 paint();

 for(;;)
 {
  i=__menuet__wait_for_event();
  switch(i)
  {
   case 1:
    paint_win();
    DrawBoard();
    paint();
    continue;
   case 2:
    __menuet__getkey();
    continue;
   case 3:
    if(__menuet__get_button_id()==1) {
		__menuet__sys_exit();}
    else
	paint_win();
	init();
	DrawBoard();
	paint();
    continue;
   case 4:
    continue;
   case 5:
    continue;
   case 6:
	__asm__ __volatile__("int $0x40":"=a"(mouse_butn):"0"(37),"b"(2));
	__asm__ __volatile__("int $0x40":"=a"(mouse_coord):"0"(37),"b"(1));
	X = mouse_coord >> 16;
	Y = mouse_coord & 0xffff;

			// Process a normal click in the board
		BX = X;
		BY = Y - YSHIFT;


		if ((BY >= 0) && (BY <= 321) && 
		    (mouse_butn !=0) && (UserMove)) {
			TheCol = (int)((BX/40)+1);
			TheRow = (int)((BY/40)+1);

			if (IsLegalMove(User, TheBoard, TheCol, TheRow)) {
				retval = MakeMove(User, TheCol, TheRow);
				while (retval && (!UserMove)) {
					//MsgWhoMove(UserMove);
					BestMove(BMove);

					retval = MakeMove(Computer, BMove[1], BMove[2]);
					//MsgWhoMove(UserMove);
				}
				if (!retval) {
					EndGame();
				}
			}			
		paint();
		}
	continue;

	
  }
 }
}

/* We use LIBC only for strcpy/itoa, so we don't need CRT startup code */
int __bss_count;
void __crt1_startup() { main(); }
