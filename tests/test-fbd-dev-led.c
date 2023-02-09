/*
 * Copyright (C) 2024 The Phosh Developers
 * SPDX-License-Identifier: GPL-3.0+
 *
 * Author: Guido Günther <agx@sigxcpu.org>
 */

#include "fbd-feedback-led.h"
#include "fbd-dev-led.h"
#include "fbd-dev-led-multicolor.h"
#include "fbd-dev-led-qcom.h"

#include "testlib.h"


static void
test_fbd_dev_led_simple (FbdUmockdevFixture *fixture, gconstpointer unused)
{
  GUdevClient *client;
  g_autolist (GUdevDevice) leds = NULL;
  FbdDevLed *led;
  g_autoptr (GError) err = NULL;
  GUdevDevice *dev;

  client = g_udev_client_new ((const char *const []){ "leds", NULL});
  leds = g_udev_client_query_by_subsystem (client, "leds");

  /* There should be a single device only */
  g_assert_cmpint (g_list_length (leds), ==, 1);
  dev = G_UDEV_DEVICE (leds->data);

  g_assert_cmpstr (g_udev_device_get_property (dev, "FEEDBACKD_TYPE"), ==, "led");

  led = fbd_dev_led_new (dev, &err);
  g_assert_no_error (err);
  g_assert_cmpint (fbd_dev_led_get_max_brightness (led), ==, 255);
  g_assert_true (fbd_dev_led_supports_color (led, FBD_FEEDBACK_LED_COLOR_BLUE));
  g_assert_true (fbd_dev_led_start_periodic (led, 50, 50));
  g_assert_finalize_object (led);
}


static void
test_fbd_dev_led_multicolor (FbdUmockdevFixture *fixture, gconstpointer unused)
{
  GUdevClient *client;
  g_autolist (GUdevDevice) leds = NULL;
  FbdDevLed *led;
  g_autoptr (GError) err = NULL;
  GUdevDevice *dev;

  client = g_udev_client_new ((const char *const []){ "leds", NULL});
  leds = g_udev_client_query_by_subsystem (client, "leds");

  /* There should be a single device only */
  g_assert_cmpint (g_list_length (leds), ==, 1);
  dev = G_UDEV_DEVICE (leds->data);

  g_assert_cmpstr (g_udev_device_get_property (dev, "FEEDBACKD_TYPE"), ==, "led");

  led = fbd_dev_led_multicolor_new (dev, &err);
  g_assert_no_error (err);
  g_assert_cmpint (fbd_dev_led_get_max_brightness (led), ==, 248);
  g_assert_true (fbd_dev_led_supports_color (led, FBD_FEEDBACK_LED_COLOR_BLUE));
  g_assert_true (fbd_dev_led_start_periodic (led, 50, 50));

  g_assert_finalize_object (led);
}


static void
test_fbd_dev_led_qcom_simple (FbdUmockdevFixture *fixture, gconstpointer unused)
{
  GUdevClient *client;
  g_autolist (GUdevDevice) leds = NULL;
  FbdDevLed *led;
  g_autoptr (GError) err = NULL;
  GUdevDevice *dev;

  client = g_udev_client_new ((const char *const []){ "leds", NULL});
  leds = g_udev_client_query_by_subsystem (client, "leds");

  /* There should be a single device only */
  g_assert_cmpint (g_list_length (leds), ==, 1);
  dev = G_UDEV_DEVICE (leds->data);

  g_assert_cmpstr (g_udev_device_get_property (dev, "FEEDBACKD_TYPE"), ==, "led");

  led = fbd_dev_led_qcom_new (dev, &err);
  g_assert_no_error (err);
  g_assert_cmpint (fbd_dev_led_get_max_brightness (led), ==, 511);
  g_assert_true (fbd_dev_led_supports_color (led, FBD_FEEDBACK_LED_COLOR_WHITE));
  g_assert_true (fbd_dev_led_start_periodic (led, 50, 50));
  g_assert_finalize_object (led);
}


gint
main (gint argc, gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);

  FBD_UMOCKDEV_TEST_ADD ("/feedbackd/fbd/dev/led/simple",
                         test_fbd_dev_led_simple,
                         "led-simple");
  FBD_UMOCKDEV_TEST_ADD ("/feedbackd/fbd/dev/led/multicolor",
                         test_fbd_dev_led_multicolor,
                         "led-multicolor");
  FBD_UMOCKDEV_TEST_ADD ("/feedbackd/fbd/dev/led/qcom/simple",
                         test_fbd_dev_led_qcom_simple,
                         "led-qcom-simple");

  return g_test_run();
}
