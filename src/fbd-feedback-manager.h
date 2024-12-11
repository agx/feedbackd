/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: GPL-3.0+
 */
#pragma once

#include "fbd-dev-vibra.h"
#include "fbd-dev-leds.h"
#include "fbd-dev-sound.h"
#include "fbd-feedback-profile.h"

#include "lfb-gdbus.h"
#include <glib-object.h>

G_BEGIN_DECLS

#define FBD_TYPE_FEEDBACK_MANAGER (fbd_feedback_manager_get_type())

G_DECLARE_FINAL_TYPE (FbdFeedbackManager, fbd_feedback_manager, FBD, FEEDBACK_MANAGER, LfbGdbusFeedbackSkeleton);

FbdFeedbackManager *fbd_feedback_manager_get_default (void);
FbdDevVibra *fbd_feedback_manager_get_dev_vibra (FbdFeedbackManager *self);
FbdDevSound *fbd_feedback_manager_get_dev_sound (FbdFeedbackManager *self);
FbdDevLeds  *fbd_feedback_manager_get_dev_leds  (FbdFeedbackManager *self);
void         fbd_feedback_manager_load_theme    (FbdFeedbackManager *self);
gboolean     fbd_feedback_manager_set_profile (FbdFeedbackManager *self, const gchar *profile);
FbdFeedbackProfileLevel fbd_feedback_manager_get_effective_level (FbdFeedbackManager      *self,
                                                                  const char              *app_id,
                                                                  FbdFeedbackProfileLevel  want_level,
                                                                  gboolean                 important);

G_END_DECLS
