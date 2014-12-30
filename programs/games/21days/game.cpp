/******************************************************************
*   21 days: a game for programmers
*   Copyright (C) 2014 Maxim Grishin
*
*   This program is free software; you can redistribute it and/or
*   modify it under the terms of the GNU General Public License
*   as published by the Free Software Foundation; either version 2
*   of the License, or (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
*   MA  02110-1301, USA.
*******************************************************************/

#define DEFAULT_PATH  0
#define WORK_PATH     1
#define STARTUP_PATH  2

#include "sys.h"
#include "strings.h"
#include "interface.h"
#include "pq.h"

#ifdef _KOS32
    #include <menuet/os.h>
    #define vector vector21
extern "C" {
    void *__dso_handle = 0;
    void *__cxa_atexit = 0;
    void *__stack_chk_fail = 0;
    }
#else
    #define printf2 printf
    #include <stdlib.h>  //srand
    #include <vector>
    using std::vector;
    #include <unistd.h>  // usleep
#endif

#include <time.h>
#include <math.h>
using std::string;

Undo2 history;

/**************************************
*   Player
*****************************************/
long double playerMoney = 100;
int playerSalary = 0;
int playerSalaryFirstDay = 0;
int playerPath = DEFAULT_PATH;
int playerKarma = 20;
double dt = 1.0; // days per secons
long double gameTime = 0;   // Time in days
double unreadMessages = 0;
unsigned int timeHops = 0;
bool allAchievesShowed = false;
bool charityUnlocked = false;

int  playerFreelanceAttempts = 0;
bool controlFreelanceAttempts = false;

bool sentBotsmannLetter = false;
bool astraLetterSent = false;
bool shitCodeDetected = false;
bool shitLettersSent[3] = {0,0,0};
bool shitLettersFinished[3] = {0,0,0};
bool showCoursesTab = false;
PQ3 pq;  // events priority queue
bool  noPopularity = false;
short finalCardsUnlocked = 0;
bool  finalCardUnlocked[5] = {0,0,0,0,0};
bool  returnTo21HintShowed = false;
// Player Stats
time_t playerStartedPlaying;
unsigned long playerTimeHops = 0;
unsigned long playerMessagesRead = 0;
unsigned long playerHelped = 0;
unsigned long playerDidntHelped = 0;
long double playerSpentRealDays = 0;
long double playerPrevUndo = 0;
long double playerMoneyEarned = 100;
long double playerMoneySpent = 0;
double playerMoneySpentForCharity = 0;


void mainGameCycle();
int  showSettingsScreen();
int  chooseCourse();
void showAchievesScreen();

void startNewGame();
void initSendingMessage();
void undo(long double toTime);

string getStatusLine();

void addTimer(event e) {
    if (e.type == SPECIAL_LETTER &&
        (e.idata == LETTER_SHITCODE_1
        || e.idata == LETTER_SHITCODE_2
        || e.idata == LETTER_SHITCODE_3)
    && pq.containsType(e.type, e.idata))
        return;
    if (e.type == MESSAGE && pq.containsType((int)e.type))
        return;

    pq.insert(e);
    history.insert(event(e.time + gameTime, e.type, e.idata));
    }

void unlockFinalCards(short p) {
    if (!finalCardUnlocked[p])
        finalCardsUnlocked++;

    finalCardUnlocked[p] = true;
    }

void increaseMoney(long double m) {
    playerMoney += m;
    if (m > 0)
        playerMoneyEarned += m;
    if (m < 0)
        playerMoneySpent -= m;
    }

void increaseKarma(int k, bool addTimerMsg = true) {
    if (playerKarma <= 0 && playerKarma+k>0)
        initSendingMessage();
    int oldKarma = playerKarma;
    playerKarma += k;
    if (playerKarma < -100)
        playerKarma = -100;

    if (playerKarma > 100)
        playerKarma = 100;

    if (addTimerMsg && playerKarma - oldKarma != 0)
        history.insert(event(2*dt, INCREASEPOPULARITY, playerKarma - oldKarma));
    }

void changePath(int to) {
    history.insert(event(gameTime, CHANGEPATH, playerPath));
    playerPath = to;
    }

void changeSalary(int to) {
    history.insert(event(gameTime, CHANGESALARY, playerSalary));
    playerSalary = to;
    }

void changeSalaryFirstDay(int to) {
    history.insert(event(gameTime, CHANGESALARYFIRSTDAY, playerSalaryFirstDay));
    playerSalaryFirstDay = to;
    }

int karma() {
    return playerKarma;
    }

void initSendingMessage() {
    double newTime = (karma() <= 0)?0.0:0.5 ;//* karma() / 100.0;
    if (newTime > 0) {
        addTimer(event(newTime, MESSAGE));
        }
    else {
        noPopularity = true;
        addTimer(event(0, NOPOPULARITY));
        }
    }

void checkAchieves() {
    /// No popularity
    if (!achievesPicked[1] && noPopularity)
        achievesPicked[1] = true;
    /// Millionaire
    if (!achievesPicked[3] && playerMoney >= 1000000.0) {
        achievesPicked[3] = true;
        charityUnlocked = true;
        }
    /// Time jumper
    if (!achievesPicked[8] && timeHops >= 3)
        achievesPicked[8] = true;

    allAchievesShowed = true;
    /// Show achieve notification
    for (unsigned int i = 0; i < achievesNumber; i++) {
        if (achievesPicked[i] && !achievesShowed[i]) {
            drawModalWindow(achieves[i], newAchievement);
            wait();
            if (i == 3) {
                drawModalWindow(pressRForCharity, 0, "[R]");
                wait('r','R');
                }
            achievesShowed[i] = true;
            }
        else if (!achievesShowed[i])
            allAchievesShowed = false;
        }
    }

void showMainMenu() {
    string s = days21;
    s+=mainMenuText;
    drawWindow(s, gameName, "", txt("%s    %s", one_or_two, developer));
    char menu = getAnswer('1', '2');

    switch(menu) {
        case '1':
            startNewGame();
        break;
        case '2': // TODO: Save?

        break;
        }
    }

void showAchievesScreen() {
    string s;
    for (unsigned short i = 0; i < achievesNumber; i++)
        s += txt("[%c] %s \n", achievesPicked[i]?'X':' ', achieves[i]);

    s += txt(finalCardsUnlockedText, finalCardsUnlocked, 5);
    char totalCoursesCount = 12;
    s += txt(coursesFinishedText, (int)finishedCoursesCount, (int)totalCoursesCount);
    drawWindow(s, achievementsTitle, getStatusLine(), enter);
    }

string printExactTask(int course, int task, int word) {
    switch (course) {
        case 1:
        case 2:
            return string(programmingTasks[task]);
            break;
        case 3:
        case 4:
            return string(mobileTasks[task]);
            break;
        case 5:
            return string(webProgrammingTasks[task]);
            break;
        case 6:
            return string(systemProgrammingTask[task]);
            break;
        case 7:
            return string(adminTasks[task]);
            break;
        case 0:
        default:
            return txt(level1Task[task], level1TaskWords[word]);
            break;
        }
    }

