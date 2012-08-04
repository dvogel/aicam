#include <movingavg.h>

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>

#include <config.h>

static size_t mavg_sample_count = 22050;
static size_t read_byte_count = 41100;

int main(int argc, char **argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    if (argc != 2) {
        fprintf(stderr, "USAGE: %s <source>\n", argv[0]);
        goto bailout;
    }

    FILE *src = fopen(argv[1], "r");
    if (src == NULL) {
        fprintf(stderr, "Failed to open %s\n", argv[1]);
        goto bailout;
    }

    int src_fd = fileno(src);
    int flag_set = fcntl(src_fd, F_SETFL, O_NONBLOCK);
    if (flag_set == -1) {
        fprintf(stderr, "Failed to put the fifo in non-blocking mode.");
        goto bailout;
    }

    int16_t *read_buffer = (int16_t *)malloc(read_byte_count);
    if (read_buffer == NULL) {
        fprintf(stderr, "Failed to allocate read buffer of %d bytes.", read_byte_count);
        goto bailout;
    }

    struct mavg_t *sample_avg = mavg_new(mavg_sample_count);
    int16_t sample_avg_value = 0;

    bool shutdown = false;
    while (shutdown == false) {
        ssize_t bytes_read = fread(read_buffer, 1, read_byte_count, src);
        if (bytes_read < read_byte_count) {
            if (ferror(src) != 0) {
                if (errno == EAGAIN) {
                    if (bytes_read > 0) {
                        ldiv_t samples_read = ldiv(bytes_read, 2);
                        if (samples_read.rem == 1) {
                            ungetc(read_buffer[bytes_read-1], src);
                        }

                        int16_t *samples = (int16_t *)read_buffer;
                        for (ssize_t ix = 0; ix < samples_read.quot; ix++) {
                            samples[ix] = abs(samples[ix]);
                        }
                        mavg_apply_samples(sample_avg, (int16_t *)read_buffer, samples_read.quot);
                        sample_avg_value = mavg_get(sample_avg);
                        fprintf(stdout, "%d\n", sample_avg_value);
                    }
                } else {
                    fprintf(stderr, "Error while reading %s: %s\n", argv[1], strerror(errno));
                    goto bailout;
                }
            } else if (feof(src) != 0) {
                fprintf(stderr, "warning: end-of-file encountered on %s\n", argv[1]);
            }
        }
    }

    return 0;

bailout:
    return 2;
}

