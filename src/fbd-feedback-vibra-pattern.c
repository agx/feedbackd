/*
 * Copyright (C) 2024 The Phosh Developers
 *
 * SPDX-License-Identifier: GPL-3.0+

 * Author: Guido Günther <agx@sigxcpu.org>
 */

#define G_LOG_DOMAIN "fbd-feedback-vibra-pattern"

#include "fbd-enums.h"
#include "fbd-feedback-vibra-pattern.h"
#include "fbd-feedback-vibra-priv.h"
#include "fbd-feedback-manager.h"

/**
 * FbdFeedbackVibraPattern:
 *
 * Describes a pattern feedback via a haptic motor
 *
 * The #FbdVibraVibraPattern describes the properties of a haptic feedback
 * event. It knows nothing about the hardware itself but calls
 * #FbdDevVibra for that.
 */

enum {
  PROP_0,
  PROP_MAGNITUDES,
  PROP_DURATIONS,
  PROP_LAST_PROP,
};
static GParamSpec *props[PROP_LAST_PROP];

typedef struct _FbdFeedbackVibraPattern {
  FbdFeedbackVibra parent;

  GArray          *magnitudes;
  GArray          *durations;
  int              pos;

  guint            timer_id;
} FbdFeedbackVibraPattern;

G_DEFINE_TYPE (FbdFeedbackVibraPattern, fbd_feedback_vibra_pattern, FBD_TYPE_FEEDBACK_VIBRA);


static void
set_magnitudes (FbdFeedbackVibraPattern *self, GArray *magnitudes)
{
  g_clear_pointer (&self->magnitudes, g_array_unref);
  self->magnitudes = g_array_ref (magnitudes);
}


static void
set_durations (FbdFeedbackVibraPattern *self, GArray *durations)
{
  guint total_duration = 0;

  g_clear_pointer (&self->durations, g_array_unref);
  self->durations = g_array_ref (durations);

  for (int i = 0; i < self->durations->len; i++) {
    guint *duration = &g_array_index (self->durations, guint, i);
    total_duration += *duration;
  }

  fbd_feedback_vibra_set_duration (FBD_FEEDBACK_VIBRA (self), total_duration);
}


