#include <movingavg.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>


struct mavg_t *mavg_new(size_t length)
{
	if (length == 0) {
		return NULL;
	}

	struct mavg_t *buf = (struct mavg_t *)malloc(sizeof(struct mavg_t));
	memset(buf, 0, sizeof(struct mavg_t));

	buf->full = false;
	buf->length = length;
	buf->samples = (int16_t *)malloc(sizeof(int16_t) * length);

	return buf;
}

void mavg_free(struct mavg_t *buf)
{
	free(buf->samples);
	free(buf);
}

void mavg_push_sample(struct mavg_t *buf, int16_t sample)
{
	if (buf->full == true) {
		buf->sum -= buf->samples[buf->offset];
	}
	buf->sum += sample;

	buf->samples[buf->offset] = sample;
	buf->offset += 1;
	if (buf->offset == buf->length) {
		buf->offset = 0;
		buf->full = true;
	}

	if (buf->full == true) {
		buf->value = buf->sum / buf->length;
	} else {
		buf->value = buf->sum / buf->offset;
	}
}

int16_t mavg_get(struct mavg_t *buf)
{
	return buf->value;
}

void mavg_printf(struct mavg_t *buf)
{
	printf("\n.length = %d", buf->length);
	printf("\n.offset = %d", buf->offset);
	printf("\n.full = %d", buf->full);
	printf("\n.sum = %lu", (unsigned long)buf->sum);
	printf("\n.value = %d", buf->value);
	printf("\n");
}