void showLearnDialog() {
    string s = selectCourseText;
    for (unsigned short i = 0; i < coursesNumber; i++) {
        if (!coursesUnlocked[i])
            continue;

        if (i == 2) {
            s+=txt("3.%s\n", courses[2]);
            for (int j = 0; j < advancedProgrammingCourses; j++) {
                s+=txt("     %c.%s [%c]", (char)('a'+j), advProgCourses[j], (advProgCoursesFinished[j])?'X':' ');
                if (!advProgCoursesFinished[j] && !pq.hasCourse('a'+j))
                    s+=txt(takeCourseCost, advProgLearningCost[j]);
                else if (pq.hasCourse('a'+j))
                    s+=takingThisCourse;
                else
                    s+="\n";
            }
            continue;
        }

        s+=txt("%c.%s [%c]", (char)('1'+i), courses[i], (coursesFinished[i])?'X':' ');
        if (!coursesFinished[i] && !pq.hasCourse(i))
            s+=txt(takeCourseCost, learningCost[i]);
        else if (pq.hasCourse(i)) {
            s+=takingThisCourse;
        }
        else
            s+="\n";
    }

    drawWindow(s, coursesTitle ,"", number_or_enter);
    unsigned char answer = 0;
    answer = getKey();

    if (answer == ENTER_KEY)
        return;

    else if (answer >='1' && answer <=(char)('0'+coursesNumber) && answer != '3') {
        answer -- ;
        answer -='0';

        if (coursesUnlocked[answer]){
            if (coursesFinished[answer]) {
                drawModalWindow(courseAlreadyPassed, errorMsg);
                wait();
                showLearnDialog();
                return;
                }
            else if (playerMoney > learningCost[answer] && !pq.hasCourse(answer)) {
                drawModalWindow(txt(successfullyEnrolled, courses[answer]).c_str(), congratsMsg);
                wait();
                increaseMoney(-learningCost[answer]);
                addTimer(event(learningTime[answer], COURSE, answer));
                }
            else if (playerMoney <= learningCost[answer] && !pq.hasCourse(answer)){
                drawModalWindow(notEnoughMoney, errorMsg);
                wait();
                showLearnDialog();
                }
            else
                showLearnDialog();
            return;
            }
        else
            showLearnDialog();
        }
    else if (answer == '3' && coursesUnlocked[2]) {
        drawModalWindow(ae_advanced_courses,errorMsg);
        wait();
        showLearnDialog();
        }
    /// only small letters
    else if (answer >='a' && answer <= 'e') {
        answer -= 'a';
        if (coursesUnlocked[2] && playerMoney > advProgLearningCost[answer] && !advProgCoursesFinished[answer] && !pq.hasCourse('a'+answer)) {
            drawModalWindow(txt(successfullyEnrolled,advProgCourses[answer]).c_str(), congratsMsg);
            wait();

            increaseMoney(-advProgLearningCost[answer]);
            int learnCourseNumber = answer+'a';
            if (advProgLearningTime[answer] != 0 && learnCourseNumber != -1)
                addTimer(event(advProgLearningTime[answer], COURSE, learnCourseNumber));
            return;
            }
        else if (coursesUnlocked[2] && playerMoney <= advProgLearningCost[answer] && !advProgCoursesFinished[answer] && !pq.hasCourse('a'+answer)) {
            drawModalWindow(notEnoughMoney, errorMsg);
            wait();
            showLearnDialog();
            }
        else if (advProgCoursesFinished[answer]) {
            drawModalWindow(courseAlreadyPassed, errorMsg);
            wait();
            showLearnDialog();
            return;
            }
        else
            showLearnDialog();
        }
    else if (answer == '3') {
        showLearnDialog();
        }
    else if (coursesUnlocked[2]) {
        drawModalWindow(ae_advanced_courses,errorMsg);
        wait();
        showLearnDialog();
        }
    else
        showLearnDialog();
    return;
    }

void startNewGame() {
    // Intro
    drawWindow(gameIntroTextPart1, introTitle, "", enter, true);
    wait();
    drawWindow(gameIntroTextPart2, introTitle, "", enter, true);
    wait();
    drawWindow(gameIntroTextPart3, introTitle, "", enter, true);
    wait();
    // Goals
    drawWindow(gameIntroPlan, 0, "", enter, true);
    wait();
    // Desktop
    drawWindow("", introDesktop, "", enter);
    wait();
    // Top line
    drawWindow(introStatusLine,"", getStatusLine(), enter);
    wait();
    // Keys
    drawWindow("", "", "", introAllowedKeys);
    wait('1');
    // Dialog window
    drawModalWindow(introFinished, introLetsBegin);
    wait();
    // Start game!
    mainGameCycle();
    }

void showFinalStats() {
    // Show player stats
    int minutesInGame = (time(NULL) - playerStartedPlaying)/60;
    playerSpentRealDays += (gameTime - playerPrevUndo);
    long unsigned int val[9] = {
        static_cast<long unsigned int>(playerSpentRealDays),
        static_cast<long unsigned int>(minutesInGame),
        playerMessagesRead,
        playerHelped,
        playerDidntHelped,
        static_cast<long unsigned int>(playerMoneyEarned),
        static_cast<long unsigned int>(playerMoneySpent),
        static_cast<long unsigned int>(playerMoneySpentForCharity),
        playerTimeHops};
    string s = "\n<c>";
    s += playerStatsTitle;
    for (int i = 0; i < 9; i++) {
        s += txt(playerStats[i], static_cast<int>(val[i]));
        if (i == 0 || i == 3 || i == 4)
            s += getWordEnding(val[i], i);
        }
    s += playerStatsEnd;
    drawWindow(s);
    wait(ESCAPE_KEY, ESCAPE_KEY);
    }

bool isGameOver() {
    if (noPopularity && !finalCardUnlocked[3] && !pq.containsType(SPECIAL_LETTER, LETTER_FINALPATH_NOPOPULARITY)) {
        addTimer(event(2.0, SPECIAL_LETTER, LETTER_FINALPATH_NOPOPULARITY));
        return false;
        }

    if ((playerPath == DEFAULT_PATH || playerPath == STARTUP_PATH) && playerMoney < 0.0 && !finalCardUnlocked[4]  && !pq.containsType(SPECIAL_LETTER, LETTER_FINALPATH_NOMONEY)) {
        addTimer(event(2.0, SPECIAL_LETTER, LETTER_FINALPATH_NOMONEY));
        return false;
        }
    // You win!
    if (finishedCoursesCount >= 12 && finalCardsUnlocked >= 5 && gameTime <= 22.0 && allAchievesShowed) {
        std::string s = youWin;
        s += gameOver;
        drawWindow(s, congratsMsg, "", pressF);
        wait('f','F');
        showFinalStats();
        return true;
        }
    // Game over
    if (playerMoney < 0 && karma() < 0 && noPopularity && floor(unreadMessages) == 0 && gameTime > 22.0 &&
        achievesShowed[1] && finalCardUnlocked[3] && finalCardUnlocked[4] &&
        !pq.containsType(SPECIAL_LETTER) && !pq.containsType(MESSAGE)) {
        string s = gameOverLogo;
        s += gameOverText;
        drawWindow(s, gameOverTitle, "", pressF);
        wait('f','F');
        showFinalStats();
        return true;
        }
    return false;
    }

int getAvailableCoursesCount() {
    int a = 0;
    for (unsigned int i = 0; i < coursesNumber; i++) {
        if (i!= 2 && coursesUnlocked[i] && !coursesFinished[i])
            a++;
        }
    // Anvanced programming
    if (coursesUnlocked[2]) {
        for (int i = 0; i < advancedProgrammingCourses; i++)
            if (!advProgCoursesFinished[i])
                a++;
        }
    return a;
    }

