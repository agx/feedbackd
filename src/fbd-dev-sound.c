/*
 * Copyright (C) 2020 Purism SPC
 * SPDX-License-Identifier: GPL-3.0+
 * Author: Guido Günther <agx@sigxcpu.org>
 *
 * See https://www.kernel.org/doc/html/latest/input/ff.html
 * and fftest.c from the joystick package.
 */

#define G_LOG_DOMAIN "fbd-dev-sound"

#include "fbd-dev-sound.h"
#include "fbd-feedback-sound.h"

#include <gsound.h>

#define GNOME_SOUND_SCHEMA_ID "org.gnome.desktop.sound"
#define GNOME_SOUND_KEY_THEME_NAME "theme-name"

/**
 * SECTION:fbd-dev-sound
 * @short_description: Sound interface
 * @Title: FbdDevSound
 *
 * The #FbdDevSound is used to play sounds via the systems audio
 * system.
 */

typedef struct _FbdAsyncData {
  FbdDevSoundPlayedCallback  callback;
  FbdFeedbackSound          *feedback;
} FbdAsyncData;

typedef struct _FbdDevSound {
  GObject parent;

  GSoundContext *ctx;
  GSettings *sound_settings;
} FbdDevSound;

static void initable_iface_init (GInitableIface *iface);

G_DEFINE_TYPE_WITH_CODE (FbdDevSound, fbd_dev_sound, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE, initable_iface_init));

static void
on_sound_theme_name_changed (FbdDevSound *self,
			     const gchar *key,
			     GSettings   *settings)
{
  gboolean ok;
  g_autoptr(GError) error = NULL;
  g_autofree gchar *name = NULL;

  g_return_if_fail (FBD_IS_DEV_SOUND (self));
  g_return_if_fail (G_IS_SETTINGS (settings));
  g_return_if_fail (!g_strcmp0 (key, GNOME_SOUND_KEY_THEME_NAME));
  g_return_if_fail (self->ctx);

  name = g_settings_get_string (settings, key);
  g_debug ("Setting sound theme to %s", name);

  ok = gsound_context_set_attributes (self->ctx,
				      &error,
				      GSOUND_ATTR_CANBERRA_XDG_THEME_NAME,
				      name,
				      NULL);
  if (!ok)
    g_warning ("Failed to set sound theme name to %s: %s", key, error->message);
}

static void
fbd_dev_sound_dispose (GObject *object)
{
  FbdDevSound *self = FBD_DEV_SOUND (object);

  g_clear_object (&self->ctx);
  g_clear_object (&self->sound_settings);

  G_OBJECT_CLASS (fbd_dev_sound_parent_class)->dispose (object);
}

static gboolean
initable_init (GInitable     *initable,
                     GCancellable  *cancellable,
                     GError       **error)
{
  FbdDevSound *self = FBD_DEV_SOUND (initable);
  const char *desktop;

  self->ctx = gsound_context_new(NULL, error);
  if (!self->ctx)
    return FALSE;

  desktop = g_getenv ("XDG_CURRENT_DESKTOP");
  if (!g_strcmp0 (desktop, "GNOME")) {
    self->sound_settings = g_settings_new (GNOME_SOUND_SCHEMA_ID);

    g_signal_connect_object (self->sound_settings, "changed::" GNOME_SOUND_KEY_THEME_NAME,
			     G_CALLBACK (on_sound_theme_name_changed), self,
			     G_CONNECT_SWAPPED);
    on_sound_theme_name_changed (self, GNOME_SOUND_KEY_THEME_NAME, self->sound_settings);
  }

  return TRUE;
}

static void
initable_iface_init (GInitableIface *iface)
{
    iface->init = initable_init;
}

static void
fbd_dev_sound_class_init (FbdDevSoundClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = fbd_dev_sound_dispose;
}

static void
fbd_dev_sound_init (FbdDevSound *self)
{
}

FbdDevSound *
fbd_dev_sound_new (GError **error)
{
  return FBD_DEV_SOUND (g_initable_new (FBD_TYPE_DEV_SOUND,
                                        NULL,
                                        error,
                                        NULL));
}


static void
on_sound_play_finished_callback (GSoundContext *ctx,
                                 GAsyncResult  *res,
                                 FbdAsyncData  *data)
{
  GError *err = NULL;

  if (!gsound_context_play_full_finish (ctx, res, &err)) {
    g_warning ("Failed to play sound: %s", err->message);
  }

  (*data->callback)(data->feedback);
  g_object_unref (data->feedback);
  g_free (data);
}


gboolean
fbd_dev_sound_play (FbdDevSound *self, FbdFeedbackSound *feedback, FbdDevSoundPlayedCallback callback)
{
  FbdAsyncData *data;

  g_return_val_if_fail (FBD_IS_DEV_SOUND (self), FALSE);
  g_return_val_if_fail (GSOUND_IS_CONTEXT (self->ctx), FALSE);

  data = g_new0 (FbdAsyncData, 1);
  data->callback = callback;
  data->feedback = g_object_ref (feedback);

  gsound_context_play_full (self->ctx, NULL,
                            (GAsyncReadyCallback)on_sound_play_finished_callback,
                            data,
                            GSOUND_ATTR_EVENT_ID, fbd_feedback_sound_get_effect (feedback),
                            GSOUND_ATTR_EVENT_DESCRIPTION, "Feedbackd sound feedback",
                            NULL);
  return TRUE;
}
