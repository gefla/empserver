/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                Ken Stevens, Steve McClure, Markus Armbruster
 *
 *  Empire is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  ---
 *
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  getpass.c: Get a password
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2009-2020
 */

#include <config.h>

#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#endif
#include "misc.h"

static int
set_echo_if_tty(int on)
{
#ifdef _WIN32
    DWORD mode;
    HANDLE input_handle = GetStdHandle(STD_INPUT_HANDLE);

    if (!GetConsoleMode(input_handle, &mode))
	return 0;

    if (on)
	mode |= ENABLE_ECHO_INPUT;
    else
	mode &= ~ENABLE_ECHO_INPUT;

    if (!SetConsoleMode(input_handle, mode))
	return -1;
    return 1;
#else
    struct termios tcattr;

    if (tcgetattr(0, &tcattr) < 0)
       return 0;

    if (on)
       tcattr.c_lflag |= ECHO;
    else
       tcattr.c_lflag &= ~ECHO;

    if (tcsetattr(0, TCSAFLUSH, &tcattr) < 0)
       return -1;
    return 1;
#endif
}

char *
get_password(const char *prompt)
{
    static char buf[128];
    char *p;
    size_t len;
    int echo_set;

    echo_set = set_echo_if_tty(0);
    if (echo_set <= 0)
	printf("Note: your input is echoed to the screen\n");

    printf("%s", prompt);
    fflush(stdout);
    p = fgets(buf, sizeof(buf), stdin);

    if (echo_set > 0)
	set_echo_if_tty(1);

    if (!p)
	return NULL;
    len = strlen(p);
    if (p[len - 1] == '\n')
	p[len - 1] = 0;
    return p;
}
