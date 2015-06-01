/*
 * Copyright 2015 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors: Marek Olšák <maraeo@gmail.com>
 *
 */

/* The GPU load is measured as follows.
 *
 * There is a thread which samples the GRBM_STATUS register at a certain
 * frequency and the "busy" or "idle" counter is incremented based on
 * whether the GUI_ACTIVE bit is set or not.
 *
 * Then, the user can sample the counters twice and calculate the average
 * GPU load between the two samples.
 */

#include "r600_pipe_common.h"
#include "os/os_time.h"

/* For good accuracy at 1000 fps or lower. This will be inaccurate for higher
 * fps (there are too few samples per frame). */
#define SAMPLES_PER_SEC 10000

#define GRBM_STATUS		0x8010
#define GUI_ACTIVE(x)		(((x) >> 31) & 0x1)

static bool r600_is_gpu_busy(struct r600_common_screen *rscreen)
{
	uint32_t value = 0;

	rscreen->ws->read_registers(rscreen->ws, GRBM_STATUS, 1, &value);
	return GUI_ACTIVE(value);
}

static PIPE_THREAD_ROUTINE(r600_gpu_load_thread, param)
{
	struct r600_common_screen *rscreen = (struct r600_common_screen*)param;
	const int period_us = 1000000 / SAMPLES_PER_SEC;
	int sleep_us = period_us;
	int64_t cur_time, last_time = os_time_get();

	while (!p_atomic_read(&rscreen->gpu_load_stop_thread)) {
		if (sleep_us)
			os_time_sleep(sleep_us);

		/* Make sure we sleep the ideal amount of time to match
		 * the expected frequency. */
		cur_time = os_time_get();

		if (os_time_timeout(last_time, last_time + period_us,
				    cur_time))
			sleep_us = MAX2(sleep_us - 1, 1);
		else
			sleep_us += 1;

		/*printf("Hz: %.1f\n", 1000000.0 / (cur_time - last_time));*/
		last_time = cur_time;

		/* Update the counters. */
		if (r600_is_gpu_busy(rscreen))
			p_atomic_inc(&rscreen->gpu_load_counter_busy);
		else
			p_atomic_inc(&rscreen->gpu_load_counter_idle);
	}
	p_atomic_dec(&rscreen->gpu_load_stop_thread);
	return 0;
}

void r600_gpu_load_kill_thread(struct r600_common_screen *rscreen)
{
	if (!rscreen->gpu_load_thread)
		return;

	p_atomic_inc(&rscreen->gpu_load_stop_thread);
	pipe_thread_wait(rscreen->gpu_load_thread);
	rscreen->gpu_load_thread = 0;
}

static uint64_t r600_gpu_load_read_counter(struct r600_common_screen *rscreen)
{
	/* Start the thread if needed. */
	if (!rscreen->gpu_load_thread) {
		pipe_mutex_lock(rscreen->gpu_load_mutex);
		/* Check again inside the mutex. */
		if (!rscreen->gpu_load_thread)
			rscreen->gpu_load_thread =
				pipe_thread_create(r600_gpu_load_thread, rscreen);
		pipe_mutex_unlock(rscreen->gpu_load_mutex);
	}

	/* The busy counter is in the lower 32 bits.
	 * The idle counter is in the upper 32 bits. */
	return p_atomic_read(&rscreen->gpu_load_counter_busy) |
	       ((uint64_t)p_atomic_read(&rscreen->gpu_load_counter_idle) << 32);
}

/**
 * Just return the counters.
 */
uint64_t r600_gpu_load_begin(struct r600_common_screen *rscreen)
{
	return r600_gpu_load_read_counter(rscreen);
}

unsigned r600_gpu_load_end(struct r600_common_screen *rscreen, uint64_t begin)
{
	uint64_t end = r600_gpu_load_read_counter(rscreen);
	unsigned busy = (end & 0xffffffff) - (begin & 0xffffffff);
	unsigned idle = (end >> 32) - (begin >> 32);

	/* Calculate the GPU load.
	 *
	 * If no counters have been incremented, return the current load.
	 * It's for the case when the load is queried faster than
	 * the counters are updated.
	 */
	if (idle || busy)
		return busy*100 / (busy + idle);
	else
		return r600_is_gpu_busy(rscreen) ? 100 : 0;
}
