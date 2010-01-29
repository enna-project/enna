/*
 * GeeXboX Enna Media Center.
 * Copyright (C) 2005-2009 The Enna Project
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

#include <Ecore.h>
#include <Elementary.h>
#include <Edje.h>

#include "enna.h"
#include "logs.h"
#include "view_list.h"
#include "utils.h"
#include "browser.h"
#include "games.h"
#include "games_mame.h"

#define ENNA_MODULE_NAME "games_mame"

#define MAME_SNAP_URL "http://www.progettoemma.net/snap/%s/0000.png"


typedef struct _Mame_Game {
   const char  *id;
   const char  *name;
}Mame_Game;

typedef struct _Games_Service_Mame {
    Evas_Object *o_edje;
    Evas_Object *o_list;
    const char  *snap_path;
    const char  *rom_path;
    Mame_Game   *current_game;
    Eina_List   *mame_games;
    Eina_Hash   *mame_games_hash;
   
} Games_Service_Mame;

static Games_Service_Mame *mod;

static void _mame_update_info(Mame_Game *game);

/****************************************************************************/
/*                         M.A.M.E Frontend Utils                           */
/****************************************************************************/

static void
_mame_run(void *data)
{
    Mame_Game *game = data;
    char cmd[128];

    /* Exec mame */
    snprintf(cmd, sizeof(cmd), "sdlmame %s", game->id);    
    /* TESTING: Exec mame as a child window
     * snprintf(cmd, sizeof(cmd), "SDL_WINDOWID=%d sdlmame -window  %s",
     *                             enna->ee_winid, game->id);
     */
    games_service_exec(cmd, "<c>M.A.M.E.</c><br>Game running...");
}

static int
_mame_sort_cb(const void *d1, const void *d2)
{
   const Mame_Game *game1 = d1;
   const Mame_Game *game2 = d2;

   if(!game1 || !game1->name) return(1);
   if(!game2 || !game2->name) return(-1);

   return(strcmp(game1->name, game2->name));
}

static void
_mame_listfull(void)
{
    FILE *fp;
    char line[1024];

    if (!mod->mame_games_hash)
        mod->mame_games_hash = eina_hash_string_superfast_new(NULL); //TODO: or use stringshare hash?
   
    fp = popen("sdlmame -listfull", "r");
    if (fp == NULL)
        return;
 
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        char id[64];
        char name[256];
        int res;

        res = sscanf(line, "%s \"%[^\"]", id, name);
        if (res == 2)
        {
            Mame_Game *game;

            /* alloc the new game */
            game = ENNA_NEW(Mame_Game, 1);
            game->id = eina_stringshare_add(id);
            game->name = eina_stringshare_add(name);

            /* add the game to the list and to the hash */
            mod->mame_games = eina_list_append(mod->mame_games, game);
            eina_hash_direct_add(mod->mame_games_hash, game->id, game);
        }
    }
    pclose(fp);

    if (eina_list_count(mod->mame_games) < 1000)
        enna_util_message_show("<c>Mame Error</c><br><b>Can not get supported "
                             "games list</b><br>Is sdlmame installed and updated?");

    mod->mame_games = eina_list_sort(mod->mame_games, 0, _mame_sort_cb);
}

static void
_mame_dwnl_snap_complete_cb(void *data, const char *file, int status)
{
    Mame_Game *game = data;

    if (mod->current_game == game)
        _mame_update_info
        (game);
}

static void
_mame_update_info(Mame_Game *game)
{
    char buf[PATH_MAX];
    char url[PATH_MAX];
    char *dir;

    mod->current_game = game;
    games_service_title_show(game->name);
    
    snprintf(buf, sizeof(buf), "%s/%s/0000.png", mod->snap_path, game->id);

    if (ecore_file_exists(buf))
    {
        /* Show snapshot */
        games_service_image_show(buf);
    }
    else
    {
        /* Create snapshot path */
        dir = ecore_file_dir_get(buf);
        if (!ecore_file_exists(dir))
            ecore_file_mkpath(dir);
        free(dir);

        /* Download snapshot */ 
        snprintf(url, sizeof(url), MAME_SNAP_URL, game->id);
        ecore_file_download(url, buf, _mame_dwnl_snap_complete_cb, NULL, game, NULL);
    }
}

