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

#include <string.h>

#include <Edje.h>

#include "enna.h"
#include "enna_config.h"
#include "switcher.h"

#define SMART_NAME "enna_switcher"

typedef struct _Smart_Data Smart_Data;

struct _Smart_Data
{
    Evas_Coord x, y, w, h;
    Evas_Object *obj;
    Evas_Object *o_transition;
    Evas_Object *old_slide;
    Evas_Object *slide;
};

/* local subsystem functions */
static void _edje_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _default_transition(Smart_Data * sd);
static void _smart_reconfigure(Smart_Data * sd);
static void _smart_init(void);
static void _smart_add(Evas_Object * obj);
static void _smart_del(Evas_Object * obj);
static void _smart_move(Evas_Object * obj, Evas_Coord x, Evas_Coord y);
static void _smart_resize(Evas_Object * obj, Evas_Coord w, Evas_Coord h);
static void _smart_show(Evas_Object *obj);
static void _smart_hide(Evas_Object *obj);
static void _smart_color_set(Evas_Object * obj, int r, int g, int b, int a);
static void _smart_clip_set(Evas_Object * obj, Evas_Object * clip);
static void _smart_clip_unset(Evas_Object * obj);

/* local subsystem globals */
static Evas_Smart *_smart = NULL;

/* externally accessible functions */
Evas_Object *
enna_switcher_add(Evas * evas)
{
    _smart_init();
    return evas_object_smart_add(evas, _smart);
}


void
enna_switcher_transition_set(Evas_Object *obj, const char *transition)
{
    API_ENTRY return;
    if (!transition) return;

    sd->o_transition = edje_object_add(evas_object_evas_get(sd->obj));
    edje_object_file_set(sd->o_transition, enna_config_theme_get(), transition);
    edje_object_signal_callback_add(sd->o_transition, "*", "*", _edje_cb, sd);
}

void enna_switcher_objects_switch(Evas_Object *obj, Evas_Object * new_slide)
{
    API_ENTRY return;
    if (!new_slide)
        return;

    if (!sd->o_transition)
        _default_transition(sd);

    edje_object_part_unswallow(sd->o_transition, sd->slide);
    edje_object_part_unswallow(sd->o_transition, sd->old_slide);
    sd->old_slide = sd->slide;
    sd->slide = new_slide;
    edje_object_signal_emit(sd->o_transition, "reset", "enna");
    edje_object_part_swallow(sd->o_transition, "slide.1", sd->old_slide);
    edje_object_part_swallow(sd->o_transition, "slide.2", sd->slide);
    edje_object_signal_emit(sd->o_transition, "show,2", "enna");
}

/* local subsystem globals */

static void _edje_cb(void *data, Evas_Object *obj, const char *emission,
        const char *source)
{

    Smart_Data *sd = (Smart_Data*)data;
    if (!strcmp(emission, "done"))
    {
        edje_object_part_unswallow(sd->o_transition, sd->old_slide);
        evas_object_raise(sd->old_slide);
        evas_object_hide(sd->old_slide);
        sd->old_slide = NULL;
        evas_object_smart_callback_call(sd->obj, "transition_done", NULL);
    }
}

static void _default_transition(Smart_Data * sd)
{
    if (!sd) return;
    if (sd->o_transition)
    {
        edje_object_signal_callback_del(sd->o_transition, "*", "*", _edje_cb);
        evas_object_del(sd->o_transition);
        evas_object_smart_member_add(sd->o_transition, sd->obj);
    }
    sd->o_transition = edje_object_add(evas_object_evas_get(sd->obj));
    edje_object_file_set(sd->o_transition, enna_config_theme_get(),  "transitions/crossfade");
    edje_object_signal_callback_add(sd->o_transition, "*", "*", _edje_cb, sd);
    evas_object_smart_member_add(sd->o_transition, sd->obj);
}



static void _smart_reconfigure(Smart_Data * sd)
{
    Evas_Coord x, y, w, h;

    x = sd->x;
    y = sd->y;
    w = sd->w;
    h = sd->h;

    evas_object_move(sd->o_transition, x, y);
    evas_object_resize(sd->o_transition, w, h);

}

static void _smart_init(void)
{
    if (_smart)
        return;
    static const Evas_Smart_Class sc =
    {
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
            NULL };
    _smart = evas_smart_class_new(&sc);
}

static void _smart_add(Evas_Object * obj)
{
    Smart_Data *sd;

    sd = calloc(1, sizeof(Smart_Data));
    if (!sd)
        return;

    sd->obj = obj;

    evas_object_smart_data_set(obj, sd);
}

static void _smart_del(Evas_Object * obj)
{
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    if (!sd)
        return;
    ENNA_OBJECT_DEL(sd->o_transition);
    free(sd);
}

static void _smart_move(Evas_Object * obj, Evas_Coord x, Evas_Coord y)
{
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    if (!sd)
        return;
    if ((sd->x == x) && (sd->y == y))
        return;
    sd->x = x;
    sd->y = y;
    _smart_reconfigure(sd);
}

static void _smart_resize(Evas_Object * obj, Evas_Coord w, Evas_Coord h)
{
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    if (!sd)
        return;
    if ((sd->w == w) && (sd->h == h))
        return;
    sd->w = w;
    sd->h = h;
    _smart_reconfigure(sd);
}

static void _smart_show(Evas_Object *obj)
{
    INTERNAL_ENTRY;
    evas_object_show(sd->o_transition);
}

static void _smart_hide(Evas_Object *obj)
{
    INTERNAL_ENTRY;
    evas_object_hide(sd->o_transition);
}

static void _smart_color_set(Evas_Object * obj, int r, int g, int b, int a)
{
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    if (!sd)
        return;
    evas_object_color_set(sd->o_transition, r, g, b, a);
}

static void _smart_clip_set(Evas_Object * obj, Evas_Object * clip)
{
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    if (!sd)
        return;
    evas_object_clip_set(sd->o_transition, clip);
}

static void _smart_clip_unset(Evas_Object * obj)
{
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    if (!sd)
        return;

    evas_object_clip_unset(sd->o_transition);
}

