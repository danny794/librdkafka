/*
 * librd - Rapid Development C library
 *
 * Copyright (c) 2012, Magnus Edenhill
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once


#ifndef TIMEVAL_TO_TIMESPEC
#define TIMEVAL_TO_TIMESPEC(tv,ts) do {		\
    (ts)->tv_sec = (tv)->tv_sec;		\
    (ts)->tv_nsec = (tv)->tv_usec * 1000;	\
  } while (0)

#define TIMESPEC_TO_TIMEVAL(tv, ts) do {  \
    (tv)->tv_sec = (ts)->tv_sec;	  \
    (tv)->tv_usec = (ts)->tv_nsec / 1000; \
  } while (0)
#endif

#define TIMESPEC_TO_TS(ts) \
	(((rd_ts_t)(ts)->tv_sec * 1000000LLU) + ((ts)->tv_nsec / 1000))

#define TS_TO_TIMESPEC(ts,tsx) do {			\
	(ts)->tv_sec  = (tsx) / 1000000;		\
        (ts)->tv_nsec = ((tsx) % 1000000) * 1000;	\
	if ((ts)->tv_nsec >= 1000000000LLU) {		\
	   (ts)->tv_sec++;				\
	   (ts)->tv_nsec -= 1000000000LLU;		\
	}						\
       } while (0)

#define TIMESPEC_CLEAR(ts) ((ts)->tv_sec = (ts)->tv_nsec = 0LLU)


#define RD_POLL_INFINITE  -1
#define RD_POLL_NOWAIT     0



static __inline rd_ts_t rd_clock (void) RD_UNUSED;
static __inline rd_ts_t rd_clock (void) {
#ifdef __APPLE__
	/* No monotonic clock on Darwin */
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return ((rd_ts_t)tv.tv_sec * 1000000LLU) + (rd_ts_t)tv.tv_usec;
#elif defined(_MSC_VER)
	return (rd_ts_t)GetTickCount64() * 1000LLU;
#else
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ((rd_ts_t)ts.tv_sec * 1000000LLU) + 
		((rd_ts_t)ts.tv_nsec / 1000LLU);
#endif
}



/**
 * Thread-safe version of ctime() that strips the trailing newline.
 */
static __inline const char *rd_ctime (const time_t *t) RD_UNUSED;
static __inline const char *rd_ctime (const time_t *t) {
	static RD_TLS char ret[27];

#ifndef _MSC_VER
	ctime_r(t, ret);
#else
	ctime_s(ret, sizeof(ret), t);
#endif
	ret[25] = '\0';

	return ret;
}


/**
 * @brief Initialize an absolute timeout based on the provided \p timeout_ms
 *
 * To be used with rd_timeout_adjust().
 *
 * Honours RD_POLL_INFINITE, RD_POLL_NOWAIT.
 *
 * @returns the absolute timeout which should later be passed
 *          to rd_timeout_adjust().
 */
static __inline rd_ts_t rd_timeout_init (int timeout_ms) {
	if (timeout_ms == RD_POLL_INFINITE ||
	    timeout_ms == RD_POLL_NOWAIT)
		return 0;

	return rd_clock() + (timeout_ms * 1000);
}


/**
 * @brief Adjust relative timeout \p *timeout_msp for spent time using
 *        absolute timeout \p abs_timeout.
 *
 * Honours RD_POLL_INFINITE, RD_POLL_NOWAIT.
 */
static __inline void rd_timeout_adjust (rd_ts_t abs_timeout,
					int *timeout_msp) {
	if (*timeout_msp == RD_POLL_INFINITE ||
	    *timeout_msp == RD_POLL_NOWAIT)
		return; /* No change */

	*timeout_msp = (rd_ts_t)(abs_timeout - rd_clock()) / 1000;
	if (*timeout_msp < 0)
		*timeout_msp = RD_POLL_NOWAIT;
}
