#include <movingavg.h>

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/time.h>
#include <asoundlib.h>


static char *device_name = "plughw:0";

#define STR_FMT_BUF_LEN 1024
#define SAMPLE_BUFFER_FRAME_LEN 1024
#define SAMPLE_BUFFER_BYTE_LEN SAMPLE_BUFFER_FRAME_LEN * 16


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
	if (argc > 1) {
		device_name = argv[1];
	}
	setbuf(stdout, NULL);

    int exit_status = 1;

    int errno;
    char errno_string[STR_FMT_BUF_LEN];

    snd_pcm_t *pcm = 0;
    snd_pcm_sframes_t frames;
	unsigned int rate = 44100;
	int rate_dir = 0;
    snd_pcm_hw_params_t *hw_params = 0;
    unsigned char sample_buffer[SAMPLE_BUFFER_BYTE_LEN];


    /* Open the capture device */
    if ((errno = snd_pcm_open(&pcm, device_name, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
		printf("snd_pcm_open: %s\n", snd_strerror(errno));
        goto cleanup;
    }
	printf("Using device %s\n", device_name);

    if ((errno = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
        printf("snd_pcm_hw_params_malloc() returned %d.\n", errno);
        goto cleanup;
    }

    if ((errno = snd_pcm_hw_params_any(pcm, hw_params)) < 0) {
        printf("snd_pcm_hw_params_any() returned %d.\n", errno);
        goto cleanup;
    }

    if ((errno = snd_pcm_hw_params_set_access(pcm, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        printf("snd_pcm_hw_params_set_access() returned %d.\n", errno);
        goto cleanup;
    }

    if ((errno = snd_pcm_hw_params_set_format(pcm, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
		printf("snd_pcm_hw_params_set_format: %s\n", snd_strerror(errno));
		goto cleanup;
	}
    if ((errno = snd_pcm_hw_params_set_rate_near(pcm, hw_params, &rate, &rate_dir)) < 0) {
		printf("snd_pcm_hw_params_set_rate_near: %s\n", snd_strerror(errno));
		goto cleanup;
	}
    if ((errno = snd_pcm_hw_params_set_channels(pcm, hw_params, 2)) < 0) {
		printf("snd_pcm_hw_params_set_channels: %s\n", snd_strerror(errno));
		goto cleanup;
	}


    if ((errno = snd_pcm_hw_params(pcm, hw_params)) < 0) {
		printf("snd_pcm_hw_params: %s\n", snd_strerror(errno));
        goto cleanup;
    }

	struct mavg_t *left_avg = mavg_new(SAMPLE_BUFFER_FRAME_LEN);
	struct mavg_t *right_avg = mavg_new(SAMPLE_BUFFER_FRAME_LEN);

	struct timeval start_time;
	struct timeval current_time;

	gettimeofday(&start_time, NULL);
    bool shutdown = false;
    while (shutdown == false) {

        int frames_read = -1;
        frames_read = snd_pcm_readi(pcm, sample_buffer, SAMPLE_BUFFER_FRAME_LEN);
        if (frames_read != SAMPLE_BUFFER_FRAME_LEN) {
            printf("Short read from snd_pcm_readi(%d), got %d.\n", SAMPLE_BUFFER_FRAME_LEN, frames_read);
            goto cleanup;
        }

		int16_t *p_sample = (int16_t *)sample_buffer;
		while (p_sample < (int16_t *)sample_buffer + SAMPLE_BUFFER_FRAME_LEN) {
			mavg_push_sample(left_avg, abs((int)*p_sample));
			p_sample++;
			mavg_push_sample(right_avg, abs((int)*p_sample));
			p_sample++;
		}


		gettimeofday(&current_time, NULL);
		int64_t diff = timediff_milli(&current_time, &start_time);
		if (diff > 1000) {
			memcpy(&start_time, &current_time, sizeof(struct timeval));
//			gettimeofday(&start_time, NULL);
			printf("(%d, %d)\n", mavg_get(left_avg), mavg_get(right_avg));
			fflush(stdout);
		}
    }

    exit_status = 0;

cleanup:

    if (hw_params != 0) {
        snd_pcm_hw_params_free(hw_params);
    }

    if (pcm != 0) {
        snd_pcm_close(pcm);
    }

    return exit_status;
}
