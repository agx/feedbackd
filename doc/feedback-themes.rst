.. _feedback-themes(5):

===============
feedback themes
===============

---------------------------------
Theme configuration for feedbackd
---------------------------------

DESCRIPTION
===========

In order to reflect device specifics and user overrides the feedback
(such as sound, haptic or led) that are being triggered by events can
be configured. This allows for device as well as user specific
configuration. Themes use a json format and can be validated by
``fbd-theme-validate``.

For details on how to create or modify feedback themes see [feedbackd's documentation](
https://source.puri.sm/Librem5/feedbackd#feedback-theme)

Sound events
============

Sound feedbacks specify an event name from a XDG sound theme. Sound themes
are described in the [Sound theme spec](https://freedesktop.org/wiki/Specifications/sound-theme-spec/).

See also
========

``feedbackd(8)`` ``fbcli(1)`` ``fbd-theme-validate(1)``
