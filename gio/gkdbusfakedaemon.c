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
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Lukasz Skalski <l.skalski@samsung.com>
 */

#include "config.h"
#include "gkdbus.h"
#include "gkdbusfakedaemon.h"

#include <gio/gio.h>

static gchar *introspect =
  "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\" "
  "\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n"
  "<node>\n"
  " <interface name=\"org.freedesktop.DBus.Introspectable\">\n"
  "  <method name=\"Introspect\">\n"
  "   <arg name=\"data\" type=\"s\" direction=\"out\"/>\n"
  "  </method>\n"
  " </interface>\n"
  " <interface name=\"org.freedesktop.DBus\">\n"
  "  <method name=\"AddMatch\">\n"
  "   <arg type=\"s\" direction=\"in\"/>\n"
  "  </method>\n"
  "  <method name=\"RemoveMatch\">\n"
  "   <arg type=\"s\" direction=\"in\"/>\n"
  "  </method>\n"
  "  <method name=\"GetConnectionCredentials\">\n"
  "   <arg type=\"s\" direction=\"in\"/>\n"
  "   <arg type=\"a{sv}\" direction=\"out\"/>\n"
  "  </method>\n"
  "  <method name=\"GetConnectionSELinuxSecurityContext\">\n"
  "   <arg type=\"s\" direction=\"in\"/>\n"
  "   <arg type=\"ay\" direction=\"out\"/>\n"
  "  </method>\n"
  "  <method name=\"GetConnectionUnixProcessID\">\n"
  "   <arg type=\"s\" direction=\"in\"/>\n"
  "   <arg type=\"u\" direction=\"out\"/>\n"
  "  </method>\n"
  "  <method name=\"GetConnectionUnixUser\">\n"
  "   <arg type=\"s\" direction=\"in\"/>\n"
  "   <arg type=\"u\" direction=\"out\"/>\n"
  "  </method>\n"
  "  <method name=\"GetId\">\n"
  "   <arg type=\"s\" direction=\"out\"/>\n"
  "  </method>\n"
  "  <method name=\"GetNameOwner\">\n"
  "   <arg type=\"s\" direction=\"in\"/>\n"
  "   <arg type=\"s\" direction=\"out\"/>\n"
  "  </method>\n"
  "  <method name=\"Hello\">\n"
  "   <arg type=\"s\" direction=\"out\"/>\n"
  "  </method>\n"
  "  <method name=\"ListActivatableNames\">\n"
  "   <arg type=\"as\" direction=\"out\"/>\n"
  "  </method>\n"
  "  <method name=\"ListNames\">\n"
  "   <arg type=\"as\" direction=\"out\"/>\n"
  "  </method>\n"
  "  <method name=\"ListQueuedOwners\">\n"
  "   <arg type=\"s\" direction=\"in\"/>\n"
  "   <arg type=\"as\" direction=\"out\"/>\n"
  "  </method>\n"
  "  <method name=\"NameHasOwner\">\n"
  "   <arg type=\"s\" direction=\"in\"/>\n"
  "   <arg type=\"b\" direction=\"out\"/>\n"
  "  </method>\n"
  "  <method name=\"ReleaseName\">\n"
  "   <arg type=\"s\" direction=\"in\"/>\n"
  "   <arg type=\"u\" direction=\"out\"/>\n"
  "  </method>\n"
  "  <method name=\"ReloadConfig\">\n"
  "  </method>\n"
  "  <method name=\"RequestName\">\n"
  "   <arg type=\"s\" direction=\"in\"/>\n"
  "   <arg type=\"u\" direction=\"in\"/>\n"
  "   <arg type=\"u\" direction=\"out\"/>\n"
  "  </method>\n"
  "  <method name=\"StartServiceByName\">\n"
  "   <arg type=\"s\" direction=\"in\"/>\n"
  "   <arg type=\"u\" direction=\"in\"/>\n"
  "   <arg type=\"u\" direction=\"out\"/>\n"
  "  </method>\n"
  "  <method name=\"UpdateActivationEnvironment\">\n"
  "   <arg type=\"a{ss}\" direction=\"in\"/>\n"
  "  </method>\n"
  "  <signal name=\"NameAcquired\">\n"
  "   <arg type=\"s\"/>\n"
  "  </signal>\n"
  "  <signal name=\"NameLost\">\n"
  "   <arg type=\"s\"/>\n"
  "  </signal>\n"
  "  <signal name=\"NameOwnerChanged\">\n"
  "   <arg type=\"s\"/>\n"
  "   <arg type=\"s\"/>\n"
  "   <arg type=\"s\"/>\n"
  "  </signal>\n"
  " </interface>\n"
  "</node>\n";