std::string getStatusLine() {
    string s = txt(statusLine, (int)gameTime, (int)playerMoney, karma(), (int)floor(unreadMessages));
    if (showCoursesTab)
        s += txt(statusLineCoursesNotFinished, getAvailableCoursesCount());
    return s;
    }

string getBottomLine() {
    string s;
    if (showCoursesTab)
        s += bottomLineCourses;
    s += bottomLineMsgAchieves;
    if (charityUnlocked) {
        s += charityTitle;
        s += "[R]    ";
        }
    s += bottomLineSpeedAndExit;
    return s;
    }

int showUnreadMessages() {
    int newTaskLevel = chooseCourse();

    unsigned int task = rand() % taskCount[newTaskLevel];
    unsigned int word = rand() % level1TaskWordNumber;
    unsigned int yes =  rand() % levelYesAnswerNumber;
    unsigned int no  =  rand() % levelNoAnswerNumber;

    string s = printExactTask(newTaskLevel, task, word);
    s += txt(yourAnswer,levelYesAnswer[yes],levelNoAnswer[no]);
    string buttons = one_or_two;
    buttons += escToBreakReading;
    drawWindow(s, unread_message, getStatusLine(), buttons);
    return newTaskLevel;
    }

bool messageGetAnswer(int level) {
    long double reward = taskReward[level] + rand() % 20;
    char answerKey = getAnswer('1', '2', ESCAPE_KEY);

    switch (answerKey) {
        case '1':
            // Player Stats
            playerHelped++;
            increaseMoney(reward);
            increaseKarma(reward/5.0, false);
            if (playerPath == WORK_PATH && controlFreelanceAttempts) {
                playerFreelanceAttempts++;
                if (playerFreelanceAttempts > 5) {
                    addTimer(event(1.0, SPECIAL_LETTER, LETTER_ANGRYBOSS_2));
                    controlFreelanceAttempts = false;
                    }
                }
            // Repairing computer takes 0.5 days
            gameTime += 0.5;
            break;
        case '2':
            // Player Stats
            playerDidntHelped++;
            increaseKarma(-reward/2.0, false);
            break;
        case ESCAPE_KEY:
            return true;
            break;
        }
    return false;
    }

double showChangeSpeedDialog(string e = "") {
    string s;
    if (dt >= 1.0)
        s = txt(daysPerSecond, (int)dt);
    else {
        s = txt(daysPerSecond, 0);
        s += txt(".%d", (int)(dt*10)%10); // Actually not the best way of printing double values
        }
    if (!e.empty())
        s += e;
    drawModalWindow(s.c_str(), changeSpeedTitle, changeSpeedButtons);
    char ch = getAnswer('+','-','Y','y');
    if (ch == 'y' || ch == 'Y')
        return dt;
    else if (ch == '+') {
        if (dt >= 1.0)
            dt += 1.0;
        else
            dt += 0.10;
        if (dt >= 10.0) {
            dt = 10.0;
            return showChangeSpeedDialog(" max");
            }
        return showChangeSpeedDialog();
        }
    else if (ch == '-') {
        if (dt >= 2.0)
            dt -= 1.0;
        else
            dt -= 0.10;
        if (dt <= 0.10) {
            dt = 0.10;
            return showChangeSpeedDialog(" min");
            }
        return showChangeSpeedDialog();
        }
    return dt;
    }

// returns 1 if program have to be closed
bool keyPressed(char k) {
    switch (k) {
        // Courses
        case 'c':
        case 'C':
            if (!showCoursesTab)
                break;
            showLearnDialog();
            break;
        // Achieves
        case 'a':
        case 'A':
            showAchievesScreen();
            wait();
            break;
        // Messages
        case 'm':
        case 'M': {
            int oldPopularity = karma();
            // Player Stats
            bool breakReading = false;
            while (floor(unreadMessages) > 0) {
                breakReading = messageGetAnswer(showUnreadMessages());
                if (breakReading) {
                    break;
                    }
                else {
                    if (floor(unreadMessages) > 0.0) unreadMessages--;
                    playerMessagesRead++;
                    }
                }
            if (!breakReading) {
                drawModalWindow(noUnreadMessages);
                wait();
                }
            if (karma() != oldPopularity)
                history.insert(event(gameTime, INCREASEPOPULARITY, karma()-oldPopularity));
            // Hack
            #ifdef _KOS32
            initSendingMessage();
            #endif
            }
            break;
        case 'u':
        case 'U':
            undo(gameTime - 10);
            break;
        case 'r':
        case 'R': {
            if (!charityUnlocked)
                break;

            if (playerMoney <= 0) {
                drawModalWindow(notEnoughMoneyForCharity, 0, enter);
                wait();
                break;
                }
            drawModalWindow(charityQuestion, charityTitle, yesNoDialog);
            char ch = getAnswer('y', 'n', 'Y', 'N');
            if (ch == 'y' || ch == 'Y') {
                // Player Stats
                playerMoneySpentForCharity += (playerMoney / 2.0);

                increaseMoney(-playerMoney / 2.0);
                increaseKarma(abs(karma()));
                }
            }
            break;
        case 's':
        case 'S':
            dt = showChangeSpeedDialog();
            break;
        // Escape
        case ESCAPE_KEY: {
            drawModalWindow(doYouReallyWantToExit,0 , yesNoDialog);
            char ans = getAnswer('y','Y','n','N');
            if (ans == 'y' || ans == 'Y')
                return true;
            }
            break;
        }
    return false;
    }

void breakingNews(int newsID) {
    drawWindow(news[newsID], breaking_news, getStatusLine(), enter);
    wait();
    }

