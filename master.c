#include <movingavg.h>

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <sys/time.h>


#define STR_FMT_BUF_LEN 1024
static size_t audio_source_cnt = 2;

struct audio_src_t
{
	FILE *fp;
	int fd;
	int16_t val;
};

/* From: http://www.delorie.com/gnu/docs/glibc/libc_428.html

   Subtract the `struct timeval' values X and Y,
   storing the result in RESULT.
   Return 1 if the difference is negative, otherwise 0.  */

int
timeval_subtract (result, x, y)
	struct timeval *result, *x, *y;
{
	/* Perform the carry for the later subtraction by updating y. */
	if (x->tv_usec < y->tv_usec) {
		int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
		y->tv_usec -= 1000000 * nsec;
		y->tv_sec += nsec;
	}
	if (x->tv_usec - y->tv_usec > 1000000) {
		int nsec = (x->tv_usec - y->tv_usec) / 1000000;
		y->tv_usec += 1000000 * nsec;
		y->tv_sec -= nsec;
	}

	/* Compute the time remaining to wait.
	   tv_usec is certainly positive. */
	result->tv_sec = x->tv_sec - y->tv_sec;
	result->tv_usec = x->tv_usec - y->tv_usec;

	/* Return 1 if result is negative. */
	return x->tv_sec < y->tv_sec;
}

int64_t timediff_milli(struct timeval *b, struct timeval *a)
{
	struct timeval c;
	timeval_subtract(&c, b, a);

	int64_t m = (int64_t)(c.tv_sec * 1000);
	m += (int64_t)(c.tv_usec / 1000);
	return m;
}

int main(int argc, char *argv[])
{
	int ret = 1;
	char str_fmt_buf[STR_FMT_BUF_LEN];

	struct audio_src_t *audio_sources = NULL;
	struct pollfd *polling_data = NULL;
	int polling_status = 0;
	
	audio_sources = (struct audio_src_t *)malloc(sizeof(struct audio_src_t) * audio_source_cnt);
	if (audio_sources == NULL) {
		goto cleanup;
	}

	polling_data = (struct pollfd *)malloc(sizeof(struct pollfd) * audio_source_cnt);
	if (polling_data == 0) {
		goto cleanup;
	}

	for (size_t ix = 0; ix < audio_source_cnt; ix++) {
		snprintf(str_fmt_buf, STR_FMT_BUF_LEN, "./recording plughw:%d", ix);
		audio_sources[ix].fp = popen(str_fmt_buf, "r");
		audio_sources[ix].fd = fileno(audio_sources[ix].fp);
		audio_sources[ix].val = 0;
		polling_data[ix].fd = audio_sources[ix].fd;
	}


	struct timeval start_time;
	struct timeval current_time;
	gettimeofday(&start_time, NULL);
	while (true) {
		for (size_t ix = 0; ix < audio_source_cnt; ix++) {
			polling_data[ix].fd = audio_sources[ix].fd;
			polling_data[ix].events = POLLIN;
		}

		polling_status = poll(polling_data, audio_source_cnt, 50);
		if (polling_status > 0) {
			for (size_t ix = 0; ix < audio_source_cnt; ix++) {
				if (polling_data[ix].revents & POLLIN) {
					read(polling_data[ix].fd, &audio_sources[ix].val, sizeof(audio_sources[ix].val));
				}
			}
		}

		gettimeofday(&current_time, NULL);
		int64_t diff = timediff_milli(&current_time, &start_time);
		if (diff > 100) {
			memcpy(&start_time, &current_time, sizeof(struct timeval));
			fprintf(stdout, "\x1b[2K\x1b[1G");
			for (size_t ix = 0; ix < audio_source_cnt; ix++) {
				fprintf(stdout, "%d:%d:%d ", ix, polling_data[ix].revents & POLLIN, audio_sources[ix].val);
			}
			fflush(stdout);
		}

	}

	ret = 0;

cleanup:
	if (polling_data != NULL) {
		free(polling_data);

		for (size_t ix = 0; ix < audio_source_cnt; ix++) {
			pclose(audio_sources[ix].fp);
		}
	}
	
	if (audio_sources != NULL) {
		free(audio_sources);
	}

	return ret;
}

