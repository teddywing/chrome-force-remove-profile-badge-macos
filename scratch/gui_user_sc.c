// cc gui_user_sc.c -framework SystemConfiguration -framework CoreFoundation
//
// Reference:
// https://superuser.com/questions/180819/how-can-you-find-out-the-currently-logged-in-user-in-the-os-x-gui/180845#180845

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