void checkPq() {
    vector<event> lateEvents;
    // Messages in PQ are in reverse order
    for (int i = pq.n()-1; i >= 0; i--) {
        event* e = pq.get(i);
        if (e->time > dt) {
            e->time -= dt;
            continue;
            }
        switch (e->type) {
            case COURSE: {
                string s;

                if (e->idata >= 0 && e->idata < (int)coursesNumber) {
                    s += txt(courseSuccessfullyFinished,courses[e->idata]);
                    // Not the best fix for the bug with mobile dev
                    if (!coursesFinished[e->idata])
                        finishedCoursesCount++;
                    coursesFinished[e->idata] = true;
                    }
                // Advanced programming
                else if (e->idata >='a' && e->idata <= 'e') {
                    e->idata -= 'a';
                    s += txt(courseSuccessfullyFinished,advProgCourses[e->idata]);
                    if (!advProgCoursesFinished[e->idata])
                        finishedCoursesCount++;
                    advProgCoursesFinished[e->idata] = true;
                    }
                increaseKarma(+5);

                pq.delMin();
                drawModalWindow(s.c_str(), congratsMsg);
                wait();
                }
                break;
            case SPECIAL_LETTER:
                switch (e->idata) {
                    case LETTER_RETURN_TO_DAY_21:
                        drawModalWindow(returnToDay21, 0, pressP);
                        wait('p','P');
                        returnTo21HintShowed = true;
                        break;
                    case LETTER_SHITCODE_1:
                    case LETTER_SHITCODE_2:
                    case LETTER_SHITCODE_3: {
                        if (playerPath == WORK_PATH)
                            break;
                        string s;
                        int letterIndex = e->idata*2;
                        s += specialLetters[letterIndex];
                        s += shitCodeYourAnswer;
                        drawWindow(s, new_letter, getStatusLine(), one_or_two);

                        char ch = getAnswer('1', '2');
                        if (ch == '1') {
                            if (e->idata == LETTER_SHITCODE_1 && advProgCoursesFinished[0] && advProgCoursesFinished[1]) {
                                // sussessfull
                                increaseMoney(300.0);
                                increaseKarma(+5);
                                drawWindow(txt(specialLetters[24], (int)gameTime, 300), answerLetter, getStatusLine(), enter);
                                shitLettersFinished[0] = true;
                                }
                            else if (e->idata == LETTER_SHITCODE_2 && advProgCoursesFinished[2]) {
                                // sussessfull
                                increaseMoney(500.0);
                                increaseKarma(+5);
                                drawWindow(txt(specialLetters[24], (int)gameTime, 500), answerLetter, getStatusLine(), enter);
                                shitLettersFinished[1] = true;
                                }
                            else if (e->idata == LETTER_SHITCODE_3 && advProgCoursesFinished[3]) {
                                // sussessfull
                                increaseMoney(800.0);
                                increaseKarma(+5);
                                drawWindow(txt(specialLetters[24], (int)gameTime, 800), answerLetter, getStatusLine(), enter);
                                shitLettersFinished[2] = true;
                                }
                            else {
                                drawModalWindow(specialLetters[letterIndex+1], answerLetter);
                                coursesUnlocked[2] = true;
                                shitCodeDetected = true;
                                increaseKarma(-5);
                                }

                            if (shitLettersFinished[0] && shitLettersFinished[1] && shitLettersFinished[2])
                                achievesPicked[0] = true;

                            wait();
                            }
                        else if (ch == '2') {
                            increaseKarma(-2);
                            }
                        }
                        break;
                    case LETTER_BOTSMANN:
                        if (playerPath == DEFAULT_PATH) {
                            string s = specialLetters[6];
                            drawWindow(s, new_letter, getStatusLine(), enter);
                            wait();
                            lateEvents.push_back(event(10.0, SPECIAL_LETTER, LETTER_ASTRA));
                            }
                        break;
                    case LETTER_ASTRA:
                        if (playerPath == DEFAULT_PATH) {
                            string s = specialLetters[7];
                            s += shitCodeYourAnswer;

                            drawWindow(s, new_letter, getStatusLine(), one_or_two);
                            char ch = getAnswer('1', '2');
                            if (ch == '1') {
                                s = specialLetters[8];
                                drawWindow(s, new_letter, getStatusLine(), enter);
                                increaseKarma(-10);
                                }
                            else if (ch == '2') {
                                s = specialLetters[9];
                                drawWindow(s, new_letter, getStatusLine(), enter);
                                // Unlock mobile development
                                coursesUnlocked[3] = true;
                                if (!coursesFinished[3] && !pq.hasCourse(3));
                                    lateEvents.push_back(event(learningTime[3], COURSE, 3));
                                changePath(WORK_PATH);
                                changeSalary(800);
                                changeSalaryFirstDay((int)gameTime);
                                increaseKarma(+5);
                                }
                            wait();
                            }
                        break;
                    case LETTER_UNNAMEDSTUDIO_1: {
                        string s = specialLetters[10];
                        if (playerPath == WORK_PATH) {
                            lateEvents.push_back(event(15.0, SPECIAL_LETTER, LETTER_ANGRYBOSS_1));
                            drawWindow(s, new_letter, getStatusLine(), enter);
                            wait();
                            }
                        else if(playerPath == DEFAULT_PATH) {
                            s += unnamedStudio1Answer;
                            drawWindow(s, new_letter, getStatusLine(), one_or_two);
                            char ch = getAnswer('1', '2');
                            if (ch == '1') {
                                changePath(STARTUP_PATH);
                                increaseMoney(-500);
                                lateEvents.push_back(event(12.0, SPECIAL_LETTER, LETTER_UNNAMEDSTUDIO_2));
                                }
                            }
                        }
                        break;
                    case LETTER_UNNAMEDSTUDIO_2: {
                        string s = specialLetters[13];
                        if (coursesFinished[4])
                            s+=specialLetters[27];
                        else
                            s+=specialLetters[26];
                        drawWindow(s, new_letter, getStatusLine(), enter);
                        // Design basics course unlocked
                        coursesUnlocked[4] = true;
                        unnamedStudioLettersSent[1] = true;
                        wait();
                        }
                        break;
                    case LETTER_UNNAMEDSTUDIO_3: {
                        int needMoney = (int)(playerMoney * 0.5);
                        if (needMoney > 50000)
                            needMoney = 50000;
                        else if (needMoney < 0)
                            needMoney *= -1;
                        string s = txt(specialLetters[14], needMoney);
                        s += unnamedStudio3Answer;
                        drawWindow(s, new_letter, getStatusLine(), one_or_two);

                        char ch = getAnswer('1', '2');
                        if (ch == '1') {
                            increaseMoney(-needMoney);
                            lateEvents.push_back(event(15.0, SPECIAL_LETTER, LETTER_UNNAMEDSTUDIO_4));
                            }
                        else if(ch == '2') {
                            changePath(DEFAULT_PATH);
                            achievesPicked[5] = true;
                            increaseKarma(-10);
                            }
                        }
                        break;
                    case LETTER_UNNAMEDSTUDIO_4: {
                        string s = specialLetters[15];
                        // Creating web-sites
                        if (coursesFinished[5]) {
                            s+=specialLetters[29];
                            drawWindow(s, new_letter, getStatusLine(), enter);
                            wait();
                            // Waiting for 40 days
                            lateEvents.push_back(event(40.0, SPECIAL_LETTER, LETTER_WEBMASTER_CHECK_UNNAMEDSTUDIO));
                            break;
                            }
                        int needMoney = (int)(playerMoney * 0.5);
                        if (needMoney > 50000)
                            needMoney = 50000;
                        else if (needMoney < 0)
                            needMoney *= -1;
                        s += txt(specialLetters[28], needMoney);
                        s += unnamedStudio4Answer;
                        drawWindow(s, new_letter, getStatusLine(), one_two_or_three);
                        char ch = getAnswer('1', '2', '3');
                        if (ch == '1') {
                            increaseMoney(-needMoney);
                            increaseKarma(+5);
                            // Pay for the webmaster
                            lateEvents.push_back(event(40.0, NEWS, 3));
                            }
                        else if (ch == '2') {
                            coursesUnlocked[5] = true;
                            lateEvents.push_back(event(40.0, SPECIAL_LETTER, LETTER_WEBMASTER_CHECK_UNNAMEDSTUDIO));
                            }
                        else if (ch == '3') {
                            changePath(DEFAULT_PATH);
                            achievesPicked[5] = true;
                            increaseKarma(-5);
                            }
                        }
                        break;
                    case LETTER_WEBMASTER_CHECK_UNNAMEDSTUDIO:
                        if (coursesFinished[5]) {
                            lateEvents.push_back(event(1.0, NEWS, 3));
                            }
                        else {
                            // SHow letter about fail
                            drawWindow(specialLetters[16], new_letter, getStatusLine(), enter);
                            wait();
                            drawModalWindow(startupFailedTip);
                            wait();
                            increaseMoney(1000);
                            achievesPicked[5] = true;
                            increaseKarma(-15);
                            }
                        break;
                    case LETTER_UNNAMEDSTUDIO_5: {
                        drawWindow(specialLetters[17], new_letter, getStatusLine(), enter);
                        increaseMoney(200000);
                        increaseKarma(+20);
                        wait();
                        achievesPicked[2] = true;
                        lateEvents.push_back(event(5.0, SPECIAL_LETTER, LETTER_FINALPATH_STARTUP));
                        }
                        break;
                    case LETTER_ANGRYBOSS_1: {
                        string s = specialLetters[11];
                        s += angryBossAnswer;
                        drawWindow(s, new_letter, getStatusLine(), one_or_two);
                        char ch = getAnswer('1', '2');
                        if (ch == '1') {
                            changeSalary(1200);
                            coursesUnlocked[5] = true;
                            coursesUnlocked[6] = true;
                            coursesUnlocked[7] = true;
                            playerFreelanceAttempts = 0;
                            controlFreelanceAttempts = true;
                            lateEvents.push_back(event(20.0, SPECIAL_LETTER, NO_POPULARITY_HINT));
                            }
                        else if(ch == '2') {
                            changePath(STARTUP_PATH);
                            changeSalary(0);
                            lateEvents.push_back(event(9.0, SPECIAL_LETTER, LETTER_UNNAMEDSTUDIO_2));
                            achievesPicked[4] = true;
                            increaseKarma(-10);
                            }
                        }
                        break;
                    case NO_POPULARITY_HINT:
                        if (!noPopularity && controlFreelanceAttempts && playerFreelanceAttempts < 5) {
                            drawWindow(specialLetters[25], new_letter, getStatusLine(), enter);
                            wait();
                            }
                        break;
                    case LETTER_ANGRYBOSS_2:
                        drawWindow(specialLetters[12], new_letter, getStatusLine(), enter);
                        changePath(DEFAULT_PATH);
                        changeSalary(0);
                        wait();
                        increaseKarma(-20);
                        break;
                    case LETTER_BORING_WORK:
                        if (playerPath == WORK_PATH) {
                            drawWindow(specialLetters[18], new_letter, getStatusLine(), enter);
                            wait();
                            lateEvents.push_back(event(15.0, SPECIAL_LETTER, LETTER_PERSISTENT_AND_PATIENT));
                            }
                        break;
                    case LETTER_BORING_DEFAULT_PATH:
                        if (playerPath == DEFAULT_PATH && !pq.containsType(SPECIAL_LETTER, LETTER_FINALPATH_DEF
                            && !pq.containsType(SPECIAL_LETTER, LETTER_BORING_DEFAULT_PATH))) {
                            drawWindow(specialLetters[19], new_letter, getStatusLine(), enter);
                            wait();
                            lateEvents.push_back(event(5.0, SPECIAL_LETTER, LETTER_FINALPATH_DEF));
                            }
                        break;
                    case LETTER_FINALPATH_DEF:
                        if (playerPath == DEFAULT_PATH) {
                            drawWindow(defaultFinalCard, finalCard, getStatusLine(), pressF);
                            unlockFinalCards(0);
                            wait('f', 'F');
                            }
                        break;
                    case LETTER_FINALPATH_WORK:
                        if (playerPath == WORK_PATH) {
                            drawWindow(workFinalCard, finalCard, getStatusLine(), pressF);
                            unlockFinalCards(1);
                            wait('f', 'F');
                            }
                    break;
                    case LETTER_FINALPATH_STARTUP:
                        if (playerPath == STARTUP_PATH) {
                            drawWindow(startupFinalCard, finalCard, getStatusLine(), pressF);
                            unlockFinalCards(2);
                            wait('f', 'F');
                            }
                        break;
                    case LETTER_FINALPATH_NOPOPULARITY:
                        drawWindow(zeroKarmaFinalCard, finalCard, getStatusLine(), pressF);
                        unlockFinalCards(3);
                        wait('f', 'F');
                        break;
                    case LETTER_FINALPATH_NOMONEY:
                        drawWindow(noMoneyFinalCard, finalCard, getStatusLine(), pressF);
                        unlockFinalCards(4);
                        wait('f', 'F');
                        break;
                    case LETTER_PERSISTENT_AND_PATIENT:
                        achievesPicked[7] = true;
                        lateEvents.push_back(event(2.0, SPECIAL_LETTER, LETTER_FINALPATH_WORK));
                        break;
                    case LETTER_TEST_OF_KNOWLEDGE:
                        drawWindow(specialLetters[20], new_letter, getStatusLine(), enter);
                        wait();
                        nextKnowledgeLetterIndex = 0;
                        lateEvents.push_back(event(5.0, SPECIAL_LETTER, LETTER_KNOWLEDGE_QUESTION));
                        break;
                    case LETTER_KNOWLEDGE_QUESTION: {
                        if (playerPath != WORK_PATH) {
                            break;
                            }

                        if (klowledgeCorrectAnswers == 5) {
                            /// премия
                            drawWindow(specialLetters[21], new_letter, getStatusLine(), enter);
                            increaseMoney(400);
                            wait();
                            klowledgeCorrectAnswers = 0;
                            }
                        if (klowledgeUncorrectAnswers >= 5) {
                            /// Didn't pass the knowledge check
                            drawWindow(specialLetters[22], new_letter, getStatusLine(), enter);
                            wait();
                            changePath(DEFAULT_PATH);
                            changeSalary(0);
                            break;
                            }
                        if (nextKnowledgeLetterIndex > 27) {
                            /// Knowledge check finished
                            /// TODO: add a timer for this
                            drawWindow(specialLetters[23], new_letter, getStatusLine(), enter);
                            achievesPicked[6] = true;
                            wait();
                            lateEvents.push_back(event(30.0, SPECIAL_LETTER, LETTER_BORING_WORK));
                            }
                        else {
                            string s = knowledgeCheck[nextKnowledgeLetterIndex];
                            short yes = rand() % 2 + 1; // 1 or 2
                            short no = (yes == 1)?2:1;
                            s+=txt(yourAnswer, knowledgeCheck[nextKnowledgeLetterIndex+yes], knowledgeCheck[nextKnowledgeLetterIndex+no]);
                            drawWindow(s, new_letter, getStatusLine(), one_or_two);
                            char ch = getAnswer('1', '2');//getch();
                            if ((ch - '0') == yes) {
                                drawModalWindow(rightAnswer, congratsMsg);
                                klowledgeCorrectAnswers++;
                                }
                            else {
                                drawModalWindow(wrongAnswer, failMsg);
                                klowledgeUncorrectAnswers++;
                                }
                            wait();
                            nextKnowledgeLetterIndex+=3;
                            lateEvents.push_back(event(7.0, SPECIAL_LETTER, LETTER_KNOWLEDGE_QUESTION));
                            }
                        }
                        break;
                    default:
                    break;
                }
                pq.delMin();
                break;
            case MESSAGE:
                unreadMessages+=(karma() > 0)?dt * karma()/50:0;
                pq.delMin();
                lateEvents.push_back(event(2*dt, MESSAGE));
                break;
            case NEWS: {
                switch (e->idata) {
                    // Programming in the world
                    case 0:
                        showCoursesTab = true;
                        coursesUnlocked[1] = true;
                        break;
                    // Java programmers
                    case 1:
                        coursesUnlocked[2] = true;
                        break;
                    // Mobile development
                    case 2:
                        coursesUnlocked[3] = true;
                        break;
                    case 3:
                        // Your game is very popular
                        lateEvents.push_back(event(3.0, SPECIAL_LETTER, LETTER_UNNAMEDSTUDIO_5));
                        break;
                    default:
                        break;
                    }
                breakingNews(e->idata);
                pq.delMin();
                }
                break;
            case NOPOPULARITY:
            case INCREASEPOPULARITY:
                // nop
                pq.delMin();
                break;
            default:
                break;
            }
        }
    // Add new messages only after checking all existing messages
    for (unsigned short i = 0; i< lateEvents.size(); i++) {
        event e = lateEvents[i];
        if (e.type == MESSAGE) {
            initSendingMessage();
            continue;
            }
        else if (e.type == SPECIAL_LETTER && e.idata == LETTER_ASTRA)
            astraLetterSent = true;
        addTimer(e);
        }
    }

