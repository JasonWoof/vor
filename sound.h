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

#ifndef VOR_SOUND_H
#define VOR_SOUND_H

int init_sound(void);
void play_sound(int i);
void play_tune(int i);
void pause_tune();
void resume_tune();

// Currently there's only one tune (gameplay). Calling play_tune() with the
// other tune numbers turns the music off, (and makes it easy to add tunes for
// those screens).
#define TUNE_GAMEPLAY 0
#define TUNE_TITLE_PAGE 1
#define TUNE_HIGH_SCORE_ENTRY 2
#define NUM_TUNES 1

#define SOUND_BANG 0
#define NUM_SOUNDS 1

#endif // VOR_SOUND_H
