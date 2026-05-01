/* src/distribute-core.h */
#pragma once

#include <obs.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	DIST_ACTION_ALIGN_LEFT,
	DIST_ACTION_ALIGN_HCENTER,
	DIST_ACTION_ALIGN_RIGHT,
	DIST_ACTION_ALIGN_TOP,
	DIST_ACTION_ALIGN_VCENTER,
	DIST_ACTION_ALIGN_BOTTOM,

	DIST_ACTION_DIST_H_LEFT_EDGES,
	DIST_ACTION_DIST_H_CENTERS,
	DIST_ACTION_DIST_H_RIGHT_EDGES,
	DIST_ACTION_DIST_H_SPACING,

	DIST_ACTION_DIST_V_TOP_EDGES,
	DIST_ACTION_DIST_V_CENTERS,
	DIST_ACTION_DIST_V_BOTTOM_EDGES,
	DIST_ACTION_DIST_V_SPACING,

	DIST_ACTION_SAME_WIDTH,
	DIST_ACTION_SAME_HEIGHT,
	DIST_ACTION_SAME_SIZE,

	DIST_ACTION_RANDOM_SCATTER,
} dist_action_t;

typedef struct {
	float left;
	float top;
	float right;
	float bottom;
} dist_aabb_t;

int dist_count_selected(obs_scene_t *scene);

bool dist_apply(obs_scene_t *scene, dist_action_t action);

void dist_compute_aabb(obs_sceneitem_t *item, dist_aabb_t *out);

#ifdef __cplusplus
}
#endif
