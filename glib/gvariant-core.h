/*
 * Copyright © 2007, 2008 Ryan Lortie
 * Copyright © 2010 Codethink Limited
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __G_VARIANT_CORE_H__
#define __G_VARIANT_CORE_H__

#include <glib/gvarianttypeinfo.h>
#include <glib/gvariant.h>
#include <glib/gbytes.h>

/* gvariant-core.c */

GVariant *              g_variant_new_from_children                     (GVariantTypeInfo    *type_info,
                                                                         GVariant           **children,
                                                                         gsize                n_children,
                                                                         gboolean             trusted);

GVariant *              g_variant_new_serialised                        (GVariantTypeInfo    *type_info,
                                                                         GBytes              *bytes,
                                                                         gconstpointer        data,
                                                                         gsize                size,
                                                                         gboolean             trusted);

gboolean                g_variant_is_trusted                            (GVariant            *value);

GVariantTypeInfo *      g_variant_get_type_info                         (GVariant            *value);

gconstpointer           g_variant_get_serialised                        (GVariant            *value,
                                                                         GBytes             **bytes,
                                                                         gsize               *size);

#endif /* __G_VARIANT_CORE_H__ */
