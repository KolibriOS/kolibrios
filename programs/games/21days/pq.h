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

#ifndef H_PQ
#define H_PQ

#define LETTER_RETURN_TO_DAY_21                 100001
#define LETTER_WEBMASTER_CHECK_UNNAMEDSTUDIO    100002
#define LETTER_BORING_WORK                      100003
#define LETTER_PERSISTENT_AND_PATIENT           100004
#define LETTER_BORING_DEFAULT_PATH              100005
#define LETTER_TEST_OF_KNOWLEDGE                100006
#define LETTER_KNOWLEDGE_QUESTION               100007

#ifdef _KOS32
    #include "kos_vector.h"
    #define vector vector21
#else
    #include <vector>
    #include <algorithm>    // std::sort
    #define printf2 printf
    using std::vector;
#endif

enum Letter { LETTER_SHITCODE_1 = 0, LETTER_SHITCODE_2, LETTER_SHITCODE_3,
    LETTER_BOTSMANN, LETTER_ASTRA, LETTER_UNNAMEDSTUDIO_1, LETTER_UNNAMEDSTUDIO_2,
    LETTER_UNNAMEDSTUDIO_3, LETTER_UNNAMEDSTUDIO_4, LETTER_UNNAMEDSTUDIO_5,
    LETTER_ANGRYBOSS_1, LETTER_ANGRYBOSS_2, NO_POPULARITY_HINT, LETTER_FINALPATH_DEF,
    LETTER_FINALPATH_STARTUP, LETTER_FINALPATH_WORK, LETTER_FINALPATH_NOMONEY, LETTER_FINALPATH_NOPOPULARITY};
enum EventType { COURSE = 1000, SPECIAL_LETTER, MESSAGE, NEWS, INCREASEPOPULARITY,
    CHANGEPATH, CHANGESALARY, CHANGESALARYFIRSTDAY, NOPOPULARITY};

class event {
    public:
    double time;
    EventType type;
    int idata;
    event() : time(0.0), idata(-1) {}
    event(double t,EventType ty, int data = -1) : time(t), type(ty), idata(data) {}
    bool operator>(const event a) const {
        return this->time > a.time;
        }
    bool operator<(const event ev) const {
        return time < ev.time;
        }
    };

#ifdef _KOS32
static void exch (event* A, event* B) {
    event t = *A;
    *A = *B;
    *B = t;
    }
#endif

static void sort21(vector<event>* v, bool desc = false) {
#ifdef _KOS32
    // Sorry for the bubble sort
    for (unsigned int i = 0; i < v->size()-1; i++)
        for (unsigned int j = 0; j < v->size()-i-1; j++) {
            if (desc && (*v)[j] < (*v)[j+1])
                exch(&((*v)[j]), &(*v)[j+1]);
            else if (!desc && (*v)[j] > (*v)[j+1])
                exch(&(*v)[j], &(*v)[j+1]);
            }
#else
    if (desc)
        std::sort(v->begin(), v->end(), std::greater<event>());
    else
        std::sort(v->begin(), v->end());
#endif
    }

class PQ3 {
    private:
    vector<event> v;
    bool sorted;
    public:
    void sort() {
        sort21(&v, true);
        sorted = true;
        }
    PQ3() : sorted(false) {}
    int n() {
        return v.size();
        }
    bool empty() const {
        return v.empty();
        }
    void insert(event item) {
        v.push_back(item);
        sorted = false;
        }
    event delMin() {
        if (empty())
            return event();
        if (!sorted)
            sort();
        event e = v.back();
        v.pop_back();
        return e;
        }
    event* getMin() {
        if (empty())
            return 0;
        if (!sorted)
            sort();
        return &v.back();
        }
    event* get(int i) {
        if (empty() || i >= (int)v.size())
            return 0;
        if (!sorted)
            sort();
        return &v[i];
        }
    bool hasCourse(int c) {
        for(unsigned int i = 0; i < v.size(); i++)
            if (v[i].type == COURSE && v[i].idata == c)
                return true;
        return false;
        }
    bool hasCourses() {
        for(unsigned int i = 0; i < v.size(); i++)
            if (v[i].type == COURSE)
                return true;
        return false;
        }
    bool containsType(int et) {
        for (int i = v.size()-1; i >= 0; i--)
            if (v[i].type == et)
                return true;
        return false;
        }
    bool containsType(int et, int data) {
        for (int i = v.size()-1; i >= 0; i--)
            if (v[i].type == et && v[i].idata == data)
                return true;
        return false;
        }
    };


class Undo2 {
private:
    vector<event> v;
public:
    Undo2() {}
    int n() {
        return v.size();
        }
    bool empty() const {
        return v.empty();
        }
    void insert(event item) {
        if (item.type == MESSAGE)
            return;
        v.push_back(item);
        }
    event delMax() {
        if (empty())
            return event();
        event e = v.back();
        v.pop_back();
        return e;
        }
    event getMax() {
        if (empty())
            return event();
        return v.back();
        }
    event* get(int i) {
        if (empty() || i >= (int)v.size())
            return 0;
        return &v[i];
        }
    void prepareForUndo() {
        sort21(&v, false);
        }
    bool containsTypeBefore(EventType et, int data, long double t) {
        for (unsigned int i = 0; i < v.size(); i++)
            if (v[i].type == et && v[i].idata == data && v[i].time <= t)
                return true;
        return false;
        }
    };
#endif
