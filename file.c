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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "config.h"
#include "file.h"

char *g_data_dir;
char *g_score_file;
mode_t g_score_mask;

char *
add_path(char *filename)
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
		if(!data_options[i]) continue;
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
	g_score_file = malloc(MAX_PATH_LEN);
	snprintf(g_score_file, MAX_PATH_LEN,
			"%s/.vor-scores", getenv("HOME"));
	g_score_mask = 0177;
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

	old_mask = umask(g_score_mask);
	f = fopen(g_score_file, mode);

	umask(old_mask);
	return f;
}
