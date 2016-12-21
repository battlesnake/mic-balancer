#if 0
######################
# Run as ./balance.c #
######################

outfile="$(mktemp)"
gcc -std=c11 -O2 -march=native -mtune=native -flto -ffunction-sections -fdata-sections -Wl,--gc-sections -s -lm -lpthread -lpulse -lpulse-simple -o "$outfile" *.c
trap 'rm -f "$outfile"' EXIT ERR
size "$outfile"
"$outfile"
exit 0
#endif
#include <pthread.h>
#include <unistd.h>
#include "safe_io.h"
#include "buffers.h"
#include "workers.h"

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	pthread_t ti;
	pthread_t to;

	for (int i = 0; i < BUFFER_COUNT; i++) {
		init_buffer(&buffers[i]);
	}

	if (pthread_create(&ti, NULL, (void *) thread_record, NULL)) {
		error("Failed to create recording thread");
		goto fail;
	}
	sleep(1);
	if (pthread_create(&to, NULL, (void *) thread_playback, NULL)) {
		error("Failed to create playback thread");
		goto fail;
	}

	pthread_join(ti, NULL);
	pthread_join(to, NULL);

fail:
	return 0;
}
