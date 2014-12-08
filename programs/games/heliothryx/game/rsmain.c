#include "rsmain.h"

#include "rsgame.h"

void rs_main_init() {

    rs_app.rsAppOnInitDisplay = GameInit;
    rs_app.rsAppOnTermDisplay = GameTerm;

    rs_app.OnKeyDown = GameKeyDown;
    rs_app.OnKeyUp = GameKeyUp;

    rs_app.OnAppProcess = GameProcess;
};
