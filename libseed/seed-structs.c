/*
 * -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- 
 */
/*
 * seed-structs.h
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

#include "seed-private.h"
#include <string.h>
JSClassRef seed_struct_class = 0;
JSClassRef seed_union_class = 0;
JSClassRef seed_pointer_class = 0;
JSClassRef seed_boxed_class = 0;

typedef struct _seed_struct_privates {
	gpointer pointer;
	GIBaseInfo *info;
	
	gboolean free_pointer;
} seed_struct_privates;

static void seed_pointer_finalize(JSObjectRef object)
{
	seed_struct_privates *priv =
		(seed_struct_privates *) JSObjectGetPrivate(object);

	if (priv->free_pointer)
		g_free(priv->pointer);

	g_free(priv);
}

static void seed_boxed_finalize(JSObjectRef object)
{
	seed_struct_privates *priv =
		(seed_struct_privates *) JSObjectGetPrivate(object);
	GType type;
	GIRegisteredTypeInfo *info =
		(GIRegisteredTypeInfo *) g_base_info_get_type(priv->info);

	type = g_registered_type_info_get_g_type(info);
	g_base_info_unref((GIBaseInfo *) info);

	g_boxed_free(type, priv->pointer);

}

static GIFieldInfo *seed_union_find_field(GIUnionInfo * info,
										  gchar * field_name)
{
	int n, i;
	GIFieldInfo *field;

	n = g_union_info_get_n_fields(info);
	for (i = 0; i < n; i++)
	{
		const gchar *name;

		field = g_union_info_get_field(info, i);
		name = g_base_info_get_name((GIBaseInfo *) field);
		if (!strcmp(name, field_name))
			return field;
		else
			g_base_info_unref((GIBaseInfo *) field);
	}

	return 0;
}

static GIFieldInfo *seed_struct_find_field(GIStructInfo * info,
										   gchar * field_name)
{
	int n, i;
	GIFieldInfo *field;

	n = g_struct_info_get_n_fields(info);
	for (i = 0; i < n; i++)
	{
		const gchar *name;

		field = g_struct_info_get_field(info, i);
		name = g_base_info_get_name((GIBaseInfo *) field);
		if (!strcmp(name, field_name))
			return field;
		else
			g_base_info_unref((GIBaseInfo *) field);
	}

	return 0;
}

JSValueRef
seed_field_get_value(gpointer object,
					 GIFieldInfo * field,
	                 JSValueRef * exception)
{
	GITypeInfo *field_type;
	GArgument field_value;
	JSValueRef ret;

	field_type = g_field_info_get_type(field);
	if (!g_field_info_get_field(field, object, &field_value))
	{
		GITypeTag tag;

		tag = g_type_info_get_tag(field_type);
		if (tag == GI_TYPE_TAG_INTERFACE)
		{
			GIBaseInfo *interface;

			interface = g_type_info_get_interface(field_type);
			gint offset = g_field_info_get_offset(field);
			switch (g_base_info_get_type(interface))
			{
			case GI_INFO_TYPE_STRUCT:
				return seed_make_struct((object + offset), interface);

			case GI_INFO_TYPE_UNION:
				return seed_make_union((object + offset), interface);

			case GI_INFO_TYPE_BOXED:
				return seed_make_boxed((object + offset), interface);

			default:
				g_base_info_unref(interface);
			}
		}

		return JSValueMakeNull(eng->context);
	}

	// Maybe need to release argument.
	ret = seed_gi_argument_make_js(&field_value, field_type, exception);
	if (field_type)
		g_base_info_unref((GIBaseInfo *) field_type);
	return ret;
}


static JSValueRef
seed_union_get_property(JSContextRef context,
						JSObjectRef object,
						JSStringRef property_name, JSValueRef * exception)
{
	gpointer pointer;
	gchar *cproperty_name;
	int i;
	int length;
	seed_struct_privates *priv = JSObjectGetPrivate(object);
	GIFieldInfo *field = 0;
	GITypeInfo *field_type = 0;
	GArgument field_value;
	JSValueRef ret;

	length = JSStringGetMaximumUTF8CStringSize(property_name);
	cproperty_name = g_malloc(length * sizeof(gchar));
	JSStringGetUTF8CString(property_name, cproperty_name, length);

	field = seed_union_find_field((GIUnionInfo *) priv->info, cproperty_name);
	if (!field)
	{
		g_free(cproperty_name);
		return 0;
	}

	ret = seed_field_get_value(priv->pointer, field, exception);

	g_base_info_unref((GIBaseInfo *) field);

	g_free(cproperty_name);

	return ret;
}

static bool
seed_struct_set_property(JSContextRef context,
						  JSObjectRef object,
						  JSStringRef property_name,
						  JSValueRef value, JSValueRef * exception)
{
	gint length, i, n;
	GArgument field_value;
	GIFieldInfo * field;
	GITypeInfo * field_type;
	gchar * cproperty_name;
	seed_struct_privates *priv = (seed_struct_privates *)JSObjectGetPrivate(object);
	gboolean ret;

	length = JSStringGetMaximumUTF8CStringSize(property_name);
	cproperty_name = g_malloc(length * sizeof(gchar));
	JSStringGetUTF8CString(property_name, cproperty_name, length);

	field = seed_struct_find_field((GIStructInfo *) priv->info, cproperty_name);

	if (!field)
	{
		g_free(cproperty_name);
		return 0;
	}
	
	field_type = g_field_info_get_type(field);

	seed_gi_make_argument(value, field_type, &field_value, exception);
	ret = g_field_info_set_field(field, priv->pointer, &field_value);
	
	g_free(cproperty_name);
	g_base_info_unref((GIBaseInfo *) field_type);
	g_base_info_unref((GIBaseInfo *) field);
}

static JSValueRef
seed_struct_get_property(JSContextRef context,
						 JSObjectRef object,
						 JSStringRef property_name, JSValueRef * exception)
{
	gpointer pointer;
	gchar *cproperty_name;
	int i, n;
	int length;
	seed_struct_privates *priv = JSObjectGetPrivate(object);
	GIFieldInfo *field = 0;
	GITypeInfo *field_type = 0;
	GArgument field_value;
	JSValueRef ret;

	length = JSStringGetMaximumUTF8CStringSize(property_name);
	cproperty_name = g_malloc(length * sizeof(gchar));
	JSStringGetUTF8CString(property_name, cproperty_name, length);

	field = seed_struct_find_field((GIStructInfo *) priv->info, cproperty_name);

	if (!field)
	{
		g_free(cproperty_name);
		return 0;
	}

	ret = seed_field_get_value(priv->pointer, field, exception);

	g_base_info_unref((GIBaseInfo *) field);
	g_free(cproperty_name);

	return ret;
}

JSClassDefinition seed_pointer_def = {
	0,							/* Version, always 0 */
	0,
	"seed_pointer",				/* Class Name */
	NULL,						/* Parent Class */
	NULL,						/* Static Values */
	NULL,						/* Static Functions */
	NULL,
	seed_pointer_finalize,
	NULL,						/* Has Property */
	0,
	NULL,						/* Set Property */
	NULL,						/* Delete Property */
	NULL,						/* Get Property Names */
	NULL,						/* Call As Function */
	NULL,						/* Call As Constructor */
	NULL,						/* Has Instance */
	NULL						/* Convert To Type */
};

