#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <pulse/simple.h>
#include <pulse/error.h>

static const size_t g_buf_len = 8192;

int16_t average_amplitude(int16_t *buf, size_t nsamples)
{
    int64_t sum = 0;
    for (int16_t *p_sample = buf; p_sample <= buf + nsamples; p_sample += 2) {
        sum += abs(*p_sample);
    }
    return sum / nsamples;    
}

int main(int argc, char* argv[])
{
    static const pa_sample_spec sample_spec = {
        .format = PA_SAMPLE_S16LE,
        .rate = 44100,
        .channels = 2
    };

    pa_channel_map channel_map;
    char channel_map_descr[1024];

    pa_simple *snd = NULL;
    int ret = 1;
    int error;
    uint8_t* buf = NULL;

    pa_channel_map_init_stereo(&channel_map);
    pa_channel_map_snprint(channel_map_descr,
                           sizeof(channel_map_descr),
                           &channel_map);
    fprintf(stderr, "%s\n", channel_map_descr);

    if (! (snd = pa_simple_new(NULL, argv[0], PA_STREAM_RECORD, NULL, "record", &sample_spec, &channel_map, NULL, &error))) {
		fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        goto finish;
    }

    uint32_t buffer_number = 0;
    size_t buf_byte_len = pa_bytes_per_second(&sample_spec) / 3;
    buf = malloc(buf_byte_len);
    if (buf == NULL) {
        fprintf(stderr, __FILE__": malloc(%i) failed.", buf_byte_len);
        goto finish;
    }
    size_t sample_byte_len = pa_sample_size(&sample_spec);
    size_t buf_sample_len = buf_byte_len / sample_byte_len;
    for (;;) {

        if (pa_simple_read(snd, buf, buf_byte_len, &error) < 0) {
            fprintf(stderr, __FILE__": pa_simple_read failed: %s\n", pa_strerror(error));
            goto finish;
        }

        int16_t *left_buf = (int16_t *)buf;
        int16_t *right_buf = left_buf + 1;
        int16_t left_amplitude = average_amplitude(left_buf, buf_sample_len);
        int16_t right_amplitude = average_amplitude(right_buf, buf_sample_len);

        fprintf(stdout, "Buffer %i: (%i, %i)\n", buffer_number, left_amplitude, right_amplitude);

        buffer_number = buffer_number + 1;
    }

    ret = 0; // Success

finish:
    if (buf) {
        free(buf);
    }
    if (snd) {
        pa_simple_free(snd);
    }
    return ret;
}

