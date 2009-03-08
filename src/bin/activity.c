#include "enna.h"
#include "activity.h"

static Eina_List *_enna_activities = NULL;

/**
 * @brief Register new activity
 * @param em enna module
 * @return -1 if error occurs, 0 otherwise
 */
int enna_activity_add(Enna_Class_Activity *class)
{
    Eina_List *l;
    Enna_Class_Activity *act;

    if (!class)
        return -1;
    for (l = _enna_activities; l; l = l->next)
    {
        act = l->data;
        if (act->pri > class->pri)
        {
            _enna_activities = eina_list_prepend_relative_list(
                    _enna_activities, class, l);
            return 0;
        }
    }
    _enna_activities = eina_list_append(_enna_activities, class);
    return 0;
}

static Enna_Class_Activity * enna_get_activity(const char *name)
{
    Eina_List *l;
    Enna_Class_Activity *act;

    if (!name)
        return NULL;

    for (l = _enna_activities; l; l = l->next)
    {
        act = l->data;
        if (!act)
            continue;

        if (act->name && !strcmp(act->name, name))
            return act;
    }

    return NULL;
}

/**
 * @brief Unregister an existing activity
 * @param em enna module
 * @return -1 if error occurs, 0 otherwise
 */
int enna_activity_del(const char *name)
{
    Enna_Class_Activity *act;

    act = enna_get_activity (name);
    if (!act)
        return -1;

    _enna_activities = eina_list_remove(_enna_activities, act);
    return 0;
}

/**
 * @brief Unregister all existing activities
 * @return -1 if error occurs, 0 otherwise
 */
void enna_activity_del_all (void)
{
    Eina_List *l;

    for (l = _enna_activities; l; l = l->next)
    {
        Enna_Class_Activity *act = l->data;
        _enna_activities = eina_list_remove(_enna_activities, act);
    }
}

/**
 * @brief Get list of activities registred
 * @return Eina_List of activities
 */
Eina_List *
enna_activities_get(void)
{
    return _enna_activities;
}

int enna_activity_init(const char *name)
{
    Enna_Class_Activity *act;

    act = enna_get_activity(name);
    if (!act)
        return -1;

    if (act->func.class_init)
        act->func.class_init(0);

    return 0;
}

int enna_activity_show(const char *name)
{
    Enna_Class_Activity *act;

    act = enna_get_activity(name);
    if (!act)
        return -1;

    if (act->func.class_show)
        act->func.class_show(0);

    return 0;
}

int enna_activity_shutdown(const char *name)
{
    Enna_Class_Activity *act;

    act = enna_get_activity(name);
    if (!act)
        return -1;

    if (act->func.class_shutdown)
        act->func.class_shutdown(0);

    return 0;
}

int enna_activity_hide(const char *name)
{
    Enna_Class_Activity *act;

    act = enna_get_activity(name);
    if (!act)
        return -1;

    if (act->func.class_hide)
        act->func.class_hide(0);

    return 0;
}

int enna_activity_event(Enna_Class_Activity *act, void *event_info)
{

    if (!act)
        return -1;

    if (act->func.class_event)
        act->func.class_event(event_info);

    return 0;
}

