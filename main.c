#include <CoreServices/CoreServices.h>
#include <sys/stat.h>
#include <sysexits.h>

static const char *kChromePolicyPath = "private/tmp/fsevents-test/com.google.Chrome.plist";
static const char *kChromePolicyPlistContents = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n\
<plist version=\"1.0\">\n\
<dict>\n\
	<key>EnterpriseProfileBadgeToolbarSettings</key>\n\
	<integer>1</integer>\n\
</dict>\n\
</plist>\n\
";

void write_policy_file() {
	FILE *f = fopen("/private/tmp/fsevents-test/com.google.Chrome.plist", "w");
	if (f == NULL) {
		fprintf(stderr, "error: cannot open file %s\n", kChromePolicyPath);
		exit(EX_IOERR);
	}

	fprintf(f, "%s", kChromePolicyPlistContents);

	fclose(f);
}

void fsevents_callback(
	ConstFSEventStreamRef stream_ref,
	void *client_call_back_info,
	size_t num_events,
	void *event_paths,
	const FSEventStreamEventFlags event_flags[],
	const FSEventStreamEventId event_ids[]
) {
	char **paths = event_paths;

	for (int i = 0; i < num_events; i++) {
		if (
			strncmp(paths[i], kChromePolicyPath, strlen(kChromePolicyPath)) == 0
			&& event_flags[i] & kFSEventStreamEventFlagItemRemoved
		) {
			write_policy_file();
		}
	}
}

dev_t device_for_path(const char *path) {
	struct stat st;
	int err = lstat(path, &st);
	if (err != noErr) {
		fprintf(stderr, "error: cannot stat '%s'\n", path);
		exit(EX_IOERR);
	}

	return st.st_dev;
}

int main() {
	write_policy_file();

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
		kFSEventStreamCreateFlagFileEvents | kFSEventStreamCreateFlagIgnoreSelf
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
		fprintf(stderr, "error: FSEvents stream failed to start\n");
		return EX_UNAVAILABLE;
	}

	CFRunLoopRun();

	FSEventStreamStop(stream);
	FSEventStreamInvalidate(stream);
	FSEventStreamRelease(stream);

	return EXIT_SUCCESS;
}
