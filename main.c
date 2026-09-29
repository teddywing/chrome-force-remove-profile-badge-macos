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


#include <CoreServices/CoreServices.h>
#include <sys/stat.h>
#include <sysexits.h>
#include <unistd.h>

static const char *kVersion = "0.0.1";

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
/* error: cannot open file /Library/Managed Preferences/root/com.google.Chrome.plist */
// TODO: Fix Managed Preferences path has "root" user name instead of current user name when running from "/Library/LaunchDaemons/".
		exit(EX_IOERR);
	}

	fprintf(f, "%s", kChromePolicyPlistContents);

	fclose(f);
}

// If the "com.google.Chrome.plist" Managed Preferences file is removed, write
// it again.
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

// Get the device that contains `path` for use in
// FSEventStreamCreateRelativeToDevice.
dev_t device_for_path(const char *path) {
	struct stat st;
	int err = lstat(path, &st);
	if (err != noErr) {
		fprintf(stderr, "error: cannot stat '%s'\n", path);
		exit(EX_IOERR);
	}

	return st.st_dev;
}

// Write the Chrome policy plist on launch, and start an FSEventStream to
// detect when the file disappears, and write it again at that point.
int main(int argc, const char *argv[]) {
	if (
		argc == 2
		&&
		(
			strncmp(argv[1], "-V", 2) == 0
			|| strncmp(argv[1], "--version", 9) == 0
		)
	) {
		puts(kVersion);
		return EXIT_SUCCESS;
	}

	// Build the "Managed Preferences" directory path.
	char managed_preferences_path[MAXPATHLEN] = "";
	managed_preferences_user_path(managed_preferences_path, MAXPATHLEN);
	char *managed_preferences_device_relative_path = managed_preferences_path + 1;

	// Build the Chrome policy plist absolute path.
	char policy_path[MAXPATHLEN] = "";
	strncpy(policy_path, managed_preferences_path, sizeof(managed_preferences_path));
	chrome_policy_path(policy_path, MAXPATHLEN);
	managed_preferences_chrome_policy_path = policy_path;
	managed_preferences_chrome_policy_device_relative_path = policy_path + 1;

	// Ensure the Chrome policy plist file is written and present.
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
	CFAbsoluteTime latency = 3.0; // 10.0 TODO

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