static void
fbd_feedback_vibra_pattern_set_property (GObject      *object,
                                         guint         property_id,
                                         const GValue *value,
                                         GParamSpec   *pspec)
{
  FbdFeedbackVibraPattern *self = FBD_FEEDBACK_VIBRA_PATTERN (object);

  switch (property_id) {
  case PROP_MAGNITUDES:
    set_magnitudes (self, g_value_get_boxed (value));
    break;
  case PROP_DURATIONS:
    set_durations (self, g_value_get_boxed (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


static void
fbd_feedback_vibra_pattern_get_property (GObject    *object,
                                         guint       property_id,
                                         GValue     *value,
                                         GParamSpec *pspec)
{
  FbdFeedbackVibraPattern *self = FBD_FEEDBACK_VIBRA_PATTERN (object);

  switch (property_id) {
  case PROP_MAGNITUDES:
    g_value_set_boxed (value, self->magnitudes);
    break;
  case PROP_DURATIONS:
    g_value_set_boxed (value, self->durations);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


static void on_timer_expired (gpointer data);


static void
do_pattern_step (FbdFeedbackVibraPattern *self)
{
  FbdFeedbackManager *manager = fbd_feedback_manager_get_default ();
  FbdDevVibra *dev = fbd_feedback_manager_get_dev_vibra (manager);
  double *magnitude;
  guint *duration;

  magnitude = &g_array_index (self->magnitudes, double, self->pos);
  duration = &g_array_index (self->durations, guint, self->pos);

  g_debug ("step: pos: %d/%d, magn: %f, timeout %u",
           self->pos,
           self->durations->len,
           *magnitude,
           *duration);

  /* Removing any pending effect */
  fbd_dev_vibra_remove_effect (dev);

  if (*magnitude != 0.0)
    fbd_dev_vibra_rumble (dev, *magnitude, *duration, TRUE);

  self->timer_id = g_timeout_add_once (*duration,
                                       on_timer_expired,
                                       self);
  g_source_set_name_by_id (self->timer_id, "feedback-vibra-pattern-timer");
}


static void
on_timer_expired (gpointer data)
{
  FbdFeedbackVibraPattern *self = data;

  g_return_if_fail (FBD_IS_FEEDBACK_VIBRA_PATTERN (self));
  g_return_if_fail (self->pos < self->durations->len);

  self->pos++;

  if (self->pos == self->durations->len) {
    self->pos = 0;
    self->timer_id = 0;
    return;
  }

  do_pattern_step (self);
}


static void
fbd_feedback_vibra_pattern_end_vibra (FbdFeedbackVibra *vibra)
{
  FbdFeedbackVibraPattern *self = FBD_FEEDBACK_VIBRA_PATTERN (vibra);
  FbdFeedbackManager *manager = fbd_feedback_manager_get_default ();
  FbdDevVibra *dev = fbd_feedback_manager_get_dev_vibra (manager);

  self->pos = 0;
  g_clear_handle_id (&self->timer_id, g_source_remove);

  fbd_dev_vibra_stop (dev);
}


static void
fbd_feedback_vibra_pattern_start_vibra (FbdFeedbackVibra *vibra)
{
  FbdFeedbackVibraPattern *self = FBD_FEEDBACK_VIBRA_PATTERN (vibra);
  FbdFeedbackManager *manager = fbd_feedback_manager_get_default ();
  FbdDevVibra *dev = fbd_feedback_manager_get_dev_vibra (manager);

  g_return_if_fail (FBD_IS_DEV_VIBRA (dev));
  g_return_if_fail (self->magnitudes);
  g_return_if_fail (self->durations);
  g_return_if_fail (self->durations->len == self->magnitudes->len);
  g_return_if_fail (self->pos == 0);

  g_debug ("Pattern Vibra: %u elements", self->durations->len);

  if (self->durations->len == 0)
    return;

  do_pattern_step (self);
}


static gboolean
fbd_feedback_vibra_pattern_is_available (FbdFeedbackBase *base)
{
  FbdFeedbackManager *manager = fbd_feedback_manager_get_default ();
  FbdDevVibra *dev = fbd_feedback_manager_get_dev_vibra (manager);

  return FBD_IS_DEV_VIBRA (dev);
}


static void
fbd_feedback_vibra_pattern_finalize (GObject *object)
{
  FbdFeedbackVibraPattern *self = FBD_FEEDBACK_VIBRA_PATTERN (object);

  fbd_feedback_vibra_pattern_end_vibra (FBD_FEEDBACK_VIBRA (self));
  g_clear_pointer (&self->magnitudes, g_array_unref);
  g_clear_pointer (&self->durations, g_array_unref);

  G_OBJECT_CLASS (fbd_feedback_vibra_pattern_parent_class)->finalize (object);
}


static void
fbd_feedback_vibra_pattern_class_init (FbdFeedbackVibraPatternClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  FbdFeedbackBaseClass *base_class = FBD_FEEDBACK_BASE_CLASS (klass);
  FbdFeedbackVibraClass *vibra_class = FBD_FEEDBACK_VIBRA_CLASS (klass);

  object_class->finalize = fbd_feedback_vibra_pattern_finalize;
  object_class->set_property = fbd_feedback_vibra_pattern_set_property;
  object_class->get_property = fbd_feedback_vibra_pattern_get_property;

  base_class->is_available = fbd_feedback_vibra_pattern_is_available;

  vibra_class->start_vibra = fbd_feedback_vibra_pattern_start_vibra;
  vibra_class->end_vibra = fbd_feedback_vibra_pattern_end_vibra;

  /**
   * FbdFeedbackVibraPattern:magnitudes
   *
   * The magnitudes as array of doubles. The values specify
   * the relative magnitude of a rumble. A value of 0 indicates
   * a pause.
   */
  props[PROP_MAGNITUDES] =
    g_param_spec_boxed ("magnitudes", "", "",
                        G_TYPE_ARRAY,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  /**
   * FbdFeedbackVibraPattern:durations
   *
   * The durations as array of unsigned integers. The values specify
   * an the length of each rumble.
   */
  props[PROP_DURATIONS] =
    g_param_spec_boxed ("durations", "", "",
                        G_TYPE_ARRAY,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, PROP_LAST_PROP, props);
}


static void
fbd_feedback_vibra_pattern_init (FbdFeedbackVibraPattern *self)
{
}


FbdFeedbackVibraPattern *
fbd_feedback_vibra_pattern_new (GArray *magnitudes, GArray *durations)
{
  return g_object_new (FBD_TYPE_FEEDBACK_VIBRA_PATTERN,
                       "magnitudes", magnitudes,
                       "durations", durations,
                       NULL);
}
