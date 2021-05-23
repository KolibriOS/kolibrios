    REM
    REM Tiny BASIC Interpreter and Compiler Project
    REM Tic-tac-toe Sample Game
    REM
    REM Released as public domain by Damian Gareth Walker, 2019
    REM Created: 21-Sep-2019
    REM

    REM --- Variables
    REM     A   - first square in line examined
    REM     B   - second square in line examined
    REM     C   - third square in line examined
    REM     D   - player whose pieces to count
    REM     E   - number of D's pieces on a line
    REM     F   - first square of line to examine
    REM     G   - game winner
    REM     H   - which side the human takes
    REM     I   - increment for line to examine
    REM     L   - line to examine
    REM     M   - where to move (various uses)
    REM     N   - piece found in a square
    REM     P   - player currently playing
    REM     Q   - square to examine
    REM     R-Z - contents of the board

    REM --- Main Program
    GOSUB 40
    GOSUB 60
    GOSUB 80
    END

    REM --- Subroutine to initialise the game
    REM     Outputs: H - Human play order
    REM              P - Whose turn it is
 40 PRINT "Tic tac toe. Board positions are:"
    PRINT " 1  2  3"
    PRINT " 4  5  6"
    PRINT " 7  8  9"
    PRINT "Play first or second (1/2)?"
    INPUT H
    IF H<1 THEN GOTO 40
    IF H>2 THEN GOTO 40
    LET P=1
    RETURN

    REM --- Subroutine to take turns
    REM     Inputs:  H - who is the human
    REM              P - whose turn it is
    REM     Outputs: G - who won the game
 60 IF P=H THEN GOSUB 100
    IF P<>H THEN GOSUB 120
    GOSUB 200
    IF G>0 THEN RETURN
    LET P=3-P
    IF R=0 THEN GOTO 60
    IF S=0 THEN GOTO 60
    IF T=0 THEN GOTO 60
    IF U=0 THEN GOTO 60
    IF V=0 THEN GOTO 60
    IF W=0 THEN GOTO 60
    IF X=0 THEN GOTO 60
    IF Y=0 THEN GOTO 60
    IF Z=0 THEN GOTO 60
    RETURN

    REM --- Victory
    REM Inputs: H   - which side was the human
    REM         P   - player who won
 80 IF G=H THEN PRINT "You win!"
    IF G<>0 THEN IF G<>H THEN PRINT "Computer wins"
    IF G=0 THEN PRINT "A draw"
    RETURN

    REM --- Subroutine to allow the player to move
    REM     Inputs:  P   - player number
    REM     Outputs: M   - where the player wishes to move
100 PRINT "Move? "
    INPUT Q
    IF Q<1 THEN GOTO 100
    IF Q>9 THEN GOTO 100
    GOSUB 220
    IF N<>0 THEN GOTO 100
    LET M=Q
    GOSUB 240
    RETURN

    REM --- Subroutine to make the computer's move
    REM     Inputs:  P   - player number
    REM     Outputs: M   - the move chosen
120 LET M=0
    LET D=3-H
    GOSUB 145
    IF M>0 THEN GOTO 135
    LET D=H
    GOSUB 145
    IF M=0 THEN IF V=0 THEN LET M=5
    IF M=0 THEN IF R=0 THEN LET M=1    
    IF M=0 THEN IF T=0 THEN LET M=3    
    IF M=0 THEN IF X=0 THEN LET M=7    
    IF M=0 THEN IF Z=0 THEN LET M=9
    IF M=0 THEN IF S=0 THEN LET M=2    
    IF M=0 THEN IF U=0 THEN LET M=4    
    IF M=0 THEN IF Y=0 THEN LET M=8    
    IF M=0 THEN IF W=0 THEN LET M=6    
135 GOSUB 240
    PRINT "Computer move ",M
    RETURN

    REM --- Identify moves to win or avoid a loss
    REM     Inputs:  D   - player whose pieces we're counting
    REM     Changes: E   - number of pieces on line being scanned
    REM              F   - first square in winning line
    REM              I   - increment of winning line
    REM              L   - line being scanned (counter)
145 LET L=1
146 GOSUB 170
    IF E<2 THEN GOTO 152
    IF A=0 THEN LET M=F
    IF B=0 THEN LET M=F+I
    IF C=0 THEN LET M=F+I+I
    IF M>0 THEN RETURN
152 LET L=L+1
    IF L<9 THEN GOTO 146
    RETURN

    REM --- Count a player's pieces on a line
    REM     Inputs:  D   - player whose pieces we're counting
    REM              L   - line number
    REM     Changes: F   - first square on the line
    REM              I   - increment of the line
    REM              Q   - individual squares to examine
    REM     Outputs: A   - contents of first square
    REM              B   - contents of second square
    REM              C   - contents of third square
    REM              E   - number of the player's pieces
170 IF L>3 THEN GOTO 174
    LET F=3*L-2
    LET I=1
    GOTO 180
174 IF L>6 THEN GOTO 178
    LET F=L-3
    LET I=3
    GOTO 180
178 LET F=1+2*(L-7)
    LET I=4-2*(L-7)
180 LET E=0
    LET Q=F
    GOSUB 220
    LET A=N
    IF N=D THEN LET E=E+1
    LET Q=Q+I
    GOSUB 220
    LET B=N
    IF N=D THEN LET E=E+1
    LET Q=Q+I
    GOSUB 220
    LET C=N
    IF N=D THEN LET E=E+1
    RETURN

    REM --- Subroutine to check for a win
    REM     Inputs:  R-Z - board squares
    REM     Outputs: G   - the winning player (0 for neither)
200 LET G=0
    IF R>0 THEN IF R=S THEN IF S=T THEN LET G=R
    IF U>0 THEN IF U=V THEN IF V=W THEN LET G=U
    IF X>0 THEN IF X=Y THEN IF Y=Z THEN LET G=X
    IF R>0 THEN IF R=U THEN IF U=X THEN LET G=R
    IF S>0 THEN IF S=V THEN IF V=Y THEN LET G=S
    IF T>0 THEN IF T=W THEN IF W=Z THEN LET G=T
    IF R>0 THEN IF R=V THEN IF V=Z THEN LET G=R
    IF T>0 THEN IF T=V THEN IF V=X THEN LET G=T
    RETURN

    REM --- Subroutine to see what piece is in a square
    REM     Inputs:  Q   - the square to check
    REM              R-Z - the contents of the squares
    REM     Outputs: N   - the piece in that square
220 LET N=0
    IF Q=1 THEN LET N=R
    IF Q=2 THEN LET N=S
    IF Q=3 THEN LET N=T
    IF Q=4 THEN LET N=U
    IF Q=5 THEN LET N=V
    IF Q=6 THEN LET N=W
    IF Q=7 THEN LET N=X
    IF Q=8 THEN LET N=Y
    IF Q=9 THEN LET N=Z
    RETURN

    REM --- Subroutine to put a piece in a square
    REM     Inputs:  P   - the player whose piece should be placed
    REM              M   - the square to put the piece in
    REM     Changes: R-Z - the contents of the squares
240 IF M=1 THEN LET R=P
    IF M=2 THEN LET S=P
    IF M=3 THEN LET T=P
    IF M=4 THEN LET U=P
    IF M=5 THEN LET V=P
    IF M=6 THEN LET W=P
    IF M=7 THEN LET X=P
    IF M=8 THEN LET Y=P
    IF M=9 THEN LET Z=P
    RETURN
