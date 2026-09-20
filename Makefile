# Copyright (c) 2026  Teddy Wing
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.


prefix ?= /usr/local
exec_prefix ?= $(prefix)
bindir ?= $(exec_prefix)/bin
datarootdir ?= $(prefix)/share
mandir ?= $(datarootdir)/man
man1dir ?= $(mandir)/man1


SOURCES := main.c
OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))
TARGET := chrome-force-remove-profile-badge-macos

CFLAGS += -Wall -Werror
LDFLAGS += -framework CoreServices

MAN_PAGE := chrome-force-remove-profile-badge-macos.1

all: $(TARGET)

$(OBJECTS): $(SOURCES)

$(TARGET): $(OBJECTS)
	$(CC) \
		-o $@ \
		$(CFLAGS) \
		$^ \
		$(LDFLAGS)

.PHONY: install
install: $(TARGET) $(MAN_PAGE)
	install -d $(DESTDIR)$(bindir)
	install -m 755 $(TARGET) $(DESTDIR)$(bindir)

	install -d $(DESTDIR)$(man1dir)
	install -m 644 $(MAN_PAGE) $(DESTDIR)$(man1dir)
