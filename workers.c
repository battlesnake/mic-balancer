#include <math.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include "buffers.h"
#include "safe_io.h"
#include "workers.h"

struct buffer buffers[BUFFER_COUNT];

static void process_buffer(struct buffer *buffer)
{
	SAMPLE *in = buffer->data;
	SAMPLE *out = buffer->data;
	for (size_t i = 0; i < BUFFER_LENGTH / 2; i++) {
		*out = ((int32_t) in[0] - in[1]) / 2;
		out++;
		in += 2;
	}
}

static void update_meter(SAMPLE *data)
{
	static struct timeval prev;
	struct timeval now;
	gettimeofday(&now, NULL);
	uint64_t delta = ((int64_t) now.tv_sec - prev.tv_sec) * 1000 + ((int64_t) now.tv_usec - prev.tv_usec);
	if (delta < METER_INTERVAL) {
		return;
	}
	prev = now;
	size_t length = BUFFER_LENGTH / 2;
	double sums[2] = { 0, 0 };
	SAMPLE *ptrs[2] = { &data[0], &data[length] };
	for (size_t i = 0; i < length; i++) {
		for (int j = 0; j < 2; j++) {
			double value = *ptrs[j]++ / 32768.0;
			sums[j] += value * value;
		}
	}
	float dbs[2];
	for (int j = 0; j < 2; j++) {
		dbs[j] = 10 * log10f(sums[j] / length);
	}
	info("Signal level: %-3fdB          ", dbs[0]);
	info("Noise removed: %-2fdB          \x1b[2A", dbs[1] - dbs[0]);
}

void thread_record(void *p)
{
	(void) p;
	int err;
	pa_sample_spec ss = {
		.format = PA_SAMPLE_S16NE,
		.channels = 2,
		.rate = SAMPLE_RATE
	};
	pa_simple *s = pa_simple_new(NULL, "Balancer", PA_STREAM_RECORD, NULL, "Input", &ss, NULL, NULL, &err);
	if (!s) {
		error("Failed to create playback interface: %s", pa_strerror(err));
		goto fail;
	}
	info("Latency of source: %.3fms", pa_simple_get_latency(s, NULL) / 1e3);
	int index = 0;
	while (true) {
		struct buffer *buffer = &buffers[index];
		take_buffer(buffer, false);
		if (pa_simple_read(s, &buffer->data, sizeof(buffer->data), &err) < 0) {
			error("Failed to read buffer: %s", pa_strerror(err));
			abort();
		}
		give_buffer(buffer, true);
		index++;
		index %= BUFFER_COUNT;
	}
fail:
	pa_simple_free(s);
}

void thread_playback(void *p)
{
	(void) p;
	int err;
	pa_sample_spec ss = {
		.format = PA_SAMPLE_S16NE,
		.channels = 1,
		.rate = SAMPLE_RATE
	};
	pa_simple *s = pa_simple_new(NULL, "Balancer", PA_STREAM_PLAYBACK, NULL, "Output", &ss, NULL, NULL, &err);
	if (!s) {
		error("Failed to create recording interface: %s", pa_strerror(err));
		goto fail;
	}
	info("Latency of sink: %.3fms", pa_simple_get_latency(s, NULL) / 1e3);
	int index = 0;
	while (true) {
		struct buffer *buffer = &buffers[index];
		take_buffer(buffer, true);
		process_buffer(buffer);
		update_meter(buffer->data);
		if (pa_simple_write(s, &buffer->data, sizeof(buffer->data) / 2, &err) < 0) {
			error("Failed to write buffer: %s", pa_strerror(err));
			abort();
		}
		give_buffer(buffer, false);
		index++;
		index %= BUFFER_COUNT;
	}
fail:
	pa_simple_free(s);
}
