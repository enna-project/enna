/*
 * GeeXboX Enna Media Center.
 * Copyright (C) 2005-2010 The Enna Project
 *
 * This file is part of Enna.
 *
 * Enna is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Enna is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Enna; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Smart Callback event
 *
 * "root" this event is sent when root is browse
 * "selected" this event is sent when a file or a directory is selected
 * "browse_down" this event is sent when browse down is detected
 *
 */

#include <string.h>

#include <Elementary.h>
#include <Edje.h>

#include "enna.h"
#include "enna_config.h"
#include "browser.h"
#include "view_cover.h"
#include "view_list.h"
#include "view_wall.h"
#include "image.h"
#include "logs.h"
#include "input.h"
#include "volumes.h"

#define SMART_NAME "Enna_Browser"

typedef struct _Smart_Data Smart_Data;
typedef struct _Browse_Data Browse_Data;

struct _Browse_Data
{
    Enna_Vfs_File *file;
    Smart_Data *sd;
};

struct _Smart_Data
{
    Evas_Coord x, y, w, h;
    Evas_Object *obj;
    Evas_Object *layout;
    Evas_Object *o_view;
    Evas_Object *o_letter;
    Evas_Object *o_parent;
    Evas_Object *o_parent_box;
    Eina_List *files;
    Enna_Class_Vfs *vfs;
    Enna_Vfs_File *file;
    Eina_List *visited;
    unsigned int letter_mode;
    Ecore_Timer *letter_timer;
    Ecore_Timer *hilight_timer;
    unsigned int letter_event_nbr;
    char letter_key;
    struct
    {
        Evas_Object *(*view_add)(Smart_Data *sd);
        void (*view_append)(Evas_Object *view,
                            Enna_Vfs_File *file,
                            void (*func_activated)(void *data),
                            void *data);
        void *(*view_selected_data_get)(Evas_Object *view);
        int (*view_jump_label)(Evas_Object *view, const char *label);
        Eina_Bool (*view_key_down)(Evas_Object *view, enna_input event);
        void (*view_select_nth)(Evas_Object *obj, int nth);
        Eina_List *(*view_files_get)(Evas_Object *obj);
        void (*view_jump_ascii)(Evas_Object *obj, char k);
    } view_funcs;
    Enna_Volumes_Listener *vl;
    unsigned char accept_ev : 1;
    unsigned char show_file : 1;
};



/* local subsystem functions */
static void _list_transition_right_end_cb(void *data, Evas_Object *o, const char *sig, const char *src);
static void _list_transition_left_end_cb(void *data, Evas_Object *o, const char *sig, const char *src);
static void _view_hilight_cb (void *data, Evas_Object *obj, void *event_info);

static void _smart_reconfigure(Smart_Data *sd);
static void _smart_init(void);
static void _smart_add(Evas_Object *obj);
static void _smart_del(Evas_Object *obj);
static void _smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void _smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
static void _smart_show(Evas_Object *obj);
static void _smart_hide(Evas_Object *obj);
static void _smart_color_set(Evas_Object *obj, int r, int g, int b, int a);
static void _smart_clip_set(Evas_Object *obj, Evas_Object * clip);
static void _smart_clip_unset(Evas_Object *obj);
static void _browse(void *data);
static void _browse_down(void *data);

/* local subsystem globals */
static Evas_Smart *_smart = NULL;

/* Browser View */

static void
_refresh_browser(void *data, Enna_Volume *volume)
{
    Smart_Data *sd = data;
    Browse_Data* bd;

    bd = ENNA_NEW(Browse_Data, 1);
    bd->sd = sd;
    bd->file = sd->file;
    _browse(bd);
}

static Evas_Object *
_browser_view_list_add(Smart_Data *sd)
{
    Evas_Object *view;

    if (!sd) return NULL;

    view = enna_list_add(enna->evas);

    elm_layout_content_set(sd->layout, "enna.swallow.content", view);
    evas_object_smart_callback_add(view, "hilight", _view_hilight_cb, sd);
    /* View */
    edje_object_signal_emit(view, "list,right,now", "enna");
    evas_object_smart_callback_add(view, "hilight", _view_hilight_cb, sd);
    return view;
}


