#include <dirent.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/input.h>

#define DEV_PATH "/dev/input/"

static struct pollfd *ufds;
static int nfds;
static int *dev_ids;

int filter(const struct dirent *ent)
{
	return ent->d_name[0] == '.';
}

int compar(const struct dirent **ent1, const struct dirent **ent2)
{
	return strcmp((*ent1)->d_name, (*ent2)->d_name);
}

int add_dev(int fd, int id)
{
	struct pollfd *new_ufds;
	int *new_dev_ids;

	new_ufds = realloc(ufds, sizeof(struct pollfd) * (nfds + 1));
	new_dev_ids = realloc(dev_ids, sizeof(int) * (nfds + 1));

	if (new_dev_ids) {
		dev_ids = new_dev_ids;
		dev_ids[nfds] = id;
	} else {
		fprintf(stderr, "mem alloc error\n");
		return -1;
	}

	if (new_ufds) {
		ufds = new_ufds;
		ufds[nfds].fd = fd;
		ufds[nfds].events = POLLIN;
	} else {
		fprintf(stderr, "mem alloc error\n");
		return -1;
	}

	nfds++;
	return nfds;
}

int open_dev(int id)
{
	int fd;
	char device_path[PATH_MAX];

	sprintf(device_path, "/dev/input/event%d", id);

	fd = open(device_path, O_RDONLY);

	if (fd < 0) {
		perror("open_dev");
		return -1;
	}

	return add_dev(fd, id);
}

int close_devs()
{
	while (nfds) {
		nfds--;
		if (close(ufds[nfds].fd) < 0) {
			perror("close_devs");
			return -1;
		}
	}

	return 0;
}

int record(char *rec_file)
{
	int out_fd;
	int poll_res;
	int i;
	int res;
	int err = 0;
	struct input_event i_event;

	out_fd = open(rec_file, O_CREAT | O_WRONLY);
	if (out_fd < 0) {
		perror("record:open");
		return -1;
	}

	res = write(out_fd, &nfds, sizeof(nfds));
	if (res != sizeof(nfds)) {
		fprintf(stderr, "Could not write header (nfds)\n");
		return -1;
	}

	for (res = 0, i = 0; i < nfds; i++)
		res += write(out_fd, &dev_ids[i], sizeof(dev_ids[i]));

	if (res != (nfds * sizeof(dev_ids[0]))) {
		fprintf(stderr, "Could not write header (ufds)\n");
		return -1;
	}

	while (1) {
		poll_res = poll(ufds, nfds, -1);
		if (poll_res < 0) {
			perror("record:poll");
			return -1;
		}

		for (i = 0; i < nfds; i++) {
			if (ufds[i].revents & POLLIN) {
				res = read(ufds[i].fd, &i_event, sizeof(i_event));
				if (res != sizeof(i_event)) {
					printf("Input event cannot be read\n");
					return -1;
				}

				res = write(out_fd, &dev_ids[i], sizeof(dev_ids[i]));
				if (res != sizeof(dev_ids[i]))
					err |= 1;

				res = write(out_fd, &i_event, sizeof(i_event));
				if (res != sizeof(i_event))
					err |= 1;

				if (err) {
					printf("Event info could not be written to file\n");
					return -1;
				}
			}
		}
	}
}


int main(int argc, char **argv)
{
/*	char *path;
	DIR *dir;
	int n;
	struct dirent *ent;
	struct dirent **sorted_ents;

	if (argc == 2)
		path = argv[1];
	else
		path = "/home/dirtybit";

	# dir = opendir(path);
	n = scandir(path, &sorted_ents, filter, compar);

	while (n--) {
		printf("[%d] %s\n", (*sorted_ents)->d_type,
		       (*sorted_ents)->d_name);
		sorted_ents++;
	}
*/
	char *rec_file = "/data/native/rec";
	char *dev1 = "event1";
	char *dev2 = "event2";
	char *dev3 = "event6";
	char dev1_path[PATH_MAX];
	char dev2_path[PATH_MAX];
	char dev3_path[PATH_MAX];
	char *path;

	strcpy(dev1_path, DEV_PATH);
	path = dev1_path + strlen(DEV_PATH);
	strcpy(path, dev1);

	strcpy(dev2_path, DEV_PATH);
	path = dev2_path + strlen(DEV_PATH);
	strcpy(path, dev2);

	strcpy(dev3_path, DEV_PATH);
	path = dev3_path + strlen(DEV_PATH);
	strcpy(path, dev3);

	printf("dev1: %s\ndev2: %s\ndev3: %s\n", dev1_path, dev2_path, dev3_path);

	if (open_dev(1) < 0)
		printf("Could not open device: %s\n", dev1_path);
	if (open_dev(2) < 0)
		printf("Could not open device: %s\n", dev2_path);
	if (open_dev(3) < 0)
		printf("Could not open device: %s\n", dev3_path);

	if (record(rec_file) < 0)
		printf("Error on recording\n");

	if (close_devs() < 0)
		printf("Could not close devices\n");

	exit(0);
}
