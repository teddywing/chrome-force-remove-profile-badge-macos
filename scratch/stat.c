#include <stdlib.h>
#include <stdio.h>
#include <sys/param.h>
#include <sys/stat.h>

int main() {
	struct stat st;
	dev_t dev = 0;
	char *path = "/tmp/fsevents-test";

	if (lstat(path, &st) == 0) {
		dev = st.st_dev;
	}

	printf("dev: %d\n", dev);

	return EXIT_SUCCESS;
}