/**
 * _is_message_to_dbus_daemon:
 *
 */
gboolean
_is_message_to_dbus_daemon (GDBusMessage  *message)
{
  return g_strcmp0 (g_dbus_message_get_destination (message), "org.freedesktop.DBus") == 0 &&
         (g_strcmp0 (g_dbus_message_get_interface (message), "org.freedesktop.DBus") == 0 ||
          g_strcmp0 (g_dbus_message_get_interface (message), "org.freedesktop.DBus.Introspectable") == 0) &&
         (g_strcmp0 (g_dbus_message_get_path (message), "/org/freedesktop/DBus") == 0 ||
          g_strcmp0 (g_dbus_message_get_path (message), "/") == 0);
}


/**
 * _dbus_daemon_synthetic_reply:
 *
 */
GDBusMessage *
_dbus_daemon_synthetic_reply (GKDBusWorker  *worker,
                              GDBusMessage  *message)
{
  GDBusMessage *reply;
  GVariant     *reply_body;
  GVariant     *body;
  GError       *local_error;
  const gchar  *member;

  reply = NULL;
  reply_body = NULL;
  local_error = NULL;

  member = g_dbus_message_get_member (message);
  body = g_dbus_message_get_body (message);

  if (!g_strcmp0 (member, "Introspect"))
    {
      reply_body = g_variant_new ("(s)", introspect);
    }
  else if (!g_strcmp0 (member, "AddMatch"))
    {
      if (body != NULL && g_variant_is_of_type (body, G_VARIANT_TYPE ("(s)")))
        {
          gchar *rule;

          g_variant_get (body, "(&s)", &rule);
          reply_body = _g_kdbus_AddMatch (worker, rule, &local_error);
        }
      else
        g_set_error (&local_error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'AddMatch' has wrong args (expected s)");
    }
  else if (!g_strcmp0 (member, "RemoveMatch"))
    {
      if (body != NULL && g_variant_is_of_type (body, G_VARIANT_TYPE ("(s)")))
        {
          gchar *rule;

          g_variant_get (body, "(&s)", &rule);
          reply_body = _g_kdbus_RemoveMatch (worker, rule, &local_error);
        }
      else
        g_set_error (&local_error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'RemoveMatch' has wrong args (expected s)");
    }
  else if (!g_strcmp0 (member, "GetConnectionCredentials"))
    {
      if (body != NULL && g_variant_is_of_type (body, G_VARIANT_TYPE ("(s)")))
        {
          GVariantBuilder builder;
          GVariant *variant;
          GVariant *label;
          guint32 uid, pid;
          gchar *name;

          g_variant_get (body, "(&s)", &name);
          g_variant_builder_init (&builder, G_VARIANT_TYPE ("(a{sv})"));
          g_variant_builder_open (&builder, G_VARIANT_TYPE ("a{sv}"));

          variant = _g_kdbus_GetConnectionUnixUser (worker, name, NULL);
          if (variant != NULL)
            {
              g_variant_get (variant, "(u)", &uid);
              g_variant_builder_add (&builder, "{sv}", "UnixUserID", g_variant_new_uint32 (uid));
              g_variant_unref (variant);
            }

/*
          variant = _g_kdbus_GetConnectionUnixProcessID (worker, name, NULL);
          if (variant != NULL)
            {
              g_variant_get (variant, "(u)", &pid);
              g_variant_builder_add (&builder, "{sv}", "ProcessID", g_variant_new_uint32 (pid));
              g_variant_unref (variant);
            }
*/

          label = _g_kdbus_GetConnectionSELinuxSecurityContext (worker, name, &local_error);
          if (label != NULL)
            g_variant_builder_add (&builder, "{sv}", "LinuxSecurityLabel", label);

          g_variant_builder_close (&builder);
          reply_body = g_variant_builder_end (&builder);

          if (local_error != NULL)
            {
              g_variant_unref (reply_body);
              reply_body = NULL;
            }
        }
      else
        g_set_error (&local_error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'GetConnectionCredentials' has wrong args (expected s)");
    }
  else if (!g_strcmp0 (member, "GetConnectionSELinuxSecurityContext"))
    {
      if (body != NULL && g_variant_is_of_type (body, G_VARIANT_TYPE ("(s)")))
        {
          gchar *name;

          g_variant_get (body, "(&s)", &name);
          reply_body = _g_kdbus_GetConnectionSELinuxSecurityContext (worker, name, &local_error);
          if (reply_body == NULL && local_error == NULL)
            g_set_error (&local_error, G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED, "Operation not supported");
        }
      else
        g_set_error (&local_error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'GetConnectionSELinuxSecurityContext' has wrong args (expected s)");
    }
  else if (!g_strcmp0 (member, "GetConnectionUnixProcessID"))
    {
      if (body != NULL && g_variant_is_of_type (body, G_VARIANT_TYPE ("(s)")))
        {
          gchar *name;
          pid_t pid;

          g_variant_get (body, "(&s)", &name);
          pid = _g_kdbus_GetConnectionUnixProcessID (worker, name, &local_error);
          if (local_error == NULL)
            reply_body = g_variant_new ("(u)", pid);
        }
      else
        g_set_error (&local_error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'GetConnectionUnixProcessID' has wrong args (expected s)");
    }
  else if (!g_strcmp0 (member, "GetConnectionUnixUser"))
    {
      if (body != NULL && g_variant_is_of_type (body, G_VARIANT_TYPE ("(s)")))
        {
          gchar *name;

          g_variant_get (body, "(&s)", &name);
          reply_body = _g_kdbus_GetConnectionUnixUser (worker, name, &local_error);
        }
      else
        g_set_error (&local_error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'GetConnectionUnixUser' has wrong args (expected s)");
    }
  else if (!g_strcmp0 (member, "GetId"))
    {
      if ((body == NULL) || g_variant_is_of_type (body, G_VARIANT_TYPE_TUPLE))
        {
          gchar *bus_id;

          bus_id = _g_kdbus_GetBusId (worker, &local_error);
          reply_body = g_variant_new ("(s)", bus_id);

          g_free (bus_id);
        }
      else
        g_set_error (&local_error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'GetId' has wrong args");
    }
  else if (!g_strcmp0 (member, "GetNameOwner"))
    {
      if (body != NULL && g_variant_is_of_type (body, G_VARIANT_TYPE ("(s)")))
        {
          gchar *unique_name;
          gchar *name;

          g_variant_get (body, "(&s)", &name);
          unique_name = _g_kdbus_GetNameOwner (worker, name, &local_error);
          if (local_error == NULL)
            reply_body = g_variant_new ("(s)", unique_name);
          g_free (unique_name);
        }
      else
        g_set_error (&local_error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'GetNameOwner' has wrong args (expected s)");
    }
  else if (!g_strcmp0 (member, "Hello"))
    {
      if ((body == NULL) || g_variant_is_of_type (body, G_VARIANT_TYPE_TUPLE))
        {
          const gchar *unique_name;

          unique_name = _g_kdbus_Hello (worker, &local_error);
          if (local_error == NULL)
            reply_body = g_variant_new ("(s)", unique_name);
        }
      else
        g_set_error (&local_error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'Hello' has wrong args");
    }
  else if (!g_strcmp0 (member, "ListActivatableNames"))
    {
      if ((body == NULL) || g_variant_is_of_type (body, G_VARIANT_TYPE_TUPLE))
        {
          gchar **strv;
          gint cnt;

          cnt = 0;

          strv = _g_kdbus_GetListNames (worker, TRUE, &local_error);
          if (local_error == NULL)
            {
              GVariantBuilder *builder;

              builder = g_variant_builder_new (G_VARIANT_TYPE ("as"));

              while (strv[cnt])
                g_variant_builder_add (builder, "s", strv[cnt++]);

              reply_body = g_variant_new ("(as)", builder);

              g_variant_builder_unref (builder);
            }
          g_strfreev (strv);
        }
      else
        g_set_error (&local_error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'ListActivatableNames' has wrong args");
    }
  else if (!g_strcmp0 (member, "ListNames"))
    {
      if ((body == NULL) || g_variant_is_of_type (body, G_VARIANT_TYPE_TUPLE))
        {
          gchar **strv;
          gint cnt;

          cnt = 0;

          strv = _g_kdbus_GetListNames (worker, FALSE, &local_error);
          if (local_error == NULL)
            {
              GVariantBuilder *builder;

              builder = g_variant_builder_new (G_VARIANT_TYPE ("as"));

              while (strv[cnt])
                g_variant_builder_add (builder, "s", strv[cnt++]);

              reply_body = g_variant_new ("(as)", builder);

              g_variant_builder_unref (builder);
            }
          g_strfreev (strv);
        }
      else
        g_set_error (&local_error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'ListNames' has wrong args");
    }
  else if (!g_strcmp0 (member, "ListQueuedOwners"))
    {
      if (body != NULL && g_variant_is_of_type (body, G_VARIANT_TYPE ("(s)")))
        {
          gchar **strv;
          gchar *name;
          gint cnt;

          cnt = 0;

          g_variant_get (body, "(&s)", &name);
          strv = _g_kdbus_GetListQueuedOwners (worker, name, &local_error);
          if (local_error == NULL)
            {
              GVariantBuilder *builder;

              builder = g_variant_builder_new (G_VARIANT_TYPE ("as"));

              while (strv[cnt])
                g_variant_builder_add (builder, "s", strv[cnt++]);

              reply_body = g_variant_new ("(as)", builder);

              g_variant_builder_unref (builder);
            }
          g_strfreev (strv);
        }
      else
        g_set_error (&local_error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'ListQueuedOwners' has wrong args (expected s)");
    }
  else if (!g_strcmp0 (member, "NameHasOwner"))
    {
      if (body != NULL && g_variant_is_of_type (body, G_VARIANT_TYPE ("(s)")))
        {
          gboolean ret;
          gchar *name;

          g_variant_get (body, "(&s)", &name);

          ret = _g_kdbus_NameHasOwner (worker, name, &local_error);
          if (local_error == NULL)
            {
              if (ret)
                reply_body = g_variant_new ("(b)", TRUE);
              else
                reply_body = g_variant_new ("(b)", FALSE);
            }
        }
      else
        g_set_error (&local_error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'NameHasOwner' has wrong args (expected s)");
    }
  else if (!g_strcmp0 (member, "ReleaseName"))
    {
      if (body != NULL && g_variant_is_of_type (body, G_VARIANT_TYPE ("(s)")))
        {
          GBusReleaseNameReplyFlags status;
          gchar *name;

          g_variant_get (body, "(&s)", &name);

          status = _g_kdbus_ReleaseName (worker, name, &local_error);
          if (local_error == NULL)
            reply_body = g_variant_new ("(u)", status);
        }
      else
        g_set_error (&local_error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'ReleaseName' has wrong args (expected s)");
    }
  else if (!g_strcmp0 (member, "ReloadConfig"))
    {
      if ((body == NULL) || g_variant_is_of_type (body, G_VARIANT_TYPE_TUPLE))
        reply_body = g_variant_new ("()", NULL);
      else
        g_set_error (&local_error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'ReloadConfig' has wrong args");
    }
  else if (!g_strcmp0 (member, "RequestName"))
    {
      if (body != NULL && g_variant_is_of_type (body, G_VARIANT_TYPE ("(su)")))
        {
          GBusRequestNameReplyFlags status;
          guint32 flags;
          gchar *name;

          g_variant_get (body, "(&su)", &name, &flags);

          status = _g_kdbus_RequestName (worker, name, flags, &local_error);
          if (local_error == NULL)
            reply_body = g_variant_new ("(u)", status);
        }
      else
        g_set_error (&local_error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'RequestName' has wrong args (expected su)");
    }
  else if (!g_strcmp0 (member, "StartServiceByName"))
    {
      if (body != NULL && g_variant_is_of_type (body, G_VARIANT_TYPE ("(su)")))
        {
          gchar *name;
          guint32 flags;

          g_variant_get (body, "(&su)", &name, &flags);
          reply_body = _g_kdbus_StartServiceByName (worker, name, flags, &local_error);
        }
      else
        g_set_error (&local_error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'StartServiceByName' has wrong args (expected su)");
    }
  else if (!g_strcmp0 (member, "UpdateActivationEnvironment"))
    {
      g_set_error (&local_error, G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED,
                   "'%s' method not supported", member);
    }
  else
    {
      g_set_error (&local_error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD,
                   "org.freedesktop.DBus does not understand message %s", member);
    }

  if (reply_body == NULL)
    {
      gchar *dbus_error_name;

      dbus_error_name = g_dbus_error_encode_gerror (local_error);
      reply = g_dbus_message_new_method_error (message, dbus_error_name, local_error->message);
      g_free (dbus_error_name);
    }
  else
    {
      reply = g_dbus_message_new_method_reply (message);
      g_dbus_message_set_body (reply, reply_body);
    }

  g_dbus_message_set_serial (reply, -1);

  if (local_error)
    g_error_free (local_error);

  return reply;
}
