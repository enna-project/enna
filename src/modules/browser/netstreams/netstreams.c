#include "enna.h"
#include "url_utils.h"

#define ENNA_MODULE_NAME "netstreams"

#define EXT_M3U_HEADER "#EXTM3U"
#define EXT_M3U_INF "#EXTINF:"

#define MAX_LINE 1024

typedef struct netstream_s
{
    char *uri;
    char *label;
    char *icon;
} netstream_t;

typedef struct netstreams_priv_s
{
    const char *uri;
    const char *prev_uri;
    Eina_List *config_netstreams;
} netstreams_priv_t;

typedef struct Enna_Module_Netstreams_s
{
    Evas *e;
    Enna_Module *em;
    CURL *curl;
    netstreams_priv_t *music;
    netstreams_priv_t *video;
} Enna_Module_Netstreams;

static Enna_Module_Netstreams *mod;

static Eina_List * browse_streams_list(netstreams_priv_t *data)
{
    Eina_List *list = NULL;
    Eina_List *l;

    for (l = data->config_netstreams; l; l = l->next)
    {
        Enna_Vfs_File *file;
        netstream_t *stream;

        stream = l->data;
        file = enna_vfs_create_directory(stream->uri, stream->label,
                stream->icon, NULL);
        list = eina_list_append(list, file);
    }

    data->prev_uri = NULL;
    data->uri = NULL;

    return list;
}

static char * read_line_from_stream(FILE *stream)
{
    char line[MAX_LINE];
    int i = 0;
    char *l;

    memset(line, '\0', MAX_LINE);
    l = fgets(line, MAX_LINE, stream);

    if (!strcmp(line, ""))
        return NULL;

    while (line[i] != '\n' && i < MAX_LINE)
        i++;
    line[i] = '\0';

    return strdup(line);
}

static Eina_List * parse_extm3u(FILE *f)
{
    Eina_List *list = NULL;

    while (!feof(f))
    {
        char *l1 = read_line_from_stream(f);
        if (!l1)
            break; /* End Of Stream */

        if (!strncmp(l1, EXT_M3U_INF, strlen(EXT_M3U_INF)))
        {
            char *title = strstr(l1, "-");
            if (!title)
                continue; /* invalid EXT_M3U line */

            title += 2;

            while (1)
            {
                char *l2;
                Enna_Vfs_File *file;

                l2 = read_line_from_stream(f);
                if (!l2)
                    break;

                if (*l2 == '#')
                {
                    /* skip this line */
                    free(l2);
                    continue;
                }

                file = enna_vfs_create_file(l2, title, "icon/video", NULL);
                list = eina_list_append(list, file);
                free(l2);
                break;
            }
        }
        free(l1);
    }

    return list;
}

static Eina_List * parse_netstream(const char *path, netstreams_priv_t *data)
{
    FILE *f;
    char tmp[] = "/tmp/enna-netstreams-XXXXXX";
    url_data_t chunk;
    char *file, *header;
    Eina_List *streams = NULL;
    int n, dl = 1;

    if (strstr(path, "file://"))
        dl = 0;

    /* download playlist */
    if (dl)
    {
        file = mktemp(tmp);
        chunk = url_get_data(mod->curl, (char *) path);
        if (chunk.status != CURLE_OK)
          return NULL;

        f = fopen(file, "w");
        if (!f)
            return NULL;

        n = fwrite(chunk.buffer, chunk.size, 1, f);
        free(chunk.buffer);
        fclose(f);
    }
    else
        file = (char *) path + 7;

    /* parse playlist */
    f = fopen(file, "r");
    if (!f)
        return NULL;

    /* Network Extended M3U Playlist Stream Online Listings */
    header = read_line_from_stream(f);
    if (header && !strncmp(header, EXT_M3U_HEADER, strlen(EXT_M3U_HEADER)))
        streams = parse_extm3u(f);

    free(header);
    fclose(f);
    if (dl)
        unlink(file);

    data->prev_uri = data->uri;
    data->uri = evas_stringshare_add(path);

    return streams;
}

