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

#ifndef VOR_FILE_H
#define VOR_FILE_H

#include <stdio.h>
#include <sys/types.h>

extern char *g_data_dir;
extern char *g_score_file;
extern mode_t g_score_mode;

char *load_file(char *filename);
int is_dir(char *dirname);
int is_file(char *filename);
int find_data_dir(void);
int find_score_file(void);
int find_files(void);
FILE *open_score_file(char *mode);

#endif // VOR_FILE_H
