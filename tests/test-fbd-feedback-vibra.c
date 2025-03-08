/*
 * Copyright (C) 2025 The Phosh Developers
 *
 * SPDX-License-Identifier: GPL-3.0+
 *
 * Author: Guido Günther <agx@sigxcpu.org>
 */

#define GMOBILE_USE_UNSTABLE_API
#include "fbd-feedback-manager.h"
#include "fbd-feedback-vibra-pattern.h"

#include <json-glib/json-glib.h>


static void
test_fbd_feedback_vibra_pattern (void)
{
  /* Create manager upfront so we can dispose it, otherwise creating
   * any feedback would create it implicitly */
  FbdFeedbackManager *manager = fbd_feedback_manager_get_default ();
  g_autoptr (GError) err = NULL;
  g_autoptr (JsonNode) node = NULL;
  g_autoptr (GArray) durations = NULL, magnitudes = NULL;
  GObject *object;
  guint *duration;
  double *magnitude;

  node = json_from_string("{"
                          " \"event-name\" : \"button-pressed\","
                          " \"type\"       : \"VibraPattern\","
                          " \"magnitudes\" : [ 0.2, 1.0 ],"
                          " \"durations\"  : [ 7, 3 ]"
                          "}", &err);
  g_assert_no_error (err);

  object = json_gobject_deserialize (FBD_TYPE_FEEDBACK_VIBRA_PATTERN, node);
  g_object_get (object, "durations", &durations, "magnitudes", &magnitudes, NULL);

  g_assert_nonnull(durations);
  g_assert_nonnull(magnitudes);

  duration = &g_array_index (durations, guint, 0);
  g_assert_cmpint (*duration, ==, 7);
  duration = &g_array_index (durations, guint, 1);
  g_assert_cmpint (*duration, ==, 3);

  magnitude = &g_array_index (magnitudes, double, 0);
  g_assert_cmpfloat (*magnitude, ==, 0.2);
  magnitude = &g_array_index (magnitudes, double, 1);
  g_assert_cmpfloat (*magnitude, ==, 1.0);

  g_assert_finalize_object (object);
  g_assert_finalize_object (manager);
}


gint
main (gint argc, gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func("/feedbackd/fbd/feedback-vibra/pattern", test_fbd_feedback_vibra_pattern);

  return g_test_run();
}
