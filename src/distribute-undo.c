/* src/distribute-undo.c */
#include "distribute-undo.h"
#include "plugin-support.h"

#include <obs-frontend-api.h>
#include <graphics/vec2.h>
#include <util/bmem.h>
#include <util/dstr.h>

typedef struct {
	obs_data_array_t *items;
	const char *scene_name;
} snapshot_ctx_t;

static void item_to_data(obs_sceneitem_t *item, obs_data_t *out)
{
	struct vec2 pos, scale, bounds;
	obs_sceneitem_get_pos(item, &pos);
	obs_sceneitem_get_scale(item, &scale);
	obs_sceneitem_get_bounds(item, &bounds);

	obs_data_set_int(out, "id", obs_sceneitem_get_id(item));
	obs_data_set_double(out, "pos_x", pos.x);
	obs_data_set_double(out, "pos_y", pos.y);
	obs_data_set_double(out, "scale_x", scale.x);
	obs_data_set_double(out, "scale_y", scale.y);
	obs_data_set_double(out, "rot", obs_sceneitem_get_rot(item));
	obs_data_set_int(out, "bounds_type", obs_sceneitem_get_bounds_type(item));
	obs_data_set_double(out, "bounds_x", bounds.x);
	obs_data_set_double(out, "bounds_y", bounds.y);
	obs_data_set_int(out, "alignment", obs_sceneitem_get_alignment(item));
}

static bool enum_snapshot_cb(obs_scene_t *scene, obs_sceneitem_t *item, void *param)
{
	UNUSED_PARAMETER(scene);
	snapshot_ctx_t *ctx = (snapshot_ctx_t *)param;

	if (!obs_sceneitem_selected(item))
		return true;
	if (obs_sceneitem_locked(item))
		return true;

	obs_data_t *entry = obs_data_create();
	item_to_data(item, entry);
	obs_data_array_push_back(ctx->items, entry);
	obs_data_release(entry);
	return true;
}

char *dist_snapshot_selected(obs_scene_t *scene)
{
	if (!scene)
		return NULL;

	obs_source_t *scene_src = obs_scene_get_source(scene);
	const char *scene_name = obs_source_get_name(scene_src);

	obs_data_t *root = obs_data_create();
	obs_data_array_t *items = obs_data_array_create();
	obs_data_set_string(root, "scene", scene_name ? scene_name : "");

	snapshot_ctx_t ctx = {items, scene_name};
	obs_scene_enum_items(scene, enum_snapshot_cb, &ctx);

	obs_data_set_array(root, "items", items);
	obs_data_array_release(items);

	const char *json = obs_data_get_json(root);
	char *copy = bstrdup(json ? json : "{}");
	obs_data_release(root);
	return copy;
}

static void apply_entry_to_item(obs_sceneitem_t *item, obs_data_t *entry)
{
	struct obs_transform_info info;
	obs_sceneitem_get_info2(item, &info);

	info.pos.x = (float)obs_data_get_double(entry, "pos_x");
	info.pos.y = (float)obs_data_get_double(entry, "pos_y");
	info.scale.x = (float)obs_data_get_double(entry, "scale_x");
	info.scale.y = (float)obs_data_get_double(entry, "scale_y");
	info.rot = (float)obs_data_get_double(entry, "rot");
	info.bounds_type =
		(enum obs_bounds_type)obs_data_get_int(entry, "bounds_type");
	info.bounds.x = (float)obs_data_get_double(entry, "bounds_x");
	info.bounds.y = (float)obs_data_get_double(entry, "bounds_y");
	info.alignment = (uint32_t)obs_data_get_int(entry, "alignment");

	obs_sceneitem_set_info2(item, &info);
}

void dist_undo_apply(const char *data)
{
	if (!data || !*data)
		return;

	obs_data_t *root = obs_data_create_from_json(data);
	if (!root)
		return;

	const char *scene_name = obs_data_get_string(root, "scene");
	obs_source_t *scene_src = obs_get_source_by_name(scene_name);
	if (!scene_src) {
		obs_log(LOG_WARNING, "undo: scene '%s' not found", scene_name);
		obs_data_release(root);
		return;
	}
	obs_scene_t *scene = obs_scene_from_source(scene_src);
	if (!scene) {
		obs_source_release(scene_src);
		obs_data_release(root);
		return;
	}

	obs_data_array_t *items = obs_data_get_array(root, "items");
	if (items) {
		size_t count = obs_data_array_count(items);
		for (size_t i = 0; i < count; ++i) {
			obs_data_t *entry = obs_data_array_item(items, i);
			int64_t id = obs_data_get_int(entry, "id");
			obs_sceneitem_t *item =
				obs_scene_find_sceneitem_by_id(scene, id);
			if (item)
				apply_entry_to_item(item, entry);
			obs_data_release(entry);
		}
		obs_data_array_release(items);
	}

	obs_source_release(scene_src);
	obs_data_release(root);
}
