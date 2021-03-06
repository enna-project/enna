/*
 * Enna Media Center
 * Copyright (C) 2005-2013 Enna Team. All rights reserved.
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

#include "kbdnav.h"
#include "logs.h"

#include <Evas.h>
#include <Elementary.h>

typedef struct _Enna_Kbdnav_Item Enna_Kbdnav_Item;

enum _Kbdnav_Direction
{
    UP,
    LEFT,
    DOWN,
    RIGHT
};

struct _Enna_Kbdnav
{
    Eina_List *items;
    Enna_Kbdnav_Item * current;
};

struct _Enna_Kbdnav_Item
{
    void *obj;
    const Elm_Object_Item *(*object_get)(void *item_data, void *user_data);
    void  (*select_set)(void *item_data, void *user_data);
    void  (*activate_set)(void *item_data, void *user_data);
    void *user_data;
};

Enna_Kbdnav *
enna_kbdnav_add(void)
{
    Enna_Kbdnav *nav;

    nav = calloc(1, sizeof(Enna_Kbdnav));

    return nav;
}

void 
enna_kbdnav_del(Enna_Kbdnav *nav)
{
    if (!nav)
        return;

    eina_list_free(nav->items);

    free(nav);
}


void 
enna_kbdnav_item_add(Enna_Kbdnav *nav, void *obj, 
                     Enna_Kbdnav_Class *class, void *user_data)
{
    Enna_Kbdnav_Item *it;

    if (!nav || !obj)
        return;

    if (!class->object_get)
    {
        ERR("object_get is NULL");
        return;
    }
    if (!class->select_set)
    {
        ERR("select_set is NULL");
        return;
    }

    if (!class->activate_set)
    {
        DBG("activate_set is NULL");
    }

    it = calloc(1, sizeof(Enna_Kbdnav_Item));
    it->obj = obj;
    it->object_get = class->object_get;
    it->select_set = class->select_set;
    it->activate_set = class->activate_set;
    it->user_data = user_data;

    nav->items = eina_list_append(nav->items, it);

    if(!nav->current)
    {
        it->select_set(it->obj, it->user_data);
        nav->current = it;
    }
}

void 
enna_kbdnav_item_del(Enna_Kbdnav *nav, void *obj)
{
    Enna_Kbdnav_Item *it;
    Eina_List *l;
    if (!nav || !obj)
        return;

    EINA_LIST_FOREACH(nav->items, l, it)
    {
        if (it->obj == obj)
        {
            nav->items = eina_list_remove(nav->items, it);
            return;
        }
    }
}

Eina_Bool 
enna_kbdnav_current_set(Enna_Kbdnav *nav, void *obj)
{
    Enna_Kbdnav_Item *current = NULL;
    Eina_List *l;
    Enna_Kbdnav_Item *it;
    if (!nav || !obj)
        return EINA_FALSE;

    EINA_LIST_FOREACH(nav->items, l, it)
    {
        if (it->obj == obj)
        {
            current = it;
            break;
        }
    }

    if (!current)
        return EINA_FALSE;
    else
        nav->current = current;

    return EINA_TRUE;
}

void *
enna_kbdnav_current_get(Enna_Kbdnav *nav)
{
    if (!nav || !nav->current)
        return NULL;

    return nav->current->obj;
}

static Eina_Bool 
_kbdnav_direction(Enna_Kbdnav *nav, int direction)
{
    unsigned int cx = 0, cy = 0;
    unsigned int x, y;
    Eina_List *l;
    const Elm_Object_Item *obj_it;
    Enna_Kbdnav_Item *it;
    Enna_Kbdnav_Item *previous;
    Enna_Kbdnav_Item *next = NULL;
    unsigned int cd = UINT32_MAX ;

    if (!nav)
        return EINA_FALSE;

    previous = nav->current;

    if (previous)
    {
        obj_it = previous->object_get(previous->obj, previous->user_data);
        elm_gengrid_item_pos_get(obj_it, &cx, &cy);
    }

    EINA_LIST_FOREACH(nav->items, l, it)
    {
        unsigned int d;

        if (it == nav->current)
            continue;

        obj_it = it->object_get(it->obj, it->user_data);
     //   if (!evas_object_visible_get(obj) || evas_object_pass_events_get(obj))
       //     continue;
        elm_gengrid_item_pos_get(obj_it, &x, &y);


        switch (direction)
        {
        case UP:
            DBG("Kbdnav UP");
            // UP
            if (y >= cy)
                continue;
            else
            {
                d = (cx - x)*(cx -x) + (cy - y)*(cy - y);
                if (d < cd && y < cy)
                {
                    next = it;
                    cd = d;
                }
            }
            break;
        case RIGHT:
            DBG("Kbdnav RIGHT");
            // RIGHT
            if (x < cx)
                continue;
            else
            {
                d = (cx - x)*(cx -x) + (cy - y)*(cy - y);
                if (d < cd && x > cx)
                {
                    next = it;
                    cd = d;
                }
            }
            break;
        case DOWN:
            DBG("Kbdnav DOWN");
            // DOWN
            if (y < cy)
                continue;
            else
            {
                d = (cx - x)*(cx -x) + (cy - y)*(cy - y);
                if (d < cd && y > cy)
                {
                    DBG("Kbdnav find down item");
                    next = it;
                    cd = d;
                }
            }
            break;
        case LEFT:
            DBG("Kbdnav LEFT");
            // LEFT
            if (x >= cx)
                continue;
            else
            {

                d = (cx - x)*(cx -x) + (cy - y)*(cy - y);
                if (d < cd && x < cx)
                {
                    next = it;
                    cd = d;
                }
            }
            break;
        default:
            break;
        }
    }

    if (next)
    {
        next->select_set(next->obj, next->user_data);
        nav->current = next;
        //it->unselect_set(it->obj, it->user_data);
        return EINA_TRUE;
    }
    else
        nav->current = previous;

    return EINA_FALSE;
}

Eina_Bool 
enna_kbdnav_up(Enna_Kbdnav *nav)
{
    return _kbdnav_direction(nav, UP);
}

Eina_Bool 
enna_kbdnav_right(Enna_Kbdnav *nav)
{
    return _kbdnav_direction(nav, RIGHT);
}

Eina_Bool 
enna_kbdnav_down(Enna_Kbdnav *nav)
{
    return _kbdnav_direction(nav, DOWN);
}


Eina_Bool 
enna_kbdnav_left(Enna_Kbdnav *nav)
{
    return _kbdnav_direction(nav, LEFT);
}

void 
enna_kbdnav_activate(Enna_Kbdnav *nav)
{
    if (!nav || !nav->current || !nav->current->activate_set)
        return;
    DBG("Kbdnav Activate");
    nav->current->activate_set(nav->current->obj, nav->current->user_data);
}