void makeStory() {
    switch (playerPath) {
        case DEFAULT_PATH:
            if (playerMoney > 200.0 && !showCoursesTab && !pq.containsType(NEWS, 0)) { // Basics of programming
                addTimer(event(10.0, NEWS, 0));
                newsShowed[0] = true;
                }
            else if (playerMoney > 300.0 && !shitCodeDetected && !shitLettersSent[0] && coursesFinished[1]) {
                addTimer(event(9.0, SPECIAL_LETTER, LETTER_SHITCODE_1));
                shitLettersSent[0] = true;
                }
            else if (playerMoney > 400.0 && !newsShowed[2] && coursesFinished[1] && !pq.containsType(NEWS, 2)) {   // Frappy Perd
                addTimer(event(9.0, NEWS, 2));
                newsShowed[2] = true;
                }
            else if (playerMoney > 500 && !sentBotsmannLetter && coursesFinished[1]) {   // Letter from Botsmann
                addTimer(event(17.0, SPECIAL_LETTER, LETTER_BOTSMANN));
                sentBotsmannLetter = true;
                }
            else if (playerMoney > 600.0 && !shitCodeDetected && !shitLettersSent[1] && coursesFinished[1] && !pq.containsType(SPECIAL_LETTER, LETTER_SHITCODE_1)) {
                addTimer(event(13.0, SPECIAL_LETTER, LETTER_SHITCODE_2));
                shitLettersSent[1] = true;
                }
            else if (playerMoney > 800.0 && sentBotsmannLetter && !unnamedStudioLettersSent[0] && coursesFinished[3] && astraLetterSent && !pq.containsType(SPECIAL_LETTER, LETTER_ASTRA)) { // Startup
                addTimer(event(15.0, SPECIAL_LETTER, LETTER_UNNAMEDSTUDIO_1));
                unnamedStudioLettersSent[0] = true;
                }
            else if (playerMoney > 1000.0 && !shitCodeDetected && !shitLettersSent[2] && coursesFinished[1] && !pq.containsType(SPECIAL_LETTER, LETTER_SHITCODE_2)) { // Send 3 letters with checking for a shit code
                addTimer(event(14.0, SPECIAL_LETTER, LETTER_SHITCODE_3));
                shitLettersSent[2] = true;
                }
            else if (getAvailableCoursesCount() == 0 && playerMoney > 2000 && sentBotsmannLetter
                && unnamedStudioLettersSent[0] && !finalCardUnlocked[0] && !pq.containsType(SPECIAL_LETTER, LETTER_BORING_DEFAULT_PATH)) {
                addTimer(event(10.0, SPECIAL_LETTER, LETTER_BORING_DEFAULT_PATH));
                }
            break;
        case WORK_PATH:
            if (playerSalary == 1200 && noPopularity && !knowledgeLetterSent && unnamedStudioLettersSent[0] && controlFreelanceAttempts && playerFreelanceAttempts <= 5) {
                addTimer(event(5.0, SPECIAL_LETTER, LETTER_TEST_OF_KNOWLEDGE));
                knowledgeLetterSent = true;
                }
            else if (playerMoney > 500.0 && !newsShowed[2] && !pq.containsType(NEWS, 2)) {  // Frappy Perd
                addTimer(event(7.0, NEWS, 2));
                newsShowed[2] = true;
                }
            else if (playerMoney > 600.0 && !unnamedStudioLettersSent[0] && coursesFinished[3]) { // Startup
                addTimer(event(14.0, SPECIAL_LETTER, LETTER_UNNAMEDSTUDIO_1));
                unnamedStudioLettersSent[0] = true;
                }
            break;
        case STARTUP_PATH:
            if (unnamedStudioLettersSent[1] && !unnamedStudioLettersSent[2] && coursesFinished[4]) {
                addTimer(event(15.0, SPECIAL_LETTER, LETTER_UNNAMEDSTUDIO_3));
                unnamedStudioLettersSent[2] = true;
                }
            else if (playerMoney > 600.0 && !shitCodeDetected && !shitLettersSent[1] && coursesFinished[1] && !pq.containsType(SPECIAL_LETTER, LETTER_SHITCODE_1)) {
                addTimer(event(14.0, SPECIAL_LETTER, LETTER_SHITCODE_2));
                shitLettersSent[1] = true;
                }
            else if (playerMoney > 1000.0 && !shitCodeDetected && !shitLettersSent[2] && coursesFinished[1] && !pq.containsType(SPECIAL_LETTER, LETTER_SHITCODE_2)) { // Send 3 letters with checking for a shit code
                addTimer(event(19.0, SPECIAL_LETTER, LETTER_SHITCODE_3));
                shitLettersSent[2] = true;
                }
            break;
        }
    // News about JAVA programmers
    if (playerMoney > 1500.0 && !newsShowed[1] && shitLettersSent[0] && shitLettersSent[1]
        && shitLettersSent[2] && !shitCodeDetected && coursesFinished[1]  && !coursesUnlocked[2]) { // JAVA programmers
        addTimer(event(5.0, NEWS, 1));
        newsShowed[1] = true;
        }
    if (!returnTo21HintShowed && finishedCoursesCount == 12 && finalCardsUnlocked == 5 && gameTime > 21.0 && allAchievesShowed && !pq.containsType(SPECIAL_LETTER, LETTER_RETURN_TO_DAY_21)) {
        addTimer(event(2.0, SPECIAL_LETTER, LETTER_RETURN_TO_DAY_21));
        }
    }