static Evas_Object *
_browser_view_cover_add(Smart_Data *sd)
{
    Evas_Object *view;

    if (!sd) return NULL;

    view = enna_view_cover_add(enna->evas, 1);

    elm_layout_content_set(sd->layout, "enna.swallow.content", view);
    evas_object_smart_callback_add(view, "hilight", _view_hilight_cb, sd);
    return view;
}

static Evas_Object *
_browser_view_wall_add(Smart_Data *sd)
{
    Evas_Object *view;

    if (!sd) return NULL;

    view = enna_wall_add(enna->evas);

    elm_layout_content_set(sd->layout, "enna.swallow.content", view);
    evas_object_smart_callback_add(view, "hilight", _view_hilight_cb, sd);
    return view;
}

static void
_browser_view_wall_select_nth(Evas_Object *view, int nth)
{
    enna_wall_select_nth(view, nth, 0);
}

static void
_parent_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    _browse_down(data);
}

/* externally accessible functions */
Evas_Object *
enna_browser_add(Evas * evas)
{
    _smart_init();
    return evas_object_smart_add(evas, _smart);
}

static int
_view_delay_hilight_cb(void *data)
{
    Browse_Data *bd = data;
    Browser_Selected_File_Data *ev;

    ev = calloc(1, sizeof(Browser_Selected_File_Data));
    ev->file = bd->file;
    ev->files = NULL;

    bd->sd->hilight_timer = NULL;
    evas_object_smart_callback_call (bd->sd->obj, "delay,hilight", ev);

    return 0;
}

static void
_view_hilight_cb (void *data, Evas_Object *obj, void *event_info)
{
    Smart_Data *sd = data;
    Browser_Selected_File_Data *ev;
    Browse_Data *bd = event_info;

    if (!sd || !bd) return;

    ev = calloc(1, sizeof(Browser_Selected_File_Data));
    ev->file = bd->file;
    ev->files = NULL;

    evas_object_smart_callback_call (sd->obj, "hilight", ev);
    ENNA_TIMER_DEL(sd->hilight_timer);
    sd->hilight_timer = ecore_timer_add(0.2, _view_delay_hilight_cb, bd);
}

void
enna_browser_view_add(Evas_Object *obj, Enna_Browser_View_Type view_type)
{
    API_ENTRY return;

    switch(view_type)
    {
    case ENNA_BROWSER_VIEW_LIST:
        sd->view_funcs.view_add = _browser_view_list_add;
        sd->view_funcs.view_append = enna_list_file_append;
        sd->view_funcs.view_selected_data_get = enna_list_selected_data_get;
        sd->view_funcs.view_jump_label = enna_list_jump_label;
        sd->view_funcs.view_key_down = enna_list_input_feed;
        sd->view_funcs.view_select_nth = enna_list_select_nth;
        sd->view_funcs.view_files_get = enna_list_files_get;
        sd->view_funcs.view_jump_ascii = enna_list_jump_ascii;
        break;
    case ENNA_BROWSER_VIEW_COVER:
        sd->view_funcs.view_add = _browser_view_cover_add;
        sd->view_funcs.view_append = enna_view_cover_file_append;
        sd->view_funcs.view_selected_data_get = enna_view_cover_selected_data_get;
        sd->view_funcs.view_jump_label = enna_view_cover_jump_label;
        sd->view_funcs.view_key_down = enna_view_cover_input_feed;
        sd->view_funcs.view_select_nth = enna_view_cover_select_nth;
        sd->view_funcs.view_files_get = enna_view_cover_files_get;
        sd->view_funcs.view_jump_ascii = enna_view_cover_jump_ascii;
        break;
    case ENNA_BROWSER_VIEW_WALL:
        sd->view_funcs.view_add = _browser_view_wall_add;
        sd->view_funcs.view_append = enna_wall_file_append;
        sd->view_funcs.view_selected_data_get = enna_wall_selected_data_get;
        sd->view_funcs.view_jump_label = enna_wall_jump_label;
        sd->view_funcs.view_key_down = enna_wall_input_feed;
        sd->view_funcs.view_select_nth = _browser_view_wall_select_nth;
        sd->view_funcs.view_files_get = enna_wall_files_get;
        sd->view_funcs.view_jump_ascii = enna_wall_jump_ascii;
    default:
        break;
    }
    evas_object_smart_callback_del(sd->o_view, "hilight", _view_hilight_cb);
    ENNA_OBJECT_DEL(sd->o_view);
    sd->o_view = sd->view_funcs.view_add(sd);

}

