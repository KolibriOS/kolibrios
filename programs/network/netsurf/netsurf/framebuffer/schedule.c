/*
 * Copyright 2008 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of NetSurf, http://www.netsurf-browser.org/
 *
 * NetSurf is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * NetSurf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/time.h>
#include <time.h>
#include <stdlib.h>

#include "utils/schedule.h"
#include "framebuffer/schedule.h"

#include "utils/log.h"

/* linked list of scheduled callbacks */
static struct nscallback *schedule_list = NULL;

/**
 * scheduled callback.
 */
struct nscallback
{
        struct nscallback *next;
	struct timeval tv;
	void (*callback)(void *p);
	void *p;
};


/**
 * Schedule a callback.
 *
 * \param  tival     interval before the callback should be made / cs
 * \param  callback  callback function
 * \param  p         user parameter, passed to callback function
 *
 * The callback function will be called as soon as possible after t cs have
 * passed.
 */

void schedule(int cs_ival, void (*callback)(void *p), void *p)
{
	struct nscallback *nscb;
	struct timeval tv;

        tv.tv_sec = cs_ival / 100; /* cs to seconds */
        tv.tv_usec = (cs_ival % 100) * 10000; /* remainder to microseconds */

	nscb = calloc(1, sizeof(struct nscallback));

	gettimeofday(&nscb->tv, NULL);
	timeradd(&nscb->tv, &tv, &nscb->tv);

	nscb->callback = callback;
	nscb->p = p;

        /* add to list front */
        nscb->next = schedule_list;
        schedule_list = nscb;
}

/**
 * Unschedule a callback.
 *
 * \param  callback  callback function
 * \param  p         user parameter, passed to callback function
 *
 * All scheduled callbacks matching both callback and p are removed.
 */

void schedule_remove(void (*callback)(void *p), void *p)
{
        struct nscallback *cur_nscb;
        struct nscallback *prev_nscb;
        struct nscallback *unlnk_nscb;

        if (schedule_list == NULL)
                return;

	LOG(("removing %p, %p", callback, p));

        cur_nscb = schedule_list;
        prev_nscb = NULL;

        while (cur_nscb != NULL) {
                if ((cur_nscb->callback ==  callback) &&
                    (cur_nscb->p ==  p)) {
                        /* item to remove */

                        LOG(("callback entry %p removing  %p(%p)",
                             cur_nscb, cur_nscb->callback, cur_nscb->p));

                        /* remove callback */
                        unlnk_nscb = cur_nscb;
                        cur_nscb = unlnk_nscb->next;

                        if (prev_nscb == NULL) {
                                schedule_list = cur_nscb;
                        } else {
                                prev_nscb->next = cur_nscb;
                        }
                        free (unlnk_nscb);
                } else {
                        /* move to next element */
                        prev_nscb = cur_nscb;
                        cur_nscb = prev_nscb->next;
                }
        }
}

/**
 * Process scheduled callbacks up to current time.
 *
 * @return The number of milliseconds untill the next scheduled event
 * or -1 for no event.
 */
int 
schedule_run(void)
{
	struct timeval tv;
	struct timeval nexttime;
	struct timeval rettime;
        struct nscallback *cur_nscb;
        struct nscallback *prev_nscb;
        struct nscallback *unlnk_nscb;

        if (schedule_list == NULL)
                return -1;

	/* reset enumeration to the start of the list */
        cur_nscb = schedule_list;
        prev_nscb = NULL;
	nexttime = cur_nscb->tv;

	gettimeofday(&tv, NULL);

        while (cur_nscb != NULL) {
                if (timercmp(&tv, &cur_nscb->tv, 0)) {
                        /* scheduled time */

                        /* remove callback */
                        unlnk_nscb = cur_nscb;

                        if (prev_nscb == NULL) {
                                schedule_list = unlnk_nscb->next;
                        } else {
                                prev_nscb->next = unlnk_nscb->next;
                        }

                        unlnk_nscb->callback(unlnk_nscb->p);

                        free(unlnk_nscb);

                        /* need to deal with callback modifying the list. */
			if (schedule_list == NULL)
				return -1; /* no more callbacks scheduled */
			
                        /* reset enumeration to the start of the list */
                        cur_nscb = schedule_list;
                        prev_nscb = NULL;
			nexttime = cur_nscb->tv;
                } else {
			/* if the time to the event is sooner than the
			 * currently recorded soonest event record it 
			 */
			if (timercmp(&nexttime, &cur_nscb->tv, 0)) {
				nexttime = cur_nscb->tv;
			}
                        /* move to next element */
                        prev_nscb = cur_nscb;
                        cur_nscb = prev_nscb->next;
                }
        }

	/* make rettime relative to now */
	timersub(&nexttime, &tv, &rettime);

	/*LOG(("returning time to next event as %ldms",(rettime.tv_sec * 1000) + (rettime.tv_usec / 1000))); */
	/* return next event time in milliseconds (24days max wait) */
        return (rettime.tv_sec * 1000) + (rettime.tv_usec / 1000);
}

void list_schedule(void)
{
	struct timeval tv;
        struct nscallback *cur_nscb;

	gettimeofday(&tv, NULL);

        LOG(("schedule list at %ld:%ld", tv.tv_sec, tv.tv_usec));

        cur_nscb = schedule_list;

        while (cur_nscb != NULL) {
                LOG(("Schedule %p at %ld:%ld",
                     cur_nscb, cur_nscb->tv.tv_sec, cur_nscb->tv.tv_usec));
                cur_nscb = cur_nscb->next;
        }
}


/*
 * Local Variables:
 * c-basic-offset:8
 * End:
 */
