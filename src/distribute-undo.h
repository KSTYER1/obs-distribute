/* src/distribute-undo.h */
#pragma once

#include <obs.h>

#ifdef __cplusplus
extern "C" {
#endif

char *dist_snapshot_selected(obs_scene_t *scene);

void dist_undo_apply(const char *data);

#ifdef __cplusplus
}
#endif
