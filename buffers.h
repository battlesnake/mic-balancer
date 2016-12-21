#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>

#define BUFFER_LENGTH 4096
#define SAMPLE int16_t

struct buffer {
	SAMPLE data[BUFFER_LENGTH];
	bool filled;
	pthread_mutex_t mx;
	pthread_cond_t cv;
};


void init_buffer(struct buffer *buffer);
void take_buffer(struct buffer *buffer, bool want_filled);
void give_buffer(struct buffer *buffer, bool filled);
