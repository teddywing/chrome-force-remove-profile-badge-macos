#include <stdio.h>
#include <sys/stat.h>
#include <pwd.h>

static const char *_PATH_CONSOLE = "/dev/console";

// Source - https://stackoverflow.com/a/40469354
// Posted by Graham Miln
// Retrieved 2026-09-29, License - CC BY-SA 3.0

int main() {
struct stat info;
if (lstat(_PATH_CONSOLE,&info) == 0) {
    printf("%s is owned by %d\n",_PATH_CONSOLE,info.st_uid);

	struct passwd *p = getpwuid(info.st_uid);
    printf("%s is owned by %s\n",_PATH_CONSOLE,p->pw_name);
}
}
