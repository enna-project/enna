/* Interface */

#include "enna.h"
#include <Emotion.h>


static int            em_init(Enna_Module *em);
static int            em_shutdown(Enna_Module *em);

static void _class_init(int dummy);
static void _class_shutdown(int dummy);
static int _class_file_set(const char *uri);
static int _class_play(void);
static int _class_pause(void);
static int _class_stop(void);
static double _class_position_get();
static double _class_length_get();
static Enna_Metadata *_class_metadata_get(void);

static Enna_Class_MediaplayerBackend class =
{
  "emotion",
  1,
  {
    _class_init,
    _class_shutdown,
    _class_file_set,
    _class_play,
    _class_pause,
    _class_stop,
    _class_position_get,
    _class_length_get,
    _class_metadata_get,
  }
};

typedef struct _Enna_Module_Emotion Enna_Module_Emotion;

struct _Enna_Module_Emotion
{
   Evas *evas;
   Evas_Object *o_emotion;
   Enna_Module *em;
};

static Enna_Module_Emotion *mod;

EAPI Enna_Module_Api module_api =
{
    ENNA_MODULE_VERSION,
    "emotion"
};

static void _class_init(int dummy)
{
   printf("emotion class init\n");

}

static void _class_shutdown(int dummy)
{
   emotion_object_play_set(mod->o_emotion, 0);
   evas_object_del(mod->o_emotion);
}

static int _class_file_set(const char *uri)
{
   emotion_object_file_set(mod->o_emotion, uri);
   return 0;
}

static int _class_play(void)
{
   emotion_object_play_set(mod->o_emotion, 1);
   return 0;
}

static int _class_stop(void)
{
   emotion_object_play_set(mod->o_emotion, 0);
   emotion_object_position_set(mod->o_emotion, 0);
   return 0;
}

static int _class_pause(void)
{
   emotion_object_play_set(mod->o_emotion, 0);
   return 0;
}

static double _class_position_get()
{
   return emotion_object_position_get(mod->o_emotion);
}

static double _class_length_get()
{
   return emotion_object_play_length_get(mod->o_emotion);
}

static Enna_Metadata *_class_metadata_get(void)
{
   Enna_Metadata *m;

   m = calloc(1, sizeof(Enna_Metadata));

   m->title = emotion_object_meta_info_get(mod->o_emotion, EMOTION_META_INFO_TRACK_TITLE);
   m->album= emotion_object_meta_info_get(mod->o_emotion, EMOTION_META_INFO_TRACK_ALBUM);
   m->artist = emotion_object_meta_info_get(mod->o_emotion, EMOTION_META_INFO_TRACK_ARTIST);
   m->genre = emotion_object_meta_info_get(mod->o_emotion, EMOTION_META_INFO_TRACK_GENRE);
   m->year =  emotion_object_meta_info_get(mod->o_emotion, EMOTION_META_INFO_TRACK_YEAR);
   m->comment = emotion_object_meta_info_get(mod->o_emotion, EMOTION_META_INFO_TRACK_COMMENT);
   m->discid = emotion_object_meta_info_get(mod->o_emotion, EMOTION_META_INFO_TRACK_DISC_ID);
   m->track = emotion_object_meta_info_get(mod->o_emotion, EMOTION_META_INFO_TRACK_COUNT);

   return m;

}

/* Module interface */

static int
em_init(Enna_Module *em)
{
   mod = calloc(1, sizeof(Enna_Module_Emotion));
   mod->em = em;
   mod->evas = em->evas;
   mod->o_emotion = emotion_object_add(mod->evas);
   /* Fixme should come frome config */
   if (!emotion_object_init(mod->o_emotion, "gstreamer"))
     {
	printf("Error : could not initialize gstreamer plugin for emotion\n");
	return 0;
     }
   enna_mediaplayer_backend_register(&class);
   return 1;
}


static int
em_shutdown(Enna_Module *em)
{

   _class_shutdown(0);
   free(mod);
   return 1;
}

EAPI void
module_init(Enna_Module *em)
{
    if (!em)
        return;

    if (!em_init(em))
        return;
}

EAPI void
module_shutdown(Enna_Module *em)
{
    em_shutdown(em);
}