static void
_mame_game_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
    Mame_Game *game;

    game = enna_list_selected_data_get(obj);
    if (game) _mame_update_info(game);
}

static void
mame_my_games_list(void)
{
    Evas_Object *o;
    Eina_List *file_list, *game_list = NULL;
    char *rom, *id;
    Mame_Game *game;
    int count = 0;

    if (!mod->mame_games)
        return;

    /* Populate a list with existing roms */
    file_list = ecore_file_ls(mod->rom_path);
    EINA_LIST_FREE(file_list, rom)
    {
        id = ecore_file_strip_ext(rom);
        game = eina_hash_find(mod->mame_games_hash, id);
        if (game) game_list = eina_list_append(game_list, game);
        free(id);
        free(rom);
    }

    /* Show the enna_list or an error message */
    if (game_list)
    {
        o = enna_list_add(enna->evas);
        evas_object_smart_callback_add(o, "selected", _mame_game_selected_cb, NULL);
        game_list = eina_list_sort(game_list, 0, _mame_sort_cb);
        EINA_LIST_FREE(game_list, game)
        {
            Enna_Vfs_File *item;

            item = ENNA_NEW(Enna_Vfs_File, 1);
            item->label   = strdup(game->name);
            item->uri     = strdup(game->id);
            item->is_menu = 1;
            
            enna_list_file_append(o, item, _mame_run, (void*)game);
            count++;
        }
        
        enna_list_select_nth(o, 0);
        ENNA_OBJECT_DEL(mod->o_list);
        mod->o_list = o;
        edje_object_part_swallow(mod->o_edje, "service.browser.swallow", o);
    }
    else
    {
        enna_util_message_show("<c>Mame Error</c><br><b>No roms found in path</b>"
                               "<br>Roms must be located at: ~/.mame/roms<br>");
        ENNA_OBJECT_DEL(o);
    }
    games_service_total_show(count);
}

/****************************************************************************/
/*                         Private Service API                              */
/****************************************************************************/

static Eina_Bool
mame_event(Evas_Object *edje, enna_input event)
{
    enna_log(ENNA_MSG_EVENT, ENNA_MODULE_NAME,
             "Key pressed M.A.M.E. : %d", event);

    if (mod->o_list)
        return enna_list_input_feed(mod->o_list, event);
    else
        return ENNA_EVENT_CONTINUE;
}

static void
mame_show(Evas_Object *edje)
{
    char buf[PATH_MAX];

    /* Alloc local data once for all */
    if (!mod)
    {
        mod = ENNA_NEW(Games_Service_Mame, 1);
        mod->o_edje = edje;
        snprintf(buf, sizeof(buf), "%s/.mame/snap", enna_util_user_home_get());
        mod->snap_path = strdup(buf);
        snprintf(buf, sizeof(buf), "%s/.mame/roms", enna_util_user_home_get());
        mod->rom_path = strdup(buf);
    }

    /* Build game list once for all */
    if (!mod->mame_games)
    {
      //TODO show a dialog while doing this
      _mame_listfull();
    }

    /* Show the list of local roms */
    mame_my_games_list();
}

static void
mame_hide(Evas_Object *edje)
{
    ENNA_OBJECT_DEL(mod->o_list);

    games_service_image_show(NULL);
    games_service_title_show("");
}

/****************************************************************************/
/*                         Public Service API                               */
/****************************************************************************/

Games_Service games_mame = {
    "M.A.M.E.",
    "background/games",
    "icon/mame",
    mame_show,
    mame_hide,
    mame_event
};