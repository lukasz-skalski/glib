/* GIO - GLib Input, Output and Streaming Library
 *
 * Copyright (C) 2015 Samsung Electronics
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
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __G_KDBUS_COMPATIBILITY_H__
#define __G_KDBUS_COMPATIBILITY_H__

#include "gkdbus.h"

G_BEGIN_DECLS

gboolean    emulate_dbus_daemon     (GDBusMessage  *message);
gboolean    prepare_synthetic_reply (GKDBusWorker  *worker,
                                     GDBusMessage  *message);

G_END_DECLS

#endif /* __G_KDBUS_COMPATIBILITY__ */
