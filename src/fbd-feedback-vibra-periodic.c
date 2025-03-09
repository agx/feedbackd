/*
 * Copyright (C) 2020 Purism SPC
 * SPDX-License-Identifier: GPL-3.0+
 * Author: Guido Günther <agx@sigxcpu.org>
 */

#define G_LOG_DOMAIN "fbd-feedback-vibra-periodic"

#include "fbd-enums.h"
#include "fbd-feedback-vibra-priv.h"
#include "fbd-feedback-vibra-periodic.h"
#include "fbd-feedback-manager.h"

#include <json-glib/json-glib.h>

/**
 * FbdFeedbackVibraPeriodic:
 *
 * Describes a periodic feedback via a haptic motor
 *
 * The #FbdVibraVibraPeriodic describes the properties of a haptic feedback
 * event. It knows nothing about the hardware itself but calls
 * #FbdDevVibra for that.
 */

enum {
  PROP_0,
  PROP_MAGNITUDE,
  PROP_FADE_IN_LEVEL,
  PROP_FADE_IN_TIME,
  PROP_LAST_PROP,
};
static GParamSpec *props[PROP_LAST_PROP];

typedef struct _FbdFeedbackVibraPeriodic {
  FbdFeedbackVibra parent;

  double magnitude;
  double fade_in_level;
  guint fade_in_time;
} FbdFeedbackVibraPeriodic;

G_DEFINE_TYPE (FbdFeedbackVibraPeriodic, fbd_feedback_vibra_periodic, FBD_TYPE_FEEDBACK_VIBRA);

static void
fbd_feedback_vibra_periodic_set_property (GObject      *object,
                                          guint         property_id,
                                          const GValue *value,
                                          GParamSpec   *pspec)
{
  FbdFeedbackVibraPeriodic *self = FBD_FEEDBACK_VIBRA_PERIODIC (object);

  switch (property_id) {
  case PROP_MAGNITUDE:
    self->magnitude = g_value_get_double (value);
    break;
  case PROP_FADE_IN_LEVEL:
    self->fade_in_level = g_value_get_double (value);
    break;
  case PROP_FADE_IN_TIME:
    self->fade_in_time = g_value_get_uint (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
fbd_feedback_vibra_periodic_get_property (GObject  *object,
                                          guint       property_id,
                                          GValue     *value,
                                          GParamSpec *pspec)
{
  FbdFeedbackVibraPeriodic *self = FBD_FEEDBACK_VIBRA_PERIODIC (object);

  switch (property_id) {
  case PROP_MAGNITUDE:
    g_value_set_double (value, self->magnitude);
    break;
  case PROP_FADE_IN_LEVEL:
    g_value_set_double (value, self->fade_in_level);
    break;
  case PROP_FADE_IN_TIME:
    g_value_set_uint (value, self->fade_in_time);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
fbd_feedback_vibra_periodic_end_vibra (FbdFeedbackVibra *vibra)
{
  FbdFeedbackManager *manager = fbd_feedback_manager_get_default ();
  FbdDevVibra *dev = fbd_feedback_manager_get_dev_vibra (manager);

  fbd_dev_vibra_stop (dev);
}

static void
fbd_feedback_vibra_periodic_start_vibra (FbdFeedbackVibra *vibra)
{
  FbdFeedbackVibraPeriodic *self = FBD_FEEDBACK_VIBRA_PERIODIC (vibra);
  FbdFeedbackManager *manager = fbd_feedback_manager_get_default ();
  FbdDevVibra *dev = fbd_feedback_manager_get_dev_vibra (manager);
  guint duration = fbd_feedback_vibra_get_duration (vibra);
  double max_strength = fbd_feedback_vibra_get_max_strength (FBD_FEEDBACK_VIBRA (self));
  double fade_in_ratio = self->fade_in_level / self->magnitude;
  double max_magnitude, fade_in_level;

  g_return_if_fail (FBD_IS_DEV_VIBRA (dev));
  max_magnitude = MIN (max_strength, self->magnitude);
  fade_in_level = MIN (max_magnitude * fade_in_ratio, self->fade_in_level);

  g_debug ("Periodic Vibra: (%f,%d) (%f,%d)",
	   max_magnitude, duration, fade_in_level, self->fade_in_time);

  fbd_dev_vibra_periodic (dev, duration, max_magnitude, fade_in_level, self->fade_in_time);
}

static gboolean
fbd_feedback_vibra_periodic_is_available (FbdFeedbackBase *base)
{
  FbdFeedbackManager *manager = fbd_feedback_manager_get_default ();
  FbdDevVibra *dev = fbd_feedback_manager_get_dev_vibra (manager);

  return FBD_IS_DEV_VIBRA (dev);
}

static void
fbd_feedback_vibra_periodic_class_init (FbdFeedbackVibraPeriodicClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  FbdFeedbackBaseClass *base_class = FBD_FEEDBACK_BASE_CLASS (klass);
  FbdFeedbackVibraClass *vibra_class = FBD_FEEDBACK_VIBRA_CLASS (klass);

  object_class->set_property = fbd_feedback_vibra_periodic_set_property;
  object_class->get_property = fbd_feedback_vibra_periodic_get_property;

  base_class->is_available = fbd_feedback_vibra_periodic_is_available;

  vibra_class->start_vibra = fbd_feedback_vibra_periodic_start_vibra;
  vibra_class->end_vibra = fbd_feedback_vibra_periodic_end_vibra;

  props[PROP_MAGNITUDE] =
    g_param_spec_double ("magnitude", "", "",
                         0.0, 1.0, 0.5,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  props[PROP_FADE_IN_TIME] =
    g_param_spec_uint ("fade-in-time", "", "",
                       0, G_MAXINT, 0,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  props[PROP_FADE_IN_LEVEL] =
    g_param_spec_double ("fade-in-level", "", "",
                         0.0, 1.0, 0.5,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, PROP_LAST_PROP, props);
}

static void
fbd_feedback_vibra_periodic_init (FbdFeedbackVibraPeriodic *self)
{
}
