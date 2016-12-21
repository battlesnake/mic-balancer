#pragma once
#include "buffers.h"

#define SAMPLE_RATE 96000
#define BUFFER_COUNT 3

#define METER_INTERVAL 300

void thread_record(void *p);
void thread_playback(void *p);

extern struct buffer buffers[BUFFER_COUNT];
