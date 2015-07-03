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

#include <gio/giotypes.h>
#include <gio/gio.h>

#include "gkdbuscompatibility.h"

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
 * emulate_dbus_daemon:
 *
 */
gboolean
emulate_dbus_daemon (GDBusMessage *message)
{
  return g_strcmp0 (g_dbus_message_get_destination (message), "org.freedesktop.DBus") == 0 &&
         (g_strcmp0 (g_dbus_message_get_interface (message), "org.freedesktop.DBus") == 0 ||
          g_strcmp0 (g_dbus_message_get_interface (message), "org.freedesktop.DBus.Introspectable") == 0) &&
         (g_strcmp0 (g_dbus_message_get_path (message), "/org/freedesktop/DBus") == 0 ||
          g_strcmp0 (g_dbus_message_get_path (message), "/") == 0);
}


/**
 * prepare_synthetic_reply:
 *
 */
gboolean
prepare_synthetic_reply (GKDBusWorker  *worker,
                         GDBusMessage  *message)
{
  GDBusMessage *reply;
  GVariant *body, *reply_body;
  GError *error;
  const gchar *member;

  reply = NULL;
  reply_body = NULL;
  error = NULL;

  member = g_dbus_message_get_member (message);
  body = g_dbus_message_get_body (message);

  if (!g_strcmp0 (member, "Introspect"))
    {
      reply_body = g_variant_new ("(s)", introspect);
    }
  else if (!g_strcmp0 (member, "AddMatch"))
    {
      if (g_variant_is_of_type (body, G_VARIANT_TYPE ("(s)")))
        {
          gchar *rule;

/* TODO

          if (g_strcmp0 (signal_data->sender_unique_name, "org.freedesktop.DBus") == 0 &&
              g_strcmp0 (signal_data->interface_name, "org.freedesktop.DBus") == 0 &&
              g_strcmp0 (signal_data->object_path, "/org/freedesktop/DBus") == 0 &&
              (g_strcmp0 (signal_data->member, "NameLost") == 0 ||
               g_strcmp0 (signal_data->member, "NameAcquired") == 0 ||
               g_strcmp0 (signal_data->member, "NameOwnerChanged") == 0))
            {
              if (g_strcmp0 (signal_data->member, "NameAcquired") == 0)
                _g_kdbus_subscribe_name_acquired (connection->kdbus_worker, signal_data->rule, arg0, NULL);
              else if (g_strcmp0 (signal_data->member, "NameLost") == 0)
                _g_kdbus_subscribe_name_lost (connection->kdbus_worker, signal_data->rule, arg0, NULL);
              else if (g_strcmp0 (signal_data->member, "NameOwnerChanged") == 0)
                _g_kdbus_subscribe_name_owner_changed (connection->kdbus_worker, signal_data->rule, arg0, NULL);
            }
*/
          g_variant_get (body, "(&s)", &rule);
          reply_body = _g_kdbus_AddMatch_tmp (worker, rule, &error);
        }
      else
        g_set_error (&error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'AddMatch' has wrong args (expected s)");
    }
  else if (!g_strcmp0 (member, "RemoveMatch"))
    {
      if (g_variant_is_of_type (body, G_VARIANT_TYPE ("(s)")))
        {
          gchar *rule;

          g_variant_get (body, "(&s)", &rule);
          reply_body = _g_kdbus_RemoveMatch_tmp (worker, rule, &error);
        }
      else
        g_set_error (&error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'RemoveMatch' has wrong args (expected s)");
    }
  else if (!g_strcmp0 (member, "GetConnectionCredentials"))
    {
      //TODO
      reply_body = g_variant_new ("()", NULL);
    }
  else if (!g_strcmp0 (member, "GetConnectionSELinuxSecurityContext"))
    {
      if (g_variant_is_of_type (body, G_VARIANT_TYPE ("(s)")))
        {
          gchar *name;

          g_variant_get (body, "(&s)", &name);
          reply_body = _g_kdbus_GetConnectionSELinuxSecurityContext (worker, name, &error);
          if (reply_body == NULL && error == NULL)
            g_set_error (&error, G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED, "Operation not supported");
        }
      else
        g_set_error (&error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'GetConnectionSELinuxSecurityContext' has wrong args (expected s)");
    }
  else if (!g_strcmp0 (member, "GetConnectionUnixProcessID"))
    {
      if (g_variant_is_of_type (body, G_VARIANT_TYPE ("(s)")))
        {
          gchar *name;

          g_variant_get (body, "(&s)", &name);
          reply_body = _g_kdbus_GetConnectionUnixProcessID (worker, name, &error);
        }
      else
        g_set_error (&error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'GetConnectionUnixProcessID' has wrong args (expected s)");
    }
  else if (!g_strcmp0 (member, "GetConnectionUnixUser"))
    {
      if (g_variant_is_of_type (body, G_VARIANT_TYPE ("(s)")))
        {
          gchar *name;

          g_variant_get (body, "(&s)", &name);
          reply_body = _g_kdbus_GetConnectionUnixUser (worker, name, &error);
        }
      else
        g_set_error (&error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'GetConnectionUnixUser' has wrong args (expected s)");
    }
  else if (!g_strcmp0 (member, "GetId"))
    {
      if (body == NULL)
        reply_body = _g_kdbus_GetBusId (worker, &error);
      else
        g_set_error (&error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'GetId' has wrong args");
    }
  else if (!g_strcmp0 (member, "GetNameOwner"))
    {
      if (g_variant_is_of_type (body, G_VARIANT_TYPE ("(s)")))
        {
          gchar *name;

          g_variant_get (body, "(&s)", &name);
          reply_body = _g_kdbus_GetNameOwner (worker, name, &error);
        }
      else
        g_set_error (&error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'GetNameOwner' has wrong args (expected s)");
    }
  else if (!g_strcmp0 (member, "Hello"))
    {
      if (body == NULL)
        reply_body = _g_kdbus_Hello (worker, &error);
      else
        g_set_error (&error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'Hello' has wrong args");
    }
  else if (!g_strcmp0 (member, "ListActivatableNames"))
    {
      if (body == NULL)
        reply_body = _g_kdbus_GetListNames (worker, 1, &error);
      else
        g_set_error (&error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'ListActivatableNames' has wrong args");
    }
  else if (!g_strcmp0 (member, "ListNames"))
    {
      if (body == NULL)
        reply_body = _g_kdbus_GetListNames (worker, 0, &error);
      else
        g_set_error (&error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'ListNames' has wrong args");
    }
  else if (!g_strcmp0 (member, "ListQueuedOwners"))
    {
      if (g_variant_is_of_type (body, G_VARIANT_TYPE ("(s)")))
        {
          gchar *name;

          g_variant_get (body, "(&s)", &name);
          reply_body = _g_kdbus_GetListQueuedOwners (worker, name, &error);
        }
      else
        g_set_error (&error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'ListQueuedOwners' has wrong args (expected s)");
    }
  else if (!g_strcmp0 (member, "NameHasOwner"))
    {
      if (g_variant_is_of_type (body, G_VARIANT_TYPE ("(s)")))
        {
          gchar *name;

          g_variant_get (body, "(&s)", &name);
          reply_body = _g_kdbus_NameHasOwner (worker, name, &error);
        }
      else
        g_set_error (&error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'NameHasOwner' has wrong args (expected s)");
    }
  else if (!g_strcmp0 (member, "ReleaseName"))
    {
      if (g_variant_is_of_type (body, G_VARIANT_TYPE ("(s)")))
        {
          gchar *name;

          g_variant_get (body, "(&s)", &name);
          reply_body = _g_kdbus_ReleaseName (worker, name, &error);
        }
      else
        g_set_error (&error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'ReleaseName' has wrong args (expected s)");
    }
  else if (!g_strcmp0 (member, "ReloadConfig"))
    {
      if (body == NULL)
        reply_body = g_variant_new ("()", NULL);
      else
        g_set_error (&error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'ReloadConfig' has wrong args");
    }
  else if (!g_strcmp0 (member, "RequestName"))
    {
      if (g_variant_is_of_type (body, G_VARIANT_TYPE ("(su)")))
        {
          gchar *name;
          guint32 flags;

          g_variant_get (body, "(&su)", &name, &flags);
          reply_body = _g_kdbus_RequestName (worker, name, flags, &error);
        }
      else
        g_set_error (&error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                     "Call to 'RequestName' has wrong args (expected s)");
    }
  else if (!g_strcmp0 (member, "StartServiceByName"))
    {
      //TODO
      reply_body = g_variant_new ("()", NULL);
    }
  else if (!g_strcmp0 (member, "UpdateActivationEnvironment"))
    {
      //TODO
      reply_body = g_variant_new ("()", NULL);
    }
  else
    { 
      /* unsupported method - set error here */
      g_set_error (&error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD,
                   "org.freedesktop.DBus does not understand message %s", member);
    }

  /* xxx */
  if (reply_body == NULL)
    {
      gchar *dbus_error_name;

      dbus_error_name = g_dbus_error_encode_gerror (error);
      reply = g_dbus_message_new_method_error (message, dbus_error_name, error->message);
      g_free (dbus_error_name);
    }
  else
    {
      reply = g_dbus_message_new_method_reply (message);
      g_dbus_message_set_body (reply, reply_body);
    }

  g_dbus_message_set_serial (reply, -1);
  g_kdbus_worker_send_message (worker, reply, NULL);

  if (error)
    g_error_free (error);

  g_object_unref (reply);
  return TRUE;
}
