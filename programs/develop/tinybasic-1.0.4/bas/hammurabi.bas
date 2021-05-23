    REM
    REM --- Tiny BASIC Interpreter and Compiler Project
    REM --- Hammurabi Demonstration Game
    REM
    REM --- Released as Public Domain by Damian Gareth Walker, 2019
    REM --- Created: 25-Aug-2019
    REM

    REM --- Variable List
    REM
    REM A - A random number returned by the Random Number Generator
    REM B - The amount of land bought/sold by the player
    REM D - The number of people who died of starvation
    REM E - Average percentage of deaths
    REM F - The amount of grain fed to the people
    REM G - The quantity of stored grain
    REM I - The number of immigrants on a given turn
    REM L - How much land the city owns
    REM M - Maximum land that can be planted
    REM P - The number of people in the city
    REM R - The amount of grain eaten by rats
    REM S - The amount of grain planted as seed
    REM T - Time played
    REM V - The value of land per acre
    REM Y - The grain yield of each piece of land
    REM Z - The random number seed

    REM --- Initialise the random number seed
    PRINT "Think of a number."
    INPUT Z

    REM --- Initialise the game
    LET D=0
    LET E=0
    LET G=2800
    LET I=5
    LET L=1000
    LET P=100
    LET R=200
    LET T=1
    LET Y=3

    REM --- Print the report
 15 PRINT "Hammurabi, I beg to report to you,"
    PRINT "In year ",T,", ",D," people starved,"
    PRINT "and ",I," came to the city."
    PRINT "You harvested ",Y," bushels of grain per acre."
    PRINT "Rats destroyed ",R," bushels."
    PRINT "Population is now ",P,","
    PRINT "and the city owns ",L," acres of land."
    PRINT "You have ",G," bushels of grain in store."
    IF D>P*45/100 THEN GOTO 100
    IF T=11 THEN GOTO 90

    REM --- Buy land
    GOSUB 250
    LET V=17+A-A/10*10
    PRINT "Land is trading at ",V," bushels per acre."
 30 PRINT "How many acres do you wish to buy (0-",G/V,")?"
    INPUT B
    IF B>G/V THEN GOTO 30
    IF B>0 THEN GOTO 40

    REM --- Sell land
 35 PRINT "How many acres do you wish to sell (0-",L,")?"
    INPUT B
    IF B>L THEN GOTO 35
    LET B=-B

    REM --- Feed the people
 40 PRINT "How many bushels to feed the people (0-",G-B*V,")?"
    INPUT F
    IF F>=0 THEN IF F<=G-B*V THEN GOTO 45
    GOTO 40

    REM --- Plant with seed
 45 LET M=2*(G-F-B*V)
    IF 10*P<M THEN LET M=10*P
    IF L+B<M THEN LET M=L+B
 50 PRINT "How many acres do you wish to plant with seed (0-",M,")?"
    INPUT S
    IF S>M THEN GOTO 50

    REM --- Work out the result
    LET L=L+B
    LET G=G-B*V-F-S/2

    REM Yield
    GOSUB 250
    LET Y=1+A-A/5*5

    REM Rats
    GOSUB 250
    LET A=1+A-A/5*5
    LET R=0
    IF A>2 THEN GOTO 70
    GOSUB 250
    IF G>0 THEN LET R=1+A-A/G*G

    REM Recalculate grain
 70 LET G=G+S*Y-R
    IF G<0 THEN LET G=0

    REM Immigration/Birth
    GOSUB 250
    LET A=1+A-A/5*5
    LET I=A*(L+S/20)/P/5+1

    REM Feeding the people
    LET D=0
    IF P<=F/20 THEN GOTO 80
    LET D=P-F/20
    LET E=((T-1)*E+(100*D/P))/(T+1)
 80 LET P=P-D+I
    IF P<=0 THEN GOTO 210
    LET T=T+1

    REM Back to report
    PRINT ""
    GOTO 15

    REM --- Evaluation
 90 PRINT "Your reign ends after ",T-1," years."
    PRINT "You leave your city with ",P," people."
    PRINT "You have ",L," acres of land to support them."
    PRINT G," bushels of grain remain in store."
    PRINT ""
    IF E<=3 THEN IF L/P>=10 THEN GOTO 110
    IF E<=10 THEN IF L/P>=9 THEN GOTO 120
    IF E<=33 THEN IF L/P>=7 THEN GOTO 130

    REM --- Terrible performance - including premature end
100 PRINT "Your performance has been so terrible that"
    PRINT "you were driven from your throne after ",T," years!"
    END

    REM --- Best performance
110 PRINT "Your expert statesmanship is worthy of Hammurabi himself!"
    PRINT "The city will honour your memory for all eternity."
    END

    REM --- Average performance
120 PRINT "Your competent rule is appreciated by your citizens."
    PRINT "They will remember you fondly for some time to come."
    END

    REM --- Poor performance
130 PRINT "Your mismanagement left your city in a very poor state."
    PRINT "Your incompetence and oppression will not be missed by your people."
    END

    REM --- Everybody starved
210 PRINT "You have starved your entire kingdom to death!"
    END

    REM --- Random Number Generator
250 LET Z=5*Z+35
    LET Z=Z-Z/4096*4096
    LET A=Z
    RETURN
