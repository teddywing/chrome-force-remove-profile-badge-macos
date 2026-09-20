SOURCES := main.c
OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))
TARGET := chrome-force-remove-profile-badge-macos

CFLAGS += -Wall -Werror
LDFLAGS += -framework CoreServices

all: $(TARGET)

$(OBJECTS): $(SOURCES)

$(TARGET): $(OBJECTS)
	$(CC) \
		-o $@ \
		$(CFLAGS) \
		$^ \
		$(LDFLAGS)
