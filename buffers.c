#include "buffers.h"

void init_buffer(struct buffer *buffer)
{
	pthread_mutex_init(&buffer->mx, NULL);
	pthread_cond_init(&buffer->cv, NULL);
}

void take_buffer(struct buffer *buffer, bool want_filled)
{
	pthread_mutex_lock(&buffer->mx);
	while (buffer->filled != want_filled) {
		pthread_cond_wait(&buffer->cv, &buffer->mx);
	}
}

void give_buffer(struct buffer *buffer, bool filled)
{
	buffer->filled = filled;
	pthread_cond_signal(&buffer->cv);
	pthread_mutex_unlock(&buffer->mx);
}