void
enna_browser_show_file_set(Evas_Object *obj, unsigned char show)
{
    API_ENTRY return;

    sd->show_file = show;
}

int
enna_browser_select_label(Evas_Object *obj, const char *label)
{

    API_ENTRY return -1;

    if (!sd || !sd->o_view) return -1;

    if (sd->view_funcs.view_jump_label)
        sd->view_funcs.view_jump_label(sd->o_view, label);

    return 0;

}

Eina_List *
enna_browser_files_get(Evas_Object *obj)
{
     API_ENTRY return NULL;
     if (!sd->o_view) return NULL;

     if (sd->view_funcs.view_files_get)
         return sd->view_funcs.view_files_get(sd->o_view);

     return NULL;
}

/* local subsystem globals */
static void
_smart_reconfigure(Smart_Data * sd)
{
    Evas_Coord x, y, w, h;

    x = sd->x;
    y = sd->y;
    w = sd->w;
    h = sd->h;

    evas_object_move(sd->layout, x, y);
    evas_object_resize(sd->layout, w, h);
}

/* Class Item interface */

static void
_smart_init(void)
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
        NULL
    };
    _smart = evas_smart_class_new(&sc);
}



static void
_smart_add(Evas_Object * obj)
{
    Smart_Data *sd;

    sd = calloc(1, sizeof(Smart_Data));
    if (!sd)
        return;

    sd->layout = elm_layout_add(enna->layout);
    elm_layout_file_set(sd->layout, enna_config_theme_get(), "enna/browser");

    sd->view_funcs.view_add = _browser_view_list_add;
    sd->view_funcs.view_append =  enna_list_file_append;
    sd->view_funcs.view_selected_data_get =  enna_list_selected_data_get;
    sd->view_funcs.view_jump_label =  enna_list_jump_label;
    sd->view_funcs.view_key_down = enna_list_input_feed;
    sd->view_funcs.view_select_nth = enna_list_select_nth;

    sd->o_view = sd->view_funcs.view_add(sd);
    evas_object_smart_callback_add(sd->o_view, "hilight", _view_hilight_cb, sd);

    sd->o_parent_box = elm_box_add(enna->layout);
    elm_layout_content_set(sd->layout, "enna.swallow.parent", sd->o_parent_box);

    edje_object_signal_emit(elm_layout_edje_get(sd->layout), "letter,hide", "enna");
    sd->o_letter =  elm_button_add(obj);
    elm_button_label_set(sd->o_letter, "");
    elm_object_scale_set(sd->o_letter, 6.0);
    evas_object_show(sd->o_letter);
    elm_layout_content_set(sd->layout, "enna.swallow.letter", sd->o_letter);

    sd->accept_ev = 1;
    sd->show_file = 1;
    evas_object_smart_member_add(sd->layout, obj);
    sd->obj = obj;
    evas_object_smart_data_set(obj, sd);
    sd->vl = enna_volumes_listener_add("browser", _refresh_browser, _refresh_browser, sd);
}

static void
_smart_del(Evas_Object * obj)
{
    INTERNAL_ENTRY;
    edje_object_signal_callback_del(elm_layout_edje_get(sd->layout),
                                    "list,transition,end", "edje", _list_transition_right_end_cb);
    edje_object_signal_callback_del(elm_layout_edje_get(sd->layout),
                                    "list,transition,end", "edje", _list_transition_left_end_cb);
    ENNA_OBJECT_DEL(sd->o_view);
    ENNA_OBJECT_DEL(sd->o_parent_box);
    evas_object_del(sd->layout);
    evas_object_del(sd->o_letter);
    enna_volumes_listener_del(sd->vl);
    ENNA_TIMER_DEL(sd->hilight_timer);
    free(sd);
}

