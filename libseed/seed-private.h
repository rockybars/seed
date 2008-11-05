/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * seed-private.h
 * Copyright (C) Robert Carr 2008 <carrr@rpi.edu>
 *
 * libseed is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libseed is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SEED_PRIVATE_H
#define _SEED_PRIVATE_H

#include <JavaScriptCore/JavaScript.h>
#include <glib.h>
#include <glib-object.h>
#include <stdlib.h>
#include <stdio.h>
#include <girepository.h>
#include <ffi.h>

typedef struct _SeedEngine SeedEngine;
typedef JSValueRef SeedValue;

struct _SeedEngine {
	JSGlobalContextRef context;
	JSObjectRef global;
};

#include "seed-engine.h"
#include "seed-types.h"
#include "seed-signals.h"
#include "seed-builtins.h"
#include "seed-structs.h"


#endif
