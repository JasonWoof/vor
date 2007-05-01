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
#ifndef WIN32
# include <sys/stat.h>
#endif
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "config.h"
#include "file.h"

#ifdef WIN32

#define DATA_DIR "data\\"

char *
add_data_path(char *filename)
{
	char *s;
	if(!filename) return filename;
	s = malloc(sizeof(DATA_DIR)+strlen(filename));
	if(s) {
		strcpy(s, DATA_DIR);
		strcpy(s+sizeof(DATA_DIR)-1, filename);
	}
	return s;
}

bool
find_files(void)
{
	return true;
}

FILE *open_score_file(char *mode)
{
	return fopen("scores", mode);
}


#else /* !WIN32 */


char *g_data_dir;
char *g_score_file;

static char *
add_path(char *path, char *file)
{
	char *s;
	size_t plen, flen;

	if(!path || !file) return NULL;
	plen = strlen(path);
	flen = strlen(file);
	s = malloc(2+plen+flen);
	if(!s) return NULL;
	memcpy(s, path, plen);
	s[plen] = '/';
	memcpy(s+plen+1, file, flen+1);
	return s;
}

char *
add_data_path(char *filename)
{
	return add_path(g_data_dir, filename);
}

static bool
is_dir(char *dirname)
{
	struct stat buf;
	stat(dirname, &buf);
	return S_ISDIR(buf.st_mode);
}

static bool
find_data_dir(void)
{
	int i;
	char *data_options[3] = {
		getenv("VOR_DATA"),
		"data",
		DATA_PREFIX
	};

	for(i=0; i<3; i++) {
		if(!data_options[i]) continue;
		g_data_dir = strdup(data_options[i]);
		if(is_dir(g_data_dir)) return true;
	}

	fprintf(stderr, "Can't find VoR data! Tried:\n");
	for(i=0; i<3; i++) {
		if(data_options[i]) fprintf(stderr, "\t%s\n", data_options[i]);
	}
	return false;
}

static bool
find_score_file(void)
{
	char *dir, *s;
	
	/* in case we get called twice */
	if(g_score_file) return true;

	dir = getenv("HOME"); if(!dir) return false;
	s = add_path(dir, ".vor-scores");
	if(s) {
		g_score_file = s;
		return true;
	} else return false;
}

bool
find_files(void)
{
	return find_data_dir() && find_score_file();
}

FILE *
open_score_file(char *mode)
{
	if(!g_score_file) return NULL;
	return fopen(g_score_file, mode);
}

#endif /* !WIN32 */