static void
_smart_move(Evas_Object * obj, Evas_Coord x, Evas_Coord y)
{
    INTERNAL_ENTRY;

    if ((sd->x == x) && (sd->y == y))
        return;
    sd->x = x;
    sd->y = y;
    _smart_reconfigure(sd);
}

static void
_smart_resize(Evas_Object * obj, Evas_Coord w, Evas_Coord h)
{
    INTERNAL_ENTRY;

    if ((sd->w == w) && (sd->h == h))
        return;
    sd->w = w;
    sd->h = h;
    _smart_reconfigure(sd);
}

static void
_smart_show(Evas_Object * obj)
{
    INTERNAL_ENTRY;
    evas_object_show(sd->layout);
}

static void
_smart_hide(Evas_Object * obj)
{
    INTERNAL_ENTRY;
    evas_object_hide(sd->layout);
}

static void
_smart_color_set(Evas_Object * obj, int r, int g, int b, int a)
{
    INTERNAL_ENTRY;
    evas_object_color_set(sd->layout, r, g, b, a);
}

static void
_smart_clip_set(Evas_Object * obj, Evas_Object * clip)
{
    INTERNAL_ENTRY;
    evas_object_clip_set(sd->layout, clip);
}

static void
_smart_clip_unset(Evas_Object * obj)
{
    INTERNAL_ENTRY;
    evas_object_clip_unset(sd->layout);
}

static void
_list_transition_default_up_end_cb(void *data, Evas_Object *o, const char *sig, const char *src)
{
    Smart_Data *sd = data;
    if (!data) return;

    sd->accept_ev = 1;

    sd->view_funcs.view_select_nth(sd->o_view, 0);
    edje_object_signal_callback_del(elm_layout_edje_get(sd->layout),
                                    "list,transition,default,end", "edje",
                                    _list_transition_default_up_end_cb);
}

static void
_list_transition_default_down_end_cb(void *data, Evas_Object *o, const char *sig, const char *src)
{
    Smart_Data *sd = data;
    Enna_Vfs_File *last;
    int selected = -1;
    if (!data) return;

    sd->accept_ev = 1;

    last = eina_list_nth(sd->visited, eina_list_count(sd->visited) - 1);

    /* Remove last entry in visited files*/
    sd->visited = eina_list_remove_list(sd->visited, eina_list_last(sd->visited));



    if (last && last->label)
    {
        selected = sd->view_funcs.view_jump_label(sd->o_view, last->label);
        if (selected == -1)
            sd->view_funcs.view_select_nth(sd->o_view, 0);
    }
    else
        sd->view_funcs.view_select_nth(sd->o_view, 0);

    edje_object_signal_callback_del(elm_layout_edje_get(sd->layout),
                                    "list,transition,default,end", "edje",
                                    _list_transition_default_down_end_cb);
}

static void
_browse(void *data)
{
    Smart_Data *sd;
    Browse_Data *bd = data;
    Enna_Vfs_File *visited;

    if (!bd)
        return;

    sd = bd->sd;
    sd->file = bd->file;

    if (!sd || !sd->vfs)
        return;

    /* FIXME : List / Gen List */
    sd->accept_ev = 0;
    if (sd->vfs->func.class_browse_up)
    {
        Browser_Selected_File_Data *ev = calloc(1, sizeof(Browser_Selected_File_Data));
        ev->vfs = sd->vfs;
        ev->file = sd->file;

        if (sd->file && (sd->file->is_directory || sd->file->is_menu))
        {
            /* File selected is a directory */
            sd->files = sd->vfs->func.class_browse_up(sd->file->uri, sd->vfs->cookie);
            /* No media found */
           if (!eina_list_count(sd->files))
            {

                sd->file = enna_vfs_create_directory(sd->file->uri, _("No media found!"), "icon/nofile", NULL);
                sd->files = NULL;
                sd->files = eina_list_append(sd->files,sd->file);

            }
            else
            {
                ev->file = sd->file;
                ev->files = sd->files;
                evas_object_smart_callback_call (sd->obj, "selected", ev);
            }
        }
        else if (sd->show_file)
        {
            /* File selected is a regular file */
            ev->files = sd->files;
            sd->accept_ev = 1;
            evas_object_smart_callback_call (sd->obj, "selected", ev);
            return;
        }

        /* Add last selected file in visited list */
        visited = calloc(1, sizeof(Enna_Vfs_File));
        visited->label = strdup(sd->file->label);
        visited->uri = strdup(sd->file->uri);
        sd->visited = eina_list_append(sd->visited, visited);

        /* Clear list and add new items */
        edje_object_signal_callback_add(elm_layout_edje_get(sd->layout),
                                        "list,transition,end", "edje",
                                        _list_transition_left_end_cb, sd);
        edje_object_signal_emit(elm_layout_edje_get(sd->layout),
                                "list,left", "enna");
    }
}

