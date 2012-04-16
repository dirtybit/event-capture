#include <fcntl.h>
#include <limits.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/input.h>

#define DEV_PATH "/dev/input/event"

#ifndef N_DEV
#define N_DEV 15
#endif

static int rec_fd;
static int nfds;
static int *ufds;

void timersleep(struct timeval time)
{
	if (time.tv_sec)
		while (sleep(time.tv_sec))
			;

	if (time.tv_usec)
		usleep(time.tv_usec);
}

int init()
{
	int res;
	int dev_id;
	int i;
	char dev_path[PATH_MAX];

	ufds = (int *) malloc(sizeof(int) * N_DEV);

	res = read(rec_fd, &nfds, sizeof(nfds));
	if (res != sizeof(int)) {
		fprintf(stderr, "Could not read header\n");
		return -1;
	}

	for (i = 0; i < nfds; i++) {
		read(rec_fd, &dev_id, sizeof(int));
		sprintf(dev_path, "%s%d", DEV_PATH, dev_id);
		res = open(dev_path, O_WRONLY);
		if (res < 0) {
			perror("Could not open input device");
			return -1;
		}
		ufds[dev_id] = res;
		printf("%d - %d\n", dev_id, ufds[dev_id]);
	}

	return 0;
}

void print_event(int id, struct input_event ev)
{
	printf("event%d %04x %04x %08x\n", id, ev.type, ev.code, ev.value);
}

int replay()
{
	int dev_id;
	int res;
	struct timeval t;
	struct timeval t_1;
	struct timeval t_diff;
	struct input_event event;

	timerclear(&t_1);
	timerclear(&t);

	res = init();
	if (res < 0) {
		fprintf(stderr, "Could not initialize input devices\n");
		return -1;
	}

	while (res = read(rec_fd, &dev_id, sizeof(dev_id))) {
		if (res < 0 || res != sizeof(dev_id)) {
			fprintf(stderr, "Recorded event could not be read (dev_id)\n");
			return -1;
		}

		res = read(rec_fd, &event, sizeof(event));
		if (res < 0 || res != sizeof(event)) {
			fprintf(stderr, "Recorded event could not be read (event)\n");
			return -1;
		}

		/* Wait as needed */
		t = event.time;
		if (timerisset(&t_1)) {
			timersub(&t, &t_1, &t_diff);
			timersleep(t_diff);
		}

		res = write(ufds[dev_id], &event, sizeof(struct input_event));

		/* print_event(dev_id, event); */
		if (res < 0) {
			perror("replay_write");
			return -1;
		} else if (res != sizeof(struct input_event)) {
			fprintf(stderr, "Recorded event could not be sent to input device\n");
			return -1;
		}
		t_1 = t;
	}

	return 0;
}

void usage()
{
	printf("Usage:\n");
	printf("\treplay [record_file]\n");
	exit(0);
}

int main(int argc, char **argv)
{
	int res;
	char *rec_file = "/data/native/recfile";

	if (argc == 2)
		if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))
			usage();
		else
			rec_file = argv[1];

	rec_fd = open(rec_file, O_RDONLY);

	if (rec_fd < 0) {
		perror("Could open record file");
		exit(1);
	}

	res = replay();
	if (res < 0) {
		fprintf(stderr, "Replay failed!\n");
		exit(1);
	}

	exit(0);
	return 0;
}