static Eina_List * browse_up(const char *path, netstreams_priv_t *data,
        char *icon)
{
    if (!path)
        return browse_streams_list(data);

    /* path is given, download playlist and parse it */
    return parse_netstream(path, data);
}

static Eina_List * browse_up_music(const char *path, void *cookie)
{
    return browse_up(path, mod->music, "icon/music");
}

static Eina_List * browse_up_video(const char *path, void *cookie)
{
    return browse_up(path, mod->video, "icon/video");
}

static Eina_List * browse_down(netstreams_priv_t *data)
{
    if (!data->prev_uri)
        return browse_streams_list(data);

    return NULL;
}

static Eina_List * browse_down_music(void *cookie)
{
    return browse_down(mod->music);
}

static Eina_List * browse_down_video(void *cookie)
{
    return browse_down(mod->video);
}

static Enna_Vfs_File * vfs_get_music(void *cookie)
{
    return enna_vfs_create_directory(mod->music->uri,
            ecore_file_file_get(mod->music->uri),
            evas_stringshare_add("icon/music"), NULL);
}

static Enna_Vfs_File * vfs_get_video(void *cookie)
{
    return enna_vfs_create_directory(mod->video->uri,
            ecore_file_file_get(mod->video->uri),
            evas_stringshare_add("icon/video"), NULL);
}

static void class_init(const char *name, netstreams_priv_t **priv,
        ENNA_VFS_CAPS caps, Enna_Class_Vfs *class, char *key)
{
    netstreams_priv_t *data;
    Enna_Config_Data *cfgdata;
    Eina_List *l;

    data = calloc(1, sizeof(netstreams_priv_t));
    *priv = data;

    enna_vfs_append(name, caps, class);

    data->prev_uri = NULL;
    data->config_netstreams = NULL;

    cfgdata = enna_config_module_pair_get("netstreams");
    if (!cfgdata)
        return;

    for (l = cfgdata->pair; l; l = l->next)
    {
        Config_Pair *pair = l->data;

        if (pair->key && !strcmp(pair->key, key))
        {
            Eina_List *tuple;
            netstream_t *stream = NULL;

            enna_config_value_store(&tuple, key, ENNA_CONFIG_STRING_LIST, pair);
            if (!tuple)
                continue;

            if (eina_list_count(tuple) != 3)
                continue;

            stream = calloc(1, sizeof(netstream_t));
            stream->uri = eina_list_nth(tuple, 0);
            stream->label = eina_list_nth(tuple, 1);
            stream->icon = eina_list_nth(tuple, 2);

            enna_log(ENNA_MSG_INFO, ENNA_MODULE_NAME,
                    "Adding new netstream '%s' (using icon %s), URI is '%s'",
                    stream->label, stream->icon, stream->uri);

            data->config_netstreams = eina_list_append(data->config_netstreams,
                    stream);
        }
    }
}

static Enna_Class_Vfs class_music =
{ "netstreams_music", 1, "Browse Network Streams", NULL, "icon/music",
{ NULL, NULL, browse_up_music, browse_down_music, vfs_get_music,
}, NULL
};

static Enna_Class_Vfs class_video =
{ "netstreams_video", 1, "Browse Network Streams", NULL, "icon/video",
{ NULL, NULL, browse_up_video, browse_down_video, vfs_get_video,
}, NULL
};

/* Module interface */

Enna_Module_Api module_api =
{
    ENNA_MODULE_VERSION,
    ENNA_MODULE_BROWSER,
    "browser_netstreams"
};

void module_init(Enna_Module *em)
{
    if (!em)
        return;

    mod = calloc(1, sizeof(Enna_Module_Netstreams));
    mod->em = em;
    em->mod = mod;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    mod->curl = curl_easy_init();

    class_init("netstreams_music", &mod->music, ENNA_CAPS_MUSIC, &class_music,
            "stream_music");
    class_init("netstreams_video", &mod->video, ENNA_CAPS_VIDEO, &class_video,
            "stream_video");
}

void module_shutdown(Enna_Module *em)
{
    Enna_Module_Netstreams *mod;

    mod = em->mod;;
    free(mod->music);
    free(mod->video);

    if (mod->curl)
        curl_easy_cleanup(mod->curl);
    curl_global_cleanup();
}