static void
_browse_down(void *data)
{
    Smart_Data *sd = data;

    if (!sd) return;

    sd->accept_ev = 0;

    if (sd->vfs && sd->vfs->func.class_browse_down)
    {
        sd->files = sd->vfs->func.class_browse_down(sd->vfs->cookie);
        sd->file = sd->vfs->func.class_vfs_get(sd->vfs->cookie);
        if (!sd->files)
        {
            evas_object_smart_callback_call (sd->obj, "root", NULL);
            return;
        }

        /* Clear list and add new items */
        edje_object_signal_callback_add(elm_layout_edje_get(sd->layout),
                                        "list,transition,end", "edje",
                                        _list_transition_right_end_cb, sd);
        edje_object_signal_emit(elm_layout_edje_get(sd->layout),
                                "list,right", "enna");

    }
}

static void
_list_transition_core(Smart_Data *sd, unsigned char direction)
{
    Evas_Object *ic;
    Eina_List *l;
    Eina_List *files = sd->files;

    if (!files)
      return;

    if (!direction)
    {
        edje_object_signal_callback_del(elm_layout_edje_get(sd->layout),
                                        "list,transition,end", "edje",
                                        _list_transition_left_end_cb);
        edje_object_signal_callback_add(elm_layout_edje_get(sd->layout),
                                        "list,transition,default,end", "edje",
                                        _list_transition_default_up_end_cb, sd);
    }
    else
    {
        edje_object_signal_callback_del(elm_layout_edje_get(sd->layout),
                                        "list,transition,end", "edje",
                                        _list_transition_right_end_cb);
        edje_object_signal_callback_add(elm_layout_edje_get(sd->layout),
                                        "list,transition,default,end", "edje",
                                        _list_transition_default_down_end_cb, sd);
    }


    ENNA_OBJECT_DEL(sd->o_view);
    sd->o_view = sd->view_funcs.view_add(sd);

    if (direction == 0)
    {

        edje_object_signal_emit(elm_layout_edje_get(sd->layout),
                                "list,right,now", "enna");
    }
    else
    {
        evas_object_smart_callback_call (sd->obj, "browse_down", NULL);
        edje_object_signal_emit(elm_layout_edje_get(sd->layout),
                                "list,left,now", "enna");
    }

    if (eina_list_count(files))
    {
        int i = 0;

        /* Create list of files */
        for (l = files, i = 0; l; l = l->next, i++)
        {
            Enna_Vfs_File *f;
            Evas_Object *icon = NULL;
            Browse_Data *bd;

            f = l->data;

            if ((!f->is_directory || !f->is_menu) && !sd->show_file)
                continue;

            if (f->icon_file && f->icon_file[0] == '/')
            {
                icon = enna_image_add(enna->evas);
                enna_image_file_set(icon, f->icon_file, NULL);
            }
            else
            {
                icon = edje_object_add(enna->evas);
                edje_object_file_set(icon, enna_config_theme_get(), f->icon);
            }

            bd = calloc(1, sizeof(Browse_Data));
            bd->file = f;
            bd->sd = sd;
            sd->view_funcs.view_append(sd->o_view, f, _browse, bd);
        }

    }
    else
    {
        /* Browse down and no file detected : Root */
        sd->vfs = NULL;

    }

    /* Change parent */
    ENNA_OBJECT_DEL(sd->o_parent);
    sd->o_parent = elm_button_add(enna->layout);
    elm_object_style_set(sd->o_parent, "simple");
    evas_object_smart_callback_add(sd->o_parent, "clicked", _parent_clicked_cb, sd);
    elm_button_label_set(sd->o_parent, sd->file ? sd->file->label : _("Main menu"));
    ic = elm_icon_add(enna->layout);
    if (!sd->file)
        elm_icon_file_set(ic, enna_config_theme_get(), "icon/home");
    else if (sd->file->is_directory)
        elm_icon_file_set(ic, enna_config_theme_get(), "icon/directory");
    else if (sd->file->icon)
        elm_icon_file_set(ic, enna_config_theme_get(), sd->file->icon);
    evas_object_show(ic);
    elm_button_icon_set(sd->o_parent, ic);
    evas_object_size_hint_align_set(sd->o_parent, 0.5, 0.5);
    elm_box_pack_start(sd->o_parent_box, sd->o_parent);
    evas_object_show(sd->o_parent);

    edje_object_signal_emit(elm_layout_edje_get(sd->layout),
                            "list,default", "enna");
    //sd->accept_ev = 1;
}

