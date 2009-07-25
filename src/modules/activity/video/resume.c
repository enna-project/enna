/*
 * Copyright (C) 2005-2009 The Enna Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software and its Copyright notices. In addition publicly
 * documented acknowledgment must be given that this software has been used if
 * no source code of this software is made available publicly. This includes
 * acknowledgments in either Copyright notices, Manuals, Publicity and
 * Marketing documents or any documentation provided with any product
 * containing this software. This License does not apply to any software that
 * links to the libraries provided by this software (statically or
 * dynamically), but only to the software provided.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "Elementary.h"

#include "enna.h"
#include "enna_config.h"
#include "view_list.h"
#include "popup.h"
#include "buffer.h"
#include "resume.h"
#include "video.h"

#define SMART_NAME "video_resume"

typedef struct _Smart_Data Smart_Data;

struct _Smart_Data
{
    Evas_Coord x, y, w, h;
    Evas_Object *popup;
    Evas_Object *o_edje;
    Evas_Object *list;
};

/* local subsystem globals */
static Evas_Smart *_smart = NULL;

/* local subsystem globals */
static void
_smart_reconfigure (Smart_Data * sd)
{
    Evas_Coord x, y, w, h;

    x = sd->x;
    y = sd->y;
    w = sd->w;
    h = sd->h;

    evas_object_move (sd->popup, x, y);
    evas_object_resize (sd->popup, w, h);
}

static void
cb_playback_resume (void *data)
{
    movie_start_playback (1);
}

static void
cb_playback_begin (void *data)
{
    movie_start_playback (0);
}

static Enna_Vfs_File *
_create_list_item (char *label, char *icon)
{
    Enna_Vfs_File *it;

    it = calloc (1, sizeof (Enna_Vfs_File));
    it->label = (char*)eina_stringshare_add (label);
    it->icon = (char*)eina_stringshare_add (icon);

    return it;
}

static void
_smart_add (Evas_Object * obj)
{
    Smart_Data *sd;
    Enna_Vfs_File *it1, *it2;
    buffer_t *label;

    sd = calloc (1, sizeof (Smart_Data));
    if (!sd)
        return;

    sd->popup = enna_popup_add (evas_object_evas_get (obj));
    sd->o_edje = edje_object_add (evas_object_evas_get (obj));
    edje_object_file_set (sd->o_edje, enna_config_theme_get (), "enna/exit");
    sd->list = enna_list_add (evas_object_evas_get (sd->popup));

    label = buffer_new ();
    buffer_append (label, "<h3><c>");
    buffer_append (label, _("Seems you already watched this movie once ..."));
    buffer_append (label, "</c></h3><br>");

    edje_object_part_text_set (sd->o_edje, "enna.text.label", label->buf);
    buffer_free (label);

    it1 = _create_list_item (_("Resume movie playback"), "ctrl/ok");
    enna_list_file_append (sd->list, it1, cb_playback_resume, NULL);

    it2 = _create_list_item (_("Start playing from beginning"),
                             "ctrl/restart");
    enna_list_file_append (sd->list,it2, cb_playback_begin, NULL);

    evas_object_size_hint_weight_set (sd->list, 1.0, 1.0);
    evas_object_show (sd->list);
    enna_list_select_nth (sd->list, 0);
    edje_object_part_swallow (sd->o_edje, "enna.content.swallow", sd->list);

    enna_popup_content_set (sd->popup, sd->o_edje);
    evas_object_smart_member_add (sd->popup, obj);
    evas_object_smart_data_set (obj, sd);
}

static void
_smart_del (Evas_Object *obj)
{
    INTERNAL_ENTRY;
    evas_object_del (sd->list);
    evas_object_del (sd->o_edje);
    evas_object_del (sd->popup);
    ENNA_FREE (sd);
}

static void
_smart_move (Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
    INTERNAL_ENTRY;

    if ((sd->x == x) && (sd->y == y))
        return;

    sd->x = x;
    sd->y = y;

    _smart_reconfigure (sd);
}

static void
_smart_resize (Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
    INTERNAL_ENTRY;

    if ((sd->w == w) && (sd->h == h))
        return;

    sd->w = w;
    sd->h = h;

    _smart_reconfigure (sd);
}

static void
_smart_show (Evas_Object *obj)
{
    INTERNAL_ENTRY;
    evas_object_show (sd->popup);
}

static void
_smart_hide (Evas_Object *obj)
{
    INTERNAL_ENTRY;
    evas_object_hide (sd->popup);
}

static void
_smart_color_set (Evas_Object *obj, int r, int g, int b, int a)
{
    INTERNAL_ENTRY;
    evas_object_color_set (sd->popup, r, g, b, a);
}

static void
_smart_clip_set (Evas_Object *obj, Evas_Object *clip)
{
    INTERNAL_ENTRY;
    evas_object_clip_set (sd->popup, clip);
}

static void
_smart_clip_unset (Evas_Object *obj)
{
    INTERNAL_ENTRY;
    evas_object_clip_unset (sd->popup);
}

static void
_smart_init (void)
{
    static const Evas_Smart_Class sc = {
        SMART_NAME,
        EVAS_SMART_CLASS_VERSION,
        _smart_add,
        _smart_del,
        _smart_move,
        _smart_resize,
        _smart_show,
        _smart_hide,
        _smart_color_set,
        _smart_clip_set,
        _smart_clip_unset,
        NULL,
        NULL
    };

    if (!_smart)
       _smart = evas_smart_class_new (&sc);
}

/* externally accessible functions */
Evas_Object *
video_resume_add (Evas * evas)
{
    _smart_init ();
    return evas_object_smart_add (evas, _smart);
}

void
video_resume_event_feed (Evas_Object *obj, void *event_info)
{
    API_ENTRY return;
    enna_list_event_feed (sd->list, event_info);
}