JSClassDefinition seed_struct_def = {
	0,							/* Version, always 0 */
	0,
	"seed_struct",				/* Class Name */
	NULL,						/* Parent Class */
	NULL,						/* Static Values */
	NULL,						/* Static Functions */
	NULL,
	NULL,
	NULL,						/* Has Property */
	seed_struct_get_property,
	seed_struct_set_property,						/* Set Property */
	NULL,						/* Delete Property */
	NULL,						/* Get Property Names */
	NULL,						/* Call As Function */
	NULL,						/* Call As Constructor */
	NULL,						/* Has Instance */
	NULL						/* Convert To Type */
};

JSClassDefinition seed_union_def = {
	0,							/* Version, always 0 */
	0,
	"seed_union",				/* Class Name */
	NULL,						/* Parent Class */
	NULL,						/* Static Values */
	NULL,						/* Static Functions */
	NULL,
	NULL,
	NULL,						/* Has Property */
	seed_union_get_property,
	NULL,						/* Set Property */
	NULL,						/* Delete Property */
	NULL,						/* Get Property Names */
	NULL,						/* Call As Function */
	NULL,						/* Call As Constructor */
	NULL,						/* Has Instance */
	NULL						/* Convert To Type */
};

JSClassDefinition seed_boxed_def = {
	0,							/* Version, always 0 */
	0,
	"seed_boxed",				/* Class Name */
	NULL,						/* Parent Class */
	NULL,						/* Static Values */
	NULL,						/* Static Functions */
	NULL,
	seed_boxed_finalize, 
	NULL,						/* Has Property */
	NULL,
	NULL,						/* Set Property */
	NULL,						/* Delete Property */
	NULL,						/* Get Property Names */
	NULL,						/* Call As Function */
	NULL,						/* Call As Constructor */
	NULL,						/* Has Instance */
	NULL						/* Convert To Type */
};

