/*
 * slideshow.c
 * Copyright (C) Nicolas Aguirre 2006,2007,2008 <aguirre.nicolas@gmail.com>
 *
 * slideshow.c is free software copyrighted by Nicolas Aguirre.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name ``Nicolas Aguirre'' nor the name of any other
 *    contributor may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * slideshow.c IS PROVIDED BY Nicolas Aguirre ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Nicolas Aguirre OR ANY OTHER CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* derived from e_icon */

#include "slideshow.h"

#define SMART_NAME "slideshow"

#define API_ENTRY \
   E_Smart_Data *sd; \
   sd = evas_object_smart_data_get(obj); \
   if ((!obj) || (!sd) || \
     (evas_object_type_get(obj) && \
     strcmp(evas_object_type_get(obj), SMART_NAME)))

#define INTERNAL_ENTRY \
   E_Smart_Data *sd; \
   sd = evas_object_smart_data_get(obj); \
   if (!sd) \
      return;


typedef struct _E_Smart_Data E_Smart_Data;

struct _E_Smart_Data
{
   Evas_Coord          x, y, w, h;
   Evas_Object        *o_edje;
   Evas_Object        *obj;
   Evas_List          *playlist;
   unsigned int        playlist_id;
   Ecore_Timer        *timer;
};

/* local subsystem functions */
static void         _enna_slideshow_smart_reconfigure(E_Smart_Data * sd);
static void         _enna_slideshow_smart_init(void);
static void         _switch_images(E_Smart_Data * sd, Evas_Object * obj1, Evas_Object * obj2);
static void         _e_smart_add(Evas_Object * obj);
static void         _e_smart_del(Evas_Object * obj);
static void         _e_smart_move(Evas_Object * obj, Evas_Coord x,
				  Evas_Coord y);
static void         _e_smart_resize(Evas_Object * obj, Evas_Coord w,
				    Evas_Coord h);
static void         _e_smart_show(Evas_Object * obj);
static void         _e_smart_hide(Evas_Object * obj);
static void         _e_smart_color_set(Evas_Object * obj, int r, int g,
				       int b, int a);
static void         _e_smart_clip_set(Evas_Object * obj, Evas_Object * clip);
static void         _e_smart_clip_unset(Evas_Object * obj);

/* local subsystem globals */
static Evas_Smart  *_e_smart = NULL;

/* externally accessible functions */
EAPI Evas_Object   *
enna_slideshow_add(Evas * evas)
{
   _enna_slideshow_smart_init();
   return evas_object_smart_add(evas, _e_smart);
}

EAPI void
enna_slideshow_image_append(Evas_Object *obj, const char *filename)
{
   Evas_Object *o;
   Evas_Coord w,h;

   API_ENTRY return;

   if (!filename) return;

   o = enna_image_add(evas_object_evas_get(obj));
   enna_image_file_set(o, filename+7);
   enna_image_size_get(o, &w, &h);
   enna_image_load_size_set(o, w, h);

   sd->playlist = evas_list_append(sd->playlist, o);

}

EAPI int
enna_slideshow_next(Evas_Object * obj)
{
   Evas_Object *o, *po;

   API_ENTRY return 0;

   po = evas_list_nth(sd->playlist, sd->playlist_id);
   sd->playlist_id++;
   o = evas_list_nth(sd->playlist, sd->playlist_id);

   if (o)
     {
	_switch_images(sd, po, o);
	return 1;
     }
   else
     {
	sd->playlist_id--;
	return 0;
     }
   return 0;
}

EAPI void
enna_slideshow_play(Evas_Object * obj)
{
   Evas_Object        *o;

   API_ENTRY return;

   if (!sd->timer)
     {
	/* Play */
	o = evas_list_nth(sd->playlist, sd->playlist_id);
	_switch_images(sd, NULL, o);
	sd->timer = ecore_timer_add(5, enna_slideshow_next, sd->obj);
     }
   else
     {
	/* Pause */
	ecore_timer_del(sd->timer);
	sd->timer = NULL;
     }

}

/* local subsystem globals */

