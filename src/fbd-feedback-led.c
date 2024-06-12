/*
 * Copyright (C) 2020 Purism SPC
 *               2024 The Phosh Developers
 * SPDX-License-Identifier: GPL-3.0+
 *
 * Author: Guido Günther <agx@sigxcpu.org>
 */

#define G_LOG_DOMAIN "fbd-feedback-led"

#include "fbd-enums.h"
#include "fbd-dev-leds.h"
#include "fbd-feedback-led.h"
#include "fbd-feedback-manager.h"

/**
 * FbdFeedbackLed:
 *
 * The `FbdFeedbackLed` describes a feedback via an LED. It currently
 * only supports periodic patterns.
 */

enum {
  PROP_0,
  PROP_FREQUENCY,
  PROP_COLOR,
  PROP_MAX_BRIGHTNESS,
  PROP_PRIORITY,
  PROP_LAST_PROP,
};
static GParamSpec *props[PROP_LAST_PROP];

typedef struct _FbdFeedbackLed {
  FbdFeedbackBase  parent;

  guint            frequency;
  guint            priority;
  guint            max_brightness;
  char            *color;
} FbdFeedbackLed;

G_DEFINE_TYPE (FbdFeedbackLed, fbd_feedback_led, FBD_TYPE_FEEDBACK_BASE)


static FbdFeedbackLedColor
color_string_to_color (const char *color)
{

  if (g_strcmp0 (color, "red") == 0)
    return FBD_FEEDBACK_LED_COLOR_RED;
  else if (g_strcmp0 (color, "green") == 0)
    return FBD_FEEDBACK_LED_COLOR_GREEN;
  else if (g_strcmp0 (color, "blue") == 0)
    return FBD_FEEDBACK_LED_COLOR_BLUE;
  else if (g_strcmp0 (color, "white") == 0)
    return FBD_FEEDBACK_LED_COLOR_WHITE;

  g_warning_once ("Can't parse color '%s' using white", color);
  return FBD_FEEDBACK_LED_COLOR_WHITE;
}


static void
fbd_feedback_led_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  FbdFeedbackLed *self = FBD_FEEDBACK_LED (object);

  switch (property_id) {
  case PROP_FREQUENCY:
    self->frequency = g_value_get_uint (value);
    break;
  case PROP_PRIORITY:
    self->priority = g_value_get_uint (value);
    break;
  case PROP_MAX_BRIGHTNESS:
    self->max_brightness = g_value_get_uint (value);
    break;
  case PROP_COLOR:
    self->color = g_value_dup_string (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


static void
fbd_feedback_led_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  FbdFeedbackLed *self = FBD_FEEDBACK_LED (object);

  switch (property_id) {
  case PROP_FREQUENCY:
    g_value_set_uint (value, self->frequency);
    break;
  case PROP_PRIORITY:
    g_value_set_uint (value, self->priority);
    break;
  case PROP_MAX_BRIGHTNESS:
    g_value_set_uint (value, self->max_brightness);
    break;
  case PROP_COLOR:
    g_value_set_string (value, self->color);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


static void
fbd_feedback_led_run (FbdFeedbackBase *base)
{
  FbdFeedbackLed *self = FBD_FEEDBACK_LED (base);
  FbdFeedbackManager *manager = fbd_feedback_manager_get_default ();
  FbdDevLeds *dev = fbd_feedback_manager_get_dev_leds (manager);
  FbdFeedbackLedColor color;

  g_return_if_fail (FBD_IS_DEV_LEDS (dev));
  g_debug ("Periodic led feedback: self->max_brightness, self->frequency");

  color = color_string_to_color (self->color);
  /* FIXME: handle priority */
  fbd_dev_leds_start_periodic (dev,
                               color,
                               self->max_brightness,
                               self->frequency);
}


static void
fbd_feedback_led_end (FbdFeedbackBase *base)
{
  FbdFeedbackLed *self = FBD_FEEDBACK_LED (base);
  FbdFeedbackManager *manager = fbd_feedback_manager_get_default ();
  FbdDevLeds *dev = fbd_feedback_manager_get_dev_leds (manager);
  FbdFeedbackLedColor color;

  color = color_string_to_color (self->color);
  if (dev)
    fbd_dev_leds_stop (dev, color);
  fbd_feedback_base_done (FBD_FEEDBACK_BASE (self));
}


static gboolean
fbd_feedback_led_is_available (FbdFeedbackBase *base)
{
  FbdFeedbackManager *manager = fbd_feedback_manager_get_default ();
  FbdDevLeds *dev = fbd_feedback_manager_get_dev_leds (manager);

  return FBD_IS_DEV_LEDS (dev);
}


static void
fbd_feedback_led_finalize (GObject *object)
{
  FbdFeedbackLed *self = FBD_FEEDBACK_LED (object);

  g_clear_pointer (&self->color, g_free);

  G_OBJECT_CLASS (fbd_feedback_led_parent_class)->finalize (object);
}


static void
fbd_feedback_led_class_init (FbdFeedbackLedClass *klass)
{
  FbdFeedbackBaseClass *base_class = FBD_FEEDBACK_BASE_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = fbd_feedback_led_set_property;
  object_class->get_property = fbd_feedback_led_get_property;
  object_class->finalize = fbd_feedback_led_finalize;

  base_class->run = fbd_feedback_led_run;
  base_class->end = fbd_feedback_led_end;
  base_class->is_available = fbd_feedback_led_is_available;

  /**
   * FbdFeedbackLed:frequency:
   *
   * The frequency the LED should blink with in mHz.
   */
  props[PROP_FREQUENCY] =
    g_param_spec_uint ("frequency", "", "",
                       0, G_MAXUINT, 0,
                       G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  /**
   * FbdFeedbackLed:color:
   *
   * The color the LED should blink with. The color is given as color
   * name or `rgb()` color value.
   */
  props[PROP_COLOR] =
    g_param_spec_string ("color", "", "",
                         NULL,
                         G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  /**
   * FbdFeedbackLed:priority:
   *
   * Priority of the LED pattern. LED devices can only display a limited
   * amount of patterns at a time. In this case the pattern with the highest
   * priority wins.
   */
  props[PROP_PRIORITY] =
    g_param_spec_uint ("priority", "", "",
                       0, 255, 0,
                       G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  /**
   * FbdFeedbackLed:max-brightness:
   *
   * The maximum brightness in the LED pattern in percent of the LEDs
   * maximum brightness.
   */
  props[PROP_MAX_BRIGHTNESS] =
    g_param_spec_uint ("max-brightness", "", "",
                       1, 100, 100,
                       G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, PROP_LAST_PROP, props);
}


static void
fbd_feedback_led_init (FbdFeedbackLed *self)
{
  self->max_brightness = 100;
}
