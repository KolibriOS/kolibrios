    REM
    REM --- Tiny BASIC Interpreter and Compiler Project
    REM --- Hunt the Hurkle Demostration Game
    REM
    REM --- Released as Public Domain by Damian Gareth Walker 2019
    REM --- Created: 11-Aug-2019
    REM

    REM --- Variables
    REM     G: hurkle column
    REM     H: hurkle row
    REM     M: moves taken
    REM     R: random number seed
    REM     X: player guess column
    REM     Y: player guess row

    REM --- Initialise the random number generator
    PRINT "Think of a number."
    INPUT R
    IF R<0 THEN LET R=0
    IF R>4095 THEN LET R=4095

    REM --- Initialise the game
    GOSUB 200
    LET G=R-(R/10*10)
    GOSUB 200
    LET H=R-(R/10*10)
    LET M=0

    REM --- Input player guess
 30 PRINT "Where is the hurkle? Enter column then row."
    INPUT X,Y
    IF X>=0 THEN IF X<=9 THEN IF Y>=0 THEN IF Y<=9 THEN GOTO 40
    PRINT "That location is off the grid!"
    GOTO 30

    REM --- Process player guess
 40 LET M=M+1
    PRINT "The Hurkle is..."
    IF G<X THEN IF H<Y THEN PRINT "...to the northwest."
    IF G=X THEN IF H<Y THEN PRINT "...to the north."
    IF G>X THEN IF H<Y THEN PRINT "...to the northeast."
    IF G>X THEN IF H=Y THEN PRINT "...to the east."
    IF G>X THEN IF H>Y THEN PRINT "...to the southeast."
    IF G=X THEN IF H>Y THEN PRINT "...to the south."
    IF G<X THEN IF H>Y THEN PRINT "...to the southwest."
    IF G<X THEN IF H=Y THEN PRINT "...to the west."
    IF G=X THEN IF H=Y THEN GOTO 60
    IF M>6 THEN GOTO 70
    PRINT "You have taken ",M," turns so far."
    GOTO 30

    REM --- Player has won
 60 PRINT "...RIGHT HERE!"
    PRINT "You took ",M," turns to find it."
    END

    REM --- Player has lost
 70 PRINT "You have taken too long over this. You lose!"
    END

    REM --- Random number generator
    REM     Input:   R - current seed
    REM     Outputs: R - updated seed
200 LET R=5*R+35
    LET R=R-R/4096*4096
    RETURN