    REM
    REM --- Tiny BASIC Interpreter and Compiler Project
    REM --- Lunar Lander Demonstration Game
    REM
    REM --- Released as Public Domain by Damian Gareth Walker 2019
    REM --- Created: 15-Aug-2019
    REM

    REM --- Variables:
    REM     A: altitude
    REM     B: fuel to burn this turn
    REM     F: fuel remaining
    REM     T: time elapsed
    REM     V: velocity this turn
    REM     W: velocity next turn

    REM --- Initialise the Program
    LET A=1000
    LET B=0
    LET F=150
    LET V=50
    LET T=0

    REM --- Main Loop
100 PRINT "Time:",T," Alt:",A," Velocity:",V," Fuel:",F," Thrust:",B
111 IF F>30 THEN PRINT "Thrust (0-30)?"
    IF F<31 THEN PRINT "Thrust (0-",F,")?"
    INPUT B
    IF B>=0 THEN IF B<=30 THEN IF B<=F THEN GOTO 120
    GOTO 111
120 LET W=V-B+5
    LET F=F-B
    LET A=A-(V+W)/2
    LET V=W
    LET T=T+1
    IF A>0 THEN GOTO 100

    REM --- End of Game
    IF V<5 THEN GOTO 140
    PRINT "You crashed!"
    GOTO 160
140 IF A<0 THEN GOTO 150
    PRINT "Perfect landing!"
    GOTO 160
150 PRINT "Touchdown."
160 IF A<0 THEN LET A=0
    PRINT "Time:",T," Alt:",A," Velocity:",V," Fuel:",F
    END
