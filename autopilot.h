//  Copyright (C) 2007 Jason Woofenden
//
//  This file is part of VoR.
//
//  VoR is free software; you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2, or (at your option)
//  any later version.
//
//  VoR is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with VoR; see the file COPYING.  If not, write to the
//  Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
//  MA 02111-1307, USA.


void autopilot_init();
int autopilot(float ticks);
void autopilot_fix_keystates(Uint8* table);