// Desktop is your main screen
string getDesktop(int &lines) {
    string s = "\n\n";
    // Information about courses
    if (!pq.hasCourses()) {
        s += noCurrentCurses;
        }
    else {
        // Check courses in pq
        s += "    ";
        s += coursesTitle;
        s += ":\n";
        for(int i = 0; i< pq.n(); i++) {
            event* e = pq.get(i);
            if (e->type == COURSE) {
                s += "     ";
                if (e->idata >= 0 && e->idata <= 7) {
                    s += courses[e->idata];
                    s += " ";
                    int percent = (int)(100 * (learningTime[e->idata] - e->time) / learningTime[e->idata]);
                    for (int i = 0; i < 20; i++) {
                        if (i > 2*percent/10)
                            s += " ";
                        else
                            s += pseudoEqual;
                        }
                    s += txt(" %d%\n", percent);
                    }
                else if (e->idata >= 'a' && e->idata <= 'e') {
                    s += advProgCourses[e->idata - 'a'];
                    s += " ";
                    int percent = (int)(100 * (advProgLearningTime[e->idata-'a'] - e->time) / advProgLearningTime[e->idata - 'a']);
                    for (int i = 0; i < 20; i++) {
                        if (i > 2*percent/10)
                            s+=" ";
                        else
                            s+=pseudoEqual;
                        }
                    s += txt(" %d%\n", percent);
                    }
                }
            }
        }

    for(unsigned int i = 0; i < s.length(); i++)
        if (s[i] == '\n')
            lines++;

    return s;
    }

