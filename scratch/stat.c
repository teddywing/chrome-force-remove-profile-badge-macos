// Copyright (c) 2026  Teddy Wing
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.


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