static void
_switch_images(E_Smart_Data * sd, Evas_Object * obj1, Evas_Object * obj2)
{
   if (!sd || (!obj1 && !obj2))
      return;

   if ((obj1 && obj2))
     {
	edje_object_part_unswallow(sd->o_edje, obj1);
	edje_object_part_swallow(sd->o_edje, "enna.switcher.swallow2", obj1);
	edje_object_signal_emit(sd->o_edje, "enna,swallow2,default,now", "enna");
	edje_object_signal_emit(sd->o_edje, "enna,swallow1,state1,now", "enna");
	edje_object_part_swallow(sd->o_edje, "enna.switcher.swallow1", obj2);
	edje_object_signal_emit(sd->o_edje, "enna,swallow1,default", "enna");
	edje_object_signal_emit(sd->o_edje, "enna,swallow2,state2", "enna");
	evas_object_hide(obj1);
     }
   else if (!obj2)
     {
	edje_object_part_unswallow(sd->o_edje, obj1);
	edje_object_part_swallow(sd->o_edje, "enna.switcher.swallow2", obj1);
	edje_object_signal_emit(sd->o_edje, "enna,swallow2,default,now", "enna");
	edje_object_signal_emit(sd->o_edje, "enna,swallow2,state2", "enna");
     }
   else if (!obj1)
     {
	edje_object_signal_emit(sd->o_edje, "enna,swallow1,state1,now", "enna");
	edje_object_part_swallow(sd->o_edje, "enna.switcher.swallow1", obj2);
	edje_object_signal_emit(sd->o_edje, "enna,swallow1,default", "enna");
	evas_object_show(obj2);
     }
}

static void
_enna_slideshow_smart_reconfigure(E_Smart_Data * sd)
{
   Evas_Coord          x, y, w, h;

   x = sd->x;
   y = sd->y;
   w = sd->w;
   h = sd->h;

   evas_object_move(sd->o_edje, x, y);
   evas_object_resize(sd->o_edje, w, h);

}

static void
_enna_slideshow_smart_init(void)
{
   if (_e_smart)
      return;
   static const Evas_Smart_Class sc = {
      SMART_NAME,
      EVAS_SMART_CLASS_VERSION,
      _e_smart_add,
      _e_smart_del,
      _e_smart_move,
      _e_smart_resize,
      _e_smart_show,
      _e_smart_hide,
      _e_smart_color_set,
      _e_smart_clip_set,
      _e_smart_clip_unset,
      NULL
   };
   _e_smart = evas_smart_class_new(&sc);
}

static void
_e_smart_add(Evas_Object * obj)
{
   E_Smart_Data       *sd;

   sd = calloc(1, sizeof(E_Smart_Data));
   if (!sd)
      return;
   sd->o_edje = edje_object_add(evas_object_evas_get(obj));
   edje_object_file_set(sd->o_edje, enna_config_theme_get(), "enna/slideshow");
   sd->x = 0;
   sd->y = 0;
   sd->w = 0;

   sd->obj = obj;

   sd->h = 0;
   evas_object_smart_member_add(sd->o_edje, obj);
   evas_object_smart_data_set(obj, sd);


   sd->playlist = NULL;
   sd->playlist_id = 0;
   sd->timer = NULL;
}

static void
_e_smart_del(Evas_Object * obj)
{
   E_Smart_Data       *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd)
      return;
   evas_object_del(sd->o_edje);
   free(sd);
}

static void
_e_smart_move(Evas_Object * obj, Evas_Coord x, Evas_Coord y)
{
   E_Smart_Data       *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd)
      return;
   if ((sd->x == x) && (sd->y == y))
      return;
   sd->x = x;
   sd->y = y;
   _enna_slideshow_smart_reconfigure(sd);
}

static void
_e_smart_resize(Evas_Object * obj, Evas_Coord w, Evas_Coord h)
{
   E_Smart_Data       *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd)
      return;
   if ((sd->w == w) && (sd->h == h))
      return;
   sd->w = w;
   sd->h = h;
   _enna_slideshow_smart_reconfigure(sd);
}

static void
_e_smart_show(Evas_Object * obj)
{
   E_Smart_Data       *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd)
      return;
   evas_object_show(sd->o_edje);
}

static void
_e_smart_hide(Evas_Object * obj)
{
   E_Smart_Data       *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd)
      return;
   evas_object_hide(sd->o_edje);
}

static void
_e_smart_color_set(Evas_Object * obj, int r, int g, int b, int a)
{
   E_Smart_Data       *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd)
      return;
   evas_object_color_set(sd->o_edje, r, g, b, a);
}

static void
_e_smart_clip_set(Evas_Object * obj, Evas_Object * clip)
{
   E_Smart_Data       *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd)
      return;
   evas_object_clip_set(sd->o_edje, clip);
}

static void
_e_smart_clip_unset(Evas_Object * obj)
{
   E_Smart_Data       *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd)
      return;
   evas_object_clip_unset(sd->o_edje);
}
