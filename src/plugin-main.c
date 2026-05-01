/* src/plugin-main.c */
#include <obs-module.h>
#include "plugin-support.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-distribute", "en-US")

MODULE_EXPORT const char *obs_module_description(void)
{
	return "Photoshop-style align and distribute for OBS scene items";
}

MODULE_EXPORT const char *obs_module_name(void)
{
	return "obs-distribute";
}

MODULE_EXPORT const char *obs_module_author(void)
{
	return "Awet";
}

extern void obs_distribute_register_dock(void);

bool obs_module_load(void)
{
	obs_log(LOG_INFO, "loaded (version %s)", PLUGIN_VERSION);
	return true;
}

void obs_module_post_load(void)
{
	obs_distribute_register_dock();
}

void obs_module_unload(void)
{
	obs_log(LOG_INFO, "unloaded");
}
