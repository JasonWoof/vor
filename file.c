/* Variations on RockDodger
 * Copyright (C) 2004 Joshua Grams <josh@qualdan.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "config.h"
#include "file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

char *g_data_dir;
char *g_score_file;
mode_t g_score_mode;

char *
load_file(char *filename)
{
	static char r[MAX_PATH_LEN];
	snprintf(r, MAX_PATH_LEN, "%s/%s", g_data_dir, filename);
	return r;
}

int
is_dir(char *dirname)
{
	struct stat buf;
	stat(dirname, &buf);
	return S_ISDIR(buf.st_mode);
}

int
is_file(char *filename)
{
	struct stat buf;
	stat(filename, &buf);
	return S_ISREG(buf.st_mode);
}

int
find_data_dir(void)
{
	int i;
	char *data_options[3] = {
		"./data",
		getenv("VOR_DATA"),
		DATA_PREFIX
	};

	for(i=0; i<3; i++) {
		g_data_dir = strdup(data_options[i]);
		if(is_dir(g_data_dir)) return true;
	}

	fprintf(stderr, "Can't find VoR data! Tried:\n");
	for(i=0; i<3; i++) {
		fprintf(stderr, "\t%s\n", data_options[i]);
	}
	return false;
}

int
find_score_file(void)
{
	g_score_file = load_file("scores");
	g_score_mode = 0111;
	if(is_file(g_score_file)) return true;

	g_score_file = malloc(MAX_PATH_LEN);
	snprintf(g_score_file, MAX_PATH_LEN,
			"%s/.vor-high", getenv("HOME"));
	g_score_mode = 0177;
	if(is_file(g_score_file)) return true;

	return false;
}

int
find_files(void)
{
	int r;
	r = find_data_dir();
	find_score_file();
	return r;
}

FILE *
open_score_file(char *mode)
{
	mode_t old_mask;
	FILE *f = NULL;

	if(!g_score_file) return f;

	old_mask = umask(g_score_mode);
	f = fopen(g_score_file, mode);

	umask(old_mask);
	return f;
}