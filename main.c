#include <CoreServices/CoreServices.h>
#include <sys/stat.h>
#include <sysexits.h>
#include <unistd.h>

static const char *kManagedPreferencesPath = "/Library/Managed Preferences";
static const char *kChromePolicyFilename = "com.google.Chrome.plist";
static const char *kChromePolicyPlistContents = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n\
<plist version=\"1.0\">\n\
<dict>\n\
	<key>EnterpriseProfileBadgeToolbarSettings</key>\n\
	<integer>1</integer>\n\
</dict>\n\
</plist>\n\
";

// Example: "/Library/Managed Preferences/<username>/com.google.Chrome.plist"
static char *managed_preferences_chrome_policy_path;

// Example: "Library/Managed Preferences/<username>/com.google.Chrome.plist"
static char *managed_preferences_chrome_policy_device_relative_path;

// Build the string "/Library/Managed Preferences/<username>" in `path`.
void managed_preferences_user_path(char *path, size_t path_size) {
	char *username = getlogin();

	size_t length;

	length = strlcat(path, kManagedPreferencesPath, path_size);
	if (length >= path_size) {
		fprintf(
			stderr,
			"error: error initialising Managed Preferences path '%s' (len=%zu dstsize=%zu)'\n",
			kManagedPreferencesPath,
			length,
			path_size
		);
		exit(EX_SOFTWARE);
	}

	length = strlcat(path, "/", path_size);
	if (length >= path_size) {
		fprintf(
			stderr,
			"error: error building Managed Preferences path '%s' '%s' (len=%zu dstsize=%zu)'\n",
			kManagedPreferencesPath,
			"/",
			length,
			path_size
		);
		exit(EX_SOFTWARE);
	}

	length = strlcat(path, username, path_size);
	if (length >= path_size) {
		fprintf(
			stderr,
			"error: error building Managed Preferences path '%s' '%s' (len=%zu dstsize=%zu)'\n",
			kManagedPreferencesPath,
			username,
			length,
			path_size
		);
		exit(EX_SOFTWARE);
	}
}

// Append "/com.google.Chrome.plist" to `path`.
void chrome_policy_path(char *path, size_t path_size) {
	size_t length;

	length = strlcat(path, "/", path_size);
	if (length >= path_size) {
		fprintf(
			stderr,
			"error: error building Managed Preferences path '%s' '%s' (len=%zu dstsize=%zu)'\n",
			kManagedPreferencesPath,
			"/",
			length,
			path_size
		);
		exit(EX_SOFTWARE);
	}

	length = strlcat(path, kChromePolicyFilename, path_size);
	if (length >= path_size) {
		fprintf(
			stderr,
			"error: error building Managed Preferences path '%s' '%s' (len=%zu dstsize=%zu)'\n",
			kManagedPreferencesPath,
			kChromePolicyFilename,
			length,
			path_size
		);
		exit(EX_SOFTWARE);
	}
}

void write_policy_file(char *path) {
	FILE *f = fopen(path, "w");
	if (f == NULL) {
		fprintf(stderr, "error: cannot open file %s\n", path);
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
			strncmp(
				paths[i],
				managed_preferences_chrome_policy_device_relative_path,
				strlen(managed_preferences_chrome_policy_device_relative_path)
			) == 0
			&& (event_flags[i] & kFSEventStreamEventFlagItemIsFile)
			&& (
				(event_flags[i] & kFSEventStreamEventFlagItemRemoved)
				|| (event_flags[i] & kFSEventStreamEventFlagItemRenamed)
			)
		) {
			write_policy_file(managed_preferences_chrome_policy_path);
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
	char managed_preferences_path[MAXPATHLEN];
	managed_preferences_user_path(managed_preferences_path, MAXPATHLEN);
	char *managed_preferences_device_relative_path = managed_preferences_path + 1;

	char policy_path[MAXPATHLEN];
	strncpy(policy_path, managed_preferences_path, strlen(managed_preferences_path));
	chrome_policy_path(policy_path, MAXPATHLEN);
	managed_preferences_chrome_policy_path = policy_path;
	managed_preferences_chrome_policy_device_relative_path = policy_path + 1;

	write_policy_file(managed_preferences_chrome_policy_path);

	FSEventStreamContext context = {
		.version = 0,
		.info = NULL,
		.retain = NULL,
		.release = NULL,
		.copyDescription = NULL
	};

	// NOTE: path must not be a symlink.
	CFStringRef path = CFStringCreateWithCString(
		kCFAllocatorDefault,
		managed_preferences_device_relative_path,
		kCFStringEncodingUTF8
	);
	CFArrayRef paths_to_watch = CFArrayCreate(NULL, (const void **)&path, 1, NULL);

	dev_t device_id = device_for_path(managed_preferences_path);
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

	CFRelease(path);
	CFRelease(paths_to_watch);

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