gpointer seed_pointer_get_pointer(JSValueRef pointer)
{
	if (JSValueIsObjectOfClass(eng->context, pointer, seed_pointer_class))
	{
		seed_struct_privates *priv = JSObjectGetPrivate((JSObjectRef) pointer);
		return priv->pointer;
	}
	return 0;
}

void seed_pointer_set_free(JSValueRef pointer, gboolean free_pointer)
{
	if (JSValueIsObjectOfClass(eng->context, pointer, seed_pointer_class))
	{
		seed_struct_privates *priv = JSObjectGetPrivate((JSObjectRef) pointer);
		priv->free_pointer = free_pointer;
	}
}

JSObjectRef seed_make_pointer(gpointer pointer)
{
	seed_struct_privates *priv = g_malloc(sizeof(seed_struct_privates));
	priv->pointer = pointer;
	priv->info = 0;
	priv->free_pointer = FALSE;

	return JSObjectMake(eng->context, seed_pointer_class, priv);
}

JSObjectRef seed_make_union(gpointer younion, GIBaseInfo * info)
{
	JSObjectRef object;
	gint i, n_methods;
	seed_struct_privates *priv = g_malloc(sizeof(seed_struct_privates));

	priv->pointer = younion;
	priv->info = info;
	priv->free_pointer = FALSE;

	object = JSObjectMake(eng->context, seed_union_class, priv);

	if (info)
	{
		n_methods = g_union_info_get_n_methods((GIUnionInfo *) info);
		for (i = 0; i < n_methods; i++)
		{
			GIFunctionInfo *finfo;

			finfo = g_union_info_get_method((GIUnionInfo *) info, i);

			seed_gobject_define_property_from_function_info((GIFunctionInfo *)
															finfo, object,
															TRUE);
		}
	}

	return object;
}

JSObjectRef seed_make_boxed(gpointer boxed, GIBaseInfo * info)
{
	JSObjectRef object;
	gint i, n_methods;
	seed_struct_privates *priv = g_malloc(sizeof(seed_struct_privates));

	priv->info = info;
	priv->pointer = boxed;
	// Boxed finalize handler handles freeing.
	priv->free_pointer = FALSE;

	object = JSObjectMake(eng->context, seed_boxed_class, priv);

	// FIXME: Instance methods?

	return object;
}

JSObjectRef seed_make_struct(gpointer strukt, GIBaseInfo * info)
{
	JSObjectRef object;
	gint i, n_methods;
	seed_struct_privates *priv = g_malloc(sizeof(seed_struct_privates));

	priv->info = info;
	priv->pointer = strukt;
	priv->free_pointer = FALSE;

	object = JSObjectMake(eng->context, seed_struct_class, priv);

	if (info)
	{
		n_methods = g_struct_info_get_n_methods((GIStructInfo *) info);
		for (i = 0; i < n_methods; i++)
		{
			GIFunctionInfo *finfo;

			finfo = g_struct_info_get_method((GIStructInfo *) info, i);

			seed_gobject_define_property_from_function_info((GIFunctionInfo *)
															finfo, object,
															TRUE);
		}
	}

	return object;
}

void seed_structs_init(void)
{
	seed_pointer_class = JSClassCreate(&seed_pointer_def);
	seed_struct_def.parentClass = seed_pointer_class;
	seed_struct_class = JSClassCreate(&seed_struct_def);
	seed_union_def.parentClass = seed_union_class;
	seed_union_class = JSClassCreate(&seed_union_def);
	seed_boxed_def.parentClass = seed_struct_class;
	seed_boxed_class = JSClassCreate(&seed_boxed_def);
}