static void
_list_transition_left_end_cb(void *data, Evas_Object *o, const char *sig, const char *src)
{
    _list_transition_core(data, 0);
}

static void
_list_transition_right_end_cb(void *data, Evas_Object *o, const char *sig, const char *src)
{
    _list_transition_core(data, 1);
}

static int
_letter_timer_cb(void *data)
{
    Smart_Data *sd;

    sd = data;
    if (!sd) return 0;

    edje_object_signal_emit(elm_layout_edje_get(sd->layout),
                            "letter,hide", "enna");
    sd->letter_mode = 0;
    ENNA_TIMER_DEL(sd->letter_timer);
    return ECORE_CALLBACK_CANCEL;
}

void
enna_browser_root_set(Evas_Object *obj, Enna_Class_Vfs *vfs)
{
    API_ENTRY return;

    if (!vfs) return;

    if (vfs->func.class_browse_up)
    {
        /* create Root menu */
        sd->files = vfs->func.class_browse_up(NULL, vfs->cookie);
        sd->vfs = vfs;
        edje_object_signal_callback_add(elm_layout_edje_get(sd->layout),
                                        "list,transition,end", "edje",
                                        _list_transition_left_end_cb, sd);
        edje_object_signal_emit(elm_layout_edje_get(sd->layout),
                                "list,left", "enna");
    }
}

static void
_jump_to_ascii(Smart_Data *sd, char k)
{
    if (!sd) return;

     if (sd->view_funcs.view_jump_ascii)
            sd->view_funcs.view_jump_ascii(sd->o_view, k);
/*
    EINA_LIST_FOREACH(sd->items, l, it)
    {
        if (it->label[0] == k || it->label[0] == k - 32)
        {
            _smart_select_item(sd, i);
            return;
        }
        i++;
    }*/
}

void
_browser_letter_show(Smart_Data *sd, const char *letter)
{
    ENNA_TIMER_DEL(sd->letter_timer);

    elm_button_label_set(sd->o_letter, letter);
    edje_object_part_text_set(elm_layout_edje_get(sd->layout),
                              "enna.text.letter", letter);
    edje_object_signal_emit(elm_layout_edje_get(sd->layout),
                            "letter,show", "enna");

    sd->letter_timer = ecore_timer_add(1.5, _letter_timer_cb, sd);

    _jump_to_ascii(sd, letter[0]);
}

