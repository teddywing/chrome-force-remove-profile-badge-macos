// cc gui_user_sc.c -framework SystemConfiguration -framework CoreFoundation
//
// Reference:
// https://superuser.com/questions/180819/how-can-you-find-out-the-currently-logged-in-user-in-the-os-x-gui/180845#180845

#include <SystemConfiguration/SystemConfiguration.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <pwd.h>

int main() {
	SCDynamicStoreRef store = SCDynamicStoreCreate(NULL, CFSTR("gui_user_sc"), NULL, NULL);

	uid_t uid;
	SCDynamicStoreCopyConsoleUser(store, &uid, NULL);

	CFRelease(store);

	printf("uid: %d\n", uid);

	struct passwd *p = getpwuid(uid);
	printf("username: %s\n", p->pw_name);
}
