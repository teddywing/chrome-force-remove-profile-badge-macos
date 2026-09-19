#include <CoreServices/CoreServices.h>
#include <sys/stat.h>
#include <sysexits.h>

static const char *kChromePolicyPath = "private/tmp/fsevents-test/com.google.Chrome.plist";

void fsevents_callback(
	ConstFSEventStreamRef stream_ref,
	void *client_call_back_info,
	size_t num_events,
	void *event_paths,
	const FSEventStreamEventFlags event_flags[],
	const FSEventStreamEventId event_ids[]
) {
/* kFSEventStreamEventFlagItemRemoved */

	char **paths = event_paths;

	printf("Callback called\n");
	for (int i = 0; i < num_events; i++) {
		/* flags are unsigned long, IDs are uint64_t */
		printf("Change %llu in %s, flags %u\n", event_ids[i], paths[i], event_flags[i]);

		if (
			strncmp(paths[i], kChromePolicyPath, strlen(kChromePolicyPath)) == 0
			&& event_flags[i] & kFSEventStreamEventFlagItemRemoved
		) {
			printf("removed policy file\n");
		}
	}

	fflush(stdout);
}

dev_t device_for_path(const char *path) {
	struct stat st;
	int err = lstat(path, &st);
	if (err != noErr) {
		printf("TODO exit '%s': %d ; %d\n", path, err, st.st_dev);
	}

	return st.st_dev;
}

int main() {
	FSEventStreamContext context = {
		.version = 0,
		.info = NULL,
		.retain = NULL,
		.release = NULL,
		.copyDescription = NULL
	};
	// NOTE: Cannot use "/tmp/fsevents-test" as "/tmp" is a symlink.
	CFStringRef path = CFSTR("private/tmp/fsevents-test");
	CFArrayRef paths_to_watch = CFArrayCreate(NULL, (const void **)&path, 1, NULL);
	dev_t device_id = device_for_path("/private/tmp/fsevents-test");
	CFAbsoluteTime latency = 3.0; // 10.0
 
	FSEventStreamRef stream = FSEventStreamCreateRelativeToDevice(
		kCFAllocatorDefault,
		&fsevents_callback,
		&context,
		device_id,
		paths_to_watch,
		kFSEventStreamEventIdSinceNow,
		latency,
		kFSEventStreamCreateFlagFileEvents
		/* kFSEventStreamCreateFlagNone */ /* kFSEventStreamCreateFlagUseCFTypes */
	);

	CFRelease(paths_to_watch);

    /* FSEventStreamScheduleWithRunLoop( */
	/* 	stream, */
	/* 	CFRunLoopGetCurrent(), */
	/* 	kCFRunLoopDefaultMode */
    /* ); */
	FSEventStreamSetDispatchQueue(stream, dispatch_get_main_queue());

	Boolean is_started = FSEventStreamStart(stream);
	if (!is_started) {
		printf("error: FSEvents stream failed to start\n");
		return EX_UNAVAILABLE;
	}

	CFRunLoopRun();

	FSEventStreamStop(stream);
	FSEventStreamInvalidate(stream);
	FSEventStreamRelease(stream);

	return EXIT_SUCCESS;
}