void mainGameCycle() {
    float hintBegins = 0.0; //days
    unsigned short hintIndex = 0;

    playerMoney = 100;
    dt = 0.5;
    int update = 0;

    // Add initial letter
    initSendingMessage();

    string status = playerStatus;
    status += helpDesker;
    int currentPath = playerPath;

    playerStartedPlaying = time(NULL);
    string buffer, oldStatusLine;
    while (1) {
        // We update screen 10 times a second
        if (update % 10 == 0)
            increaseMoney(-dt * 1.0); // Money needs per day

        int lines = 0;

        string s = getDesktop(lines);
        for (int i = 0; i < 11-lines; i++)
            s += "\n";
        s += "    ";
        s += hintOfTheMonth;
        if (gameTime > hintBegins + 30) {
            hintBegins = gameTime;
            hintIndex++;
            if (hintIndex >= hintsCount)
                hintIndex = 0;
            }
        // This could happen when jumping in the past
        else if (hintBegins > gameTime)
            hintBegins = gameTime;

        s += "    ";
        s += hints[hintIndex];

        if (playerPath != currentPath) {
            currentPath = playerPath;
            status=playerStatus;
            if (playerPath == DEFAULT_PATH)      status += helpDesker;
            else if (playerPath == WORK_PATH)    status += worker;
            else if (playerPath == STARTUP_PATH) status += startupper;
            }
        // redraw screen only if necessary
        string newStatusLine = getStatusLine();
        if (!(s == buffer) || !(oldStatusLine == newStatusLine))
            drawWindow(s, status.c_str(), newStatusLine, getBottomLine());
        buffer = s;
        oldStatusLine = newStatusLine;
        if (update % 10 == 0) {
            checkAchieves();
            if (isGameOver())
                break;

            if (playerSalary != 0 && gameTime-playerSalaryFirstDay >= 30) {
                increaseMoney(playerSalary);
                changeSalaryFirstDay(playerSalaryFirstDay+30);
                }
            makeStory();
            checkPq();
            }

        // Some key was pressed
        if (kbhit()) {
            char k = getch();
            #ifdef _KOS32
            // We have to make a second call for special keys in KolibriOS
            if (k == 0)
                k = getch();
            #endif
            if (keyPressed(k))
                return;
            // Force redraw desktop
            buffer = "";
            oldStatusLine = "";
            }

        if (update % 10 == 0)
            gameTime += dt;

        int delayTime = 10;
        #ifdef _KOS32
        __menuet__delay100(delayTime);
        #else
        usleep(delayTime*10000);
        #endif
        update++;
        }
    }