void
enna_browser_input_feed(Evas_Object *obj, enna_input event)
{
    API_ENTRY return;

    if (!sd->accept_ev) return;

    edje_object_signal_callback_del(elm_layout_edje_get(sd->layout),
                                    "list,transition,end", "edje",
                                    _list_transition_left_end_cb);
    edje_object_signal_callback_del(elm_layout_edje_get(sd->layout),
                                    "list,transition,end", "edje",
                                    _list_transition_right_end_cb);

    switch (event)
    {
    case ENNA_INPUT_EXIT:
        _browse_down(sd);
        break;
    case ENNA_INPUT_OK:
    {
        /* FIXME */
        _browse(sd->view_funcs.view_selected_data_get(sd->o_view));
        break;
    }
    case ENNA_INPUT_UP:
    case ENNA_INPUT_DOWN:
    case ENNA_INPUT_RIGHT:
    case ENNA_INPUT_LEFT:
    case ENNA_INPUT_NEXT:
    case ENNA_INPUT_PREV:
    case ENNA_INPUT_FIRST:
    case ENNA_INPUT_LAST:
        if (sd->view_funcs.view_key_down)
            sd->view_funcs.view_key_down(sd->o_view, event);
        break;

    case ENNA_INPUT_KEY_0: _browser_letter_show(sd, "0"); break;
    case ENNA_INPUT_KEY_1: _browser_letter_show(sd, "1"); break;
    case ENNA_INPUT_KEY_2: _browser_letter_show(sd, "2"); break;
    case ENNA_INPUT_KEY_3: _browser_letter_show(sd, "3"); break;
    case ENNA_INPUT_KEY_4: _browser_letter_show(sd, "4"); break;
    case ENNA_INPUT_KEY_5: _browser_letter_show(sd, "5"); break;
    case ENNA_INPUT_KEY_6: _browser_letter_show(sd, "6"); break;
    case ENNA_INPUT_KEY_7: _browser_letter_show(sd, "7"); break;
    case ENNA_INPUT_KEY_8: _browser_letter_show(sd, "8"); break;
    case ENNA_INPUT_KEY_9: _browser_letter_show(sd, "9"); break;

    case ENNA_INPUT_KEY_A: _browser_letter_show(sd, "a"); break;
    case ENNA_INPUT_KEY_B: _browser_letter_show(sd, "b"); break;
    case ENNA_INPUT_KEY_C: _browser_letter_show(sd, "c"); break;
    case ENNA_INPUT_KEY_D: _browser_letter_show(sd, "d"); break;
    case ENNA_INPUT_KEY_E: _browser_letter_show(sd, "e"); break;
    case ENNA_INPUT_KEY_F: _browser_letter_show(sd, "f"); break;
    case ENNA_INPUT_KEY_G: _browser_letter_show(sd, "g"); break;
    case ENNA_INPUT_KEY_H: _browser_letter_show(sd, "h"); break;
    case ENNA_INPUT_KEY_I: _browser_letter_show(sd, "i"); break;
    case ENNA_INPUT_KEY_J: _browser_letter_show(sd, "j"); break;
    case ENNA_INPUT_KEY_K: _browser_letter_show(sd, "k"); break;
    case ENNA_INPUT_KEY_L: _browser_letter_show(sd, "l"); break;
    case ENNA_INPUT_KEY_M: _browser_letter_show(sd, "m"); break;
    case ENNA_INPUT_KEY_N: _browser_letter_show(sd, "n"); break;
    case ENNA_INPUT_KEY_O: _browser_letter_show(sd, "o"); break;
    case ENNA_INPUT_KEY_P: _browser_letter_show(sd, "p"); break;
    case ENNA_INPUT_KEY_Q: _browser_letter_show(sd, "q"); break;
    case ENNA_INPUT_KEY_R: _browser_letter_show(sd, "r"); break;
    case ENNA_INPUT_KEY_S: _browser_letter_show(sd, "s"); break;
    case ENNA_INPUT_KEY_T: _browser_letter_show(sd, "t"); break;
    case ENNA_INPUT_KEY_U: _browser_letter_show(sd, "u"); break;
    case ENNA_INPUT_KEY_V: _browser_letter_show(sd, "v"); break;
    case ENNA_INPUT_KEY_W: _browser_letter_show(sd, "w"); break;
    case ENNA_INPUT_KEY_Z: _browser_letter_show(sd, "z"); break;

    default:
        break;
    }
}
