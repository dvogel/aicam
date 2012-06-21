#ifndef movingavg_h
#define movingavg_h

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

struct mavg_t {
	int16_t *samples;
	size_t length;
	size_t offset;
	bool full;

	/* These are used to automatically calculate the 
	 * average whenever a sample is added
	 */
	int64_t sum;
	int16_t value;
};

struct mavg_t *mavg_new(size_t length);
void mavg_free(struct mavg_t *buf);
void mavg_push_sample(struct mavg_t *buf, int16_t sample);
int16_t mavg_get(struct mavg_t *buf);
void mavg_printf(struct mavg_t *buf);

#endif