// Jump in the past to a certain time
void undo(long double toTime) {
    if (toTime < 0 )
        return;

    if (playerMoney < 100) {
        drawModalWindow(need100ForUndo, errorMsg);
        wait();
        return;
        }
    // Jump cost: $100
    increaseMoney(-100);
    drawModalWindow(prepareForTimeJump);
    wait();

    while(!pq.empty())
        pq.delMin();

    // Player Stats
    playerTimeHops++;
    playerSpentRealDays += (gameTime - playerPrevUndo);
    playerPrevUndo = toTime;

    vector<event> lateEvents;
    history.prepareForUndo();
    while (!history.empty() && history.getMax().time > toTime) {
        event e = history.getMax();
        switch (e.type) {
            case INCREASEPOPULARITY:
                increaseKarma(-e.idata, false);
                break;
            case NOPOPULARITY:
                noPopularity = false;
                break;
            case CHANGEPATH:
                playerPath = e.idata; // Previous path
                break;
            case CHANGESALARY:
                playerSalary = e.idata;
                break;
            case CHANGESALARYFIRSTDAY:
                playerSalaryFirstDay = e.idata;
                break;
        // Letters' time is time of sending a letter + time to end timer = time to get letter in the future
        case SPECIAL_LETTER:
            switch(e.idata) {
                case LETTER_BOTSMANN:
                    sentBotsmannLetter = false;
                    astraLetterSent = false;
                    break;
                case LETTER_ASTRA:
                    if (history.containsTypeBefore(SPECIAL_LETTER, LETTER_BOTSMANN, toTime))
                        lateEvents.push_back(event(e.time - toTime, SPECIAL_LETTER, LETTER_ASTRA));
                    break;
                case LETTER_UNNAMEDSTUDIO_2:
                    // Unnamed letter 1 -> AngryBoss -> Unnamed letter 2
                    if (history.containsTypeBefore(SPECIAL_LETTER, LETTER_ANGRYBOSS_1, e.time)) {
                        if (history.containsTypeBefore(SPECIAL_LETTER, LETTER_ANGRYBOSS_1, toTime))
                            lateEvents.push_back(event(e.time - toTime, SPECIAL_LETTER, LETTER_UNNAMEDSTUDIO_2));
                        }
                    // Unnamed letter 1 -> Unnamed letter 2
                    else {
                        if (history.containsTypeBefore(SPECIAL_LETTER, LETTER_UNNAMEDSTUDIO_1, toTime))
                            lateEvents.push_back(event(e.time - toTime, SPECIAL_LETTER, LETTER_UNNAMEDSTUDIO_2));
                        }
                    break;
                case NO_POPULARITY_HINT:
                    if (history.containsTypeBefore(SPECIAL_LETTER, LETTER_ANGRYBOSS_1, toTime))
                        lateEvents.push_back(event(e.time - toTime, SPECIAL_LETTER, NO_POPULARITY_HINT));
                    break;
                case LETTER_ANGRYBOSS_1:
                    if (history.containsTypeBefore(SPECIAL_LETTER, LETTER_UNNAMEDSTUDIO_1, toTime)) {
                        lateEvents.push_back(event(e.time - toTime, SPECIAL_LETTER, LETTER_ANGRYBOSS_1));
                        controlFreelanceAttempts = false;
                        }
                    break;
                case LETTER_UNNAMEDSTUDIO_1:
                    unnamedStudioLettersSent[0] = false;
                    break;
                case LETTER_WEBMASTER_CHECK_UNNAMEDSTUDIO:
                    if (history.containsTypeBefore(SPECIAL_LETTER, LETTER_UNNAMEDSTUDIO_4, toTime))
                        lateEvents.push_back(event(e.time - toTime, SPECIAL_LETTER, LETTER_WEBMASTER_CHECK_UNNAMEDSTUDIO));
                    break;
                case LETTER_UNNAMEDSTUDIO_4:
                    if (history.containsTypeBefore(SPECIAL_LETTER, LETTER_UNNAMEDSTUDIO_3, toTime))
                        lateEvents.push_back(event(e.time - toTime, SPECIAL_LETTER, LETTER_UNNAMEDSTUDIO_4));
                    break;
                case LETTER_UNNAMEDSTUDIO_3:
                        unnamedStudioLettersSent[2] = false;
                    break;
                case LETTER_UNNAMEDSTUDIO_5:
                    if (history.containsTypeBefore(NEWS, 3, toTime) && !pq.containsType(SPECIAL_LETTER, LETTER_UNNAMEDSTUDIO_5))
                        lateEvents.push_back(event(e.time - toTime, SPECIAL_LETTER, LETTER_UNNAMEDSTUDIO_5));
                    break;
                case LETTER_FINALPATH_DEF:
                    if (history.containsTypeBefore(SPECIAL_LETTER, LETTER_BORING_DEFAULT_PATH, toTime))
                        lateEvents.push_back(event(e.time - toTime, SPECIAL_LETTER, LETTER_FINALPATH_DEF));
                    break;
                case LETTER_FINALPATH_STARTUP:
                    if (history.containsTypeBefore(SPECIAL_LETTER, LETTER_UNNAMEDSTUDIO_5, toTime) && !pq.containsType(SPECIAL_LETTER, LETTER_FINALPATH_STARTUP))
                        lateEvents.push_back(event(e.time - toTime, SPECIAL_LETTER, LETTER_FINALPATH_STARTUP));
                    break;
                case LETTER_FINALPATH_WORK:
                    if (history.containsTypeBefore(SPECIAL_LETTER, LETTER_PERSISTENT_AND_PATIENT, toTime))
                        lateEvents.push_back(event(e.time - toTime, SPECIAL_LETTER, LETTER_FINALPATH_WORK));
                    break;
                case LETTER_ANGRYBOSS_2:
                    if (e.time - toTime <= 1)
                        lateEvents.push_back(event(e.time - toTime, SPECIAL_LETTER, LETTER_ANGRYBOSS_2));
                    break;
                case LETTER_PERSISTENT_AND_PATIENT:
                    if (history.containsTypeBefore(SPECIAL_LETTER,LETTER_BORING_WORK, toTime))
                        lateEvents.push_back(event(e.time - toTime, SPECIAL_LETTER, LETTER_PERSISTENT_AND_PATIENT));
                    break;
                case LETTER_BORING_WORK:
                    if (e.time - toTime <= 40)
                        lateEvents.push_back(event(e.time - toTime, SPECIAL_LETTER, LETTER_BORING_WORK));
                    break;
                case LETTER_TEST_OF_KNOWLEDGE:
                    knowledgeLetterSent = false;
                    playerFreelanceAttempts = 0;
                    klowledgeCorrectAnswers = 0;
                    klowledgeUncorrectAnswers = 0;
                    break;
                case LETTER_SHITCODE_1:
                    shitCodeDetected = false;
                    lateEvents.push_back(event(e.time - toTime, SPECIAL_LETTER, LETTER_SHITCODE_1));
                    shitLettersSent[1] = false;
                    shitLettersSent[2] = false;
                    break;
                case LETTER_SHITCODE_2:
                    shitCodeDetected = false;
                    lateEvents.push_back(event(e.time - toTime, SPECIAL_LETTER, LETTER_SHITCODE_2));
                    shitLettersSent[2] = false;
                    break;
                case LETTER_SHITCODE_3:
                    shitCodeDetected = false;
                    lateEvents.push_back(event(e.time - toTime, SPECIAL_LETTER, LETTER_SHITCODE_3));
                    break;
                case LETTER_BORING_DEFAULT_PATH:
                    lateEvents.push_back(event(e.time - toTime, SPECIAL_LETTER, LETTER_BORING_DEFAULT_PATH));
                    break;
                }
                break;
            case NEWS:
                // News about JAVA programmers
                newsShowed[e.idata] = false;
                if (e.idata == 3) {
                    // Unnamed letter 3 -> Unnamed letter 4 -> Need webmaster -> news about Java
                    if (history.containsTypeBefore(SPECIAL_LETTER, LETTER_WEBMASTER_CHECK_UNNAMEDSTUDIO, e.time)) {
                        if (history.containsTypeBefore(SPECIAL_LETTER, LETTER_WEBMASTER_CHECK_UNNAMEDSTUDIO, toTime))
                            lateEvents.push_back(event(e.time - toTime, NEWS, 3));
                        }
                    // Unnamed letter 3 -> Unnamed letter 4 -> news about Java
                    else {
                        if (history.containsTypeBefore(SPECIAL_LETTER, LETTER_UNNAMEDSTUDIO_4, toTime))
                            lateEvents.push_back(event(e.time - toTime, NEWS, 3));
                        }
                    }
            break;
            case COURSE: {
                // e.time  - course finished
                // e.idata - course number
                long double courseStarted = 0;
                bool courseFinished = false;
                if (e.idata >='a' && e.idata <= 'e') {
                    courseStarted = e.time - advProgLearningTime[e.idata-'a'];
                    courseFinished = advProgCoursesFinished[e.idata-'a'];
                    }
                else if(e.idata >= 0 && e.idata < (int)coursesNumber) {
                    courseStarted = e.time - learningTime[e.idata];
                    courseFinished = coursesFinished[e.idata];
                    }

                if (!courseFinished && toTime >= courseStarted)
                    // Player is currently taking a course
                    lateEvents.push_back(event(e.time - toTime, COURSE, e.idata));
                    }
                break;
            case MESSAGE:
                break;
            }
        history.delMax();
        }
    unreadMessages = 0;
    gameTime = toTime;
    timeHops++;

    for (unsigned short i = 0; i< lateEvents.size(); i++)
        addTimer(lateEvents[i]);


    // Messages would not arive without this:
    if (!pq.containsType(MESSAGE))
        initSendingMessage();

    // Undo shouldn't stop during the knowledge check
    if (history.containsTypeBefore(SPECIAL_LETTER, LETTER_TEST_OF_KNOWLEDGE, toTime)
        && !history.containsTypeBefore(SPECIAL_LETTER, LETTER_BORING_WORK, toTime)) {
        double newTime = toTime-10;
        for (int i = 0; i < history.n(); i++) {
            event* e = history.get(i);
            if (e->type == SPECIAL_LETTER && e->idata == LETTER_TEST_OF_KNOWLEDGE) {
                newTime = e->time - 10;
                break;
                }
            }
        drawModalWindow(cantStopDuringKnowledgeCheck, errorMsg);
        wait();
        undo(newTime);
        }
    }

int chooseCourse() {
    double p[coursesNumber];
    long double sum = 0.0;
    for (unsigned int i = 0; i < coursesNumber; i++) {
        if (coursesFinished[i])
            p[i] = probability[i];
        else
            p[i] = 0.0;
        sum += p[i];
        }
    if (sum <= 0.001)
        return 0;
    int N = (int) (sum * 100);
    int r = rand() % N;

    int n1 = (p[0] / sum) * N;
    int n2 = (p[1] / sum) * N;
    int n3 = (p[2] / sum) * N;
    int n4 = (p[3] / sum) * N;
    int n5 = (p[4] / sum) * N;

    if (r >= 0 && r < n1)
        return 0;
    else if (coursesFinished[1] && r >= n1 && r < n1+n2)
        return 1;
    else if (coursesFinished[2] && r >= n1+n2 && r < n1+n2+n3)
        return 2;
    else if (coursesFinished[3] && r >= n1+n2+n3 && r < n1+n2+n3+n4)
        return 3;
    else if (coursesFinished[4] && r >= n1+n2+n3+n4 && r < n1+n2+n3+n4+n5)
        return 4;
    else if (coursesFinished[5] && r >= n1+n2+n3+n4+n5 && r <= N)
        return 5;

    return 0;
    }
