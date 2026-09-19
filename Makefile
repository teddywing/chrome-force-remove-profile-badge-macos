SOURCES := main.c
OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))
TARGET := chrome-force-remove-profile-badge-macos

LDFLAGS := -framework CoreServices

all: $(TARGET)

$(OBJECTS): $(SOURCES)

$(TARGET): $(OBJECTS)
	$(CC) \
		-o $@ \
		$(CFLAGS) \
		$^ \
		$(LDFLAGS)
