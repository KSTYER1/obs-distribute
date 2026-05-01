/* src/distribute-core.c */
#include "distribute-core.h"
#include "plugin-support.h"

#include <graphics/matrix4.h>
#include <graphics/vec2.h>
#include <graphics/vec3.h>
#include <util/bmem.h>
#include <util/darray.h>

#include <stdlib.h>
#include <time.h>

typedef struct {
	obs_sceneitem_t *item;
	dist_aabb_t aabb;
	float width;
	float height;
	float center_x;
	float center_y;
} item_info_t;

typedef DARRAY(item_info_t) item_list_t;

void dist_compute_aabb(obs_sceneitem_t *item, dist_aabb_t *out)
{
	struct matrix4 m;
	obs_sceneitem_get_box_transform(item, &m);

	struct vec3 pts[4];
	struct vec3 in;

	vec3_set(&in, 0.0f, 0.0f, 0.0f);
	vec3_transform(&pts[0], &in, &m);
	vec3_set(&in, 1.0f, 0.0f, 0.0f);
	vec3_transform(&pts[1], &in, &m);
	vec3_set(&in, 0.0f, 1.0f, 0.0f);
	vec3_transform(&pts[2], &in, &m);
	vec3_set(&in, 1.0f, 1.0f, 0.0f);
	vec3_transform(&pts[3], &in, &m);

	out->left = out->right = pts[0].x;
	out->top = out->bottom = pts[0].y;
	for (int i = 1; i < 4; ++i) {
		if (pts[i].x < out->left)  out->left  = pts[i].x;
		if (pts[i].x > out->right) out->right = pts[i].x;
		if (pts[i].y < out->top)   out->top   = pts[i].y;
		if (pts[i].y > out->bottom)out->bottom= pts[i].y;
	}
}

static bool enum_count_cb(obs_scene_t *scene, obs_sceneitem_t *item, void *param)
{
	UNUSED_PARAMETER(scene);
	int *counter = (int *)param;
	if (obs_sceneitem_selected(item) && !obs_sceneitem_locked(item))
		(*counter)++;
	return true;
}

int dist_count_selected(obs_scene_t *scene)
{
	int count = 0;
	if (scene)
		obs_scene_enum_items(scene, enum_count_cb, &count);
	return count;
}

static bool enum_collect_cb(obs_scene_t *scene, obs_sceneitem_t *item, void *param)
{
	UNUSED_PARAMETER(scene);
	item_list_t *list = (item_list_t *)param;
	if (!obs_sceneitem_selected(item)) return true;
	if (obs_sceneitem_locked(item))   return true;
	item_info_t info;
	info.item = item;
	obs_sceneitem_addref(item);
	dist_compute_aabb(item, &info.aabb);
	info.width    = info.aabb.right  - info.aabb.left;
	info.height   = info.aabb.bottom - info.aabb.top;
	info.center_x = (info.aabb.left + info.aabb.right)  * 0.5f;
	info.center_y = (info.aabb.top  + info.aabb.bottom) * 0.5f;
	da_push_back(*list, &info);
	return true;
}

static void release_items(item_list_t *list)
{
	for (size_t i = 0; i < list->num; ++i)
		obs_sceneitem_release(list->array[i].item);
	da_free(*list);
}

static int cmp_by_left(const void *a, const void *b)    { float la=((const item_info_t*)a)->aabb.left,  lb=((const item_info_t*)b)->aabb.left;  return la<lb?-1:la>lb?1:0; }
static int cmp_by_center_x(const void *a, const void *b){ float la=((const item_info_t*)a)->center_x,   lb=((const item_info_t*)b)->center_x;   return la<lb?-1:la>lb?1:0; }
static int cmp_by_right(const void *a, const void *b)   { float la=((const item_info_t*)a)->aabb.right, lb=((const item_info_t*)b)->aabb.right; return la<lb?-1:la>lb?1:0; }
static int cmp_by_top(const void *a, const void *b)     { float la=((const item_info_t*)a)->aabb.top,   lb=((const item_info_t*)b)->aabb.top;   return la<lb?-1:la>lb?1:0; }
static int cmp_by_center_y(const void *a, const void *b){ float la=((const item_info_t*)a)->center_y,   lb=((const item_info_t*)b)->center_y;   return la<lb?-1:la>lb?1:0; }
static int cmp_by_bottom(const void *a, const void *b)  { float la=((const item_info_t*)a)->aabb.bottom,lb=((const item_info_t*)b)->aabb.bottom;return la<lb?-1:la>lb?1:0; }

static void translate_item(obs_sceneitem_t *item, float dx, float dy)
{
	if (dx==0.0f && dy==0.0f) return;
	struct vec2 pos;
	obs_sceneitem_get_pos(item, &pos);
	pos.x += dx; pos.y += dy;
	obs_sceneitem_set_pos(item, &pos);
}

static void get_source_size(obs_sceneitem_t *item, uint32_t *w, uint32_t *h)
{
	obs_source_t *src = obs_sceneitem_get_source(item);
	*w = src ? obs_source_get_width(src)  : 0;
	*h = src ? obs_source_get_height(src) : 0;
}

static void set_same_dimension(obs_sceneitem_t *ref, obs_sceneitem_t *item, bool width, bool height)
{
	enum obs_bounds_type ref_bt = obs_sceneitem_get_bounds_type(ref);
	struct vec2 ref_bounds, ref_scale;
	obs_sceneitem_get_bounds(ref, &ref_bounds);
	obs_sceneitem_get_scale(ref, &ref_scale);
	uint32_t ref_sw, ref_sh;
	get_source_size(ref, &ref_sw, &ref_sh);
	float target_w = (ref_bt!=OBS_BOUNDS_NONE) ? ref_bounds.x : (float)ref_sw*ref_scale.x;
	float target_h = (ref_bt!=OBS_BOUNDS_NONE) ? ref_bounds.y : (float)ref_sh*ref_scale.y;
	enum obs_bounds_type bt = obs_sceneitem_get_bounds_type(item);
	if (bt!=OBS_BOUNDS_NONE) {
		struct vec2 b;
		obs_sceneitem_get_bounds(item, &b);
		if (width)  b.x = target_w;
		if (height) b.y = target_h;
		obs_sceneitem_set_bounds(item, &b);
		return;
	}
	uint32_t sw, sh;
	get_source_size(item, &sw, &sh);
	struct vec2 scale;
	obs_sceneitem_get_scale(item, &scale);
	if (width  && sw>0) scale.x = target_w/(float)sw;
	if (height && sh>0) scale.y = target_h/(float)sh;
	obs_sceneitem_set_scale(item, &scale);
}

static bool apply_align(item_list_t *items, dist_action_t action)
{
	if (items->num<2) return false;
	const item_info_t *ref = &items->array[0];
	for (size_t i=1; i<items->num; ++i) {
		item_info_t *cur = &items->array[i];
		float dx=0.0f, dy=0.0f;
		switch(action) {
		case DIST_ACTION_ALIGN_LEFT:    dx=ref->aabb.left  -cur->aabb.left;   break;
		case DIST_ACTION_ALIGN_HCENTER: dx=ref->center_x   -cur->center_x;    break;
		case DIST_ACTION_ALIGN_RIGHT:   dx=ref->aabb.right -cur->aabb.right;  break;
		case DIST_ACTION_ALIGN_TOP:     dy=ref->aabb.top   -cur->aabb.top;    break;
		case DIST_ACTION_ALIGN_VCENTER: dy=ref->center_y   -cur->center_y;    break;
		case DIST_ACTION_ALIGN_BOTTOM:  dy=ref->aabb.bottom-cur->aabb.bottom; break;
		default: return false;
		}
		translate_item(cur->item, dx, dy);
	}
	return true;
}

static bool apply_distribute_horizontal(item_list_t *items, dist_action_t action)
{
	if (items->num<3) return false;
	int (*cmp)(const void*,const void*) = NULL;
	switch(action) {
	case DIST_ACTION_DIST_H_LEFT_EDGES: case DIST_ACTION_DIST_H_SPACING: cmp=cmp_by_left;     break;
	case DIST_ACTION_DIST_H_CENTERS:                                      cmp=cmp_by_center_x; break;
	case DIST_ACTION_DIST_H_RIGHT_EDGES:                                  cmp=cmp_by_right;    break;
	default: return false;
	}
	qsort(items->array, items->num, sizeof(item_info_t), cmp);
	const size_t n=items->num;
	const item_info_t *first=&items->array[0], *last=&items->array[n-1];
	if (action==DIST_ACTION_DIST_H_SPACING) {
		float total_w=0.0f;
		for (size_t i=0;i<n;++i) total_w+=items->array[i].width;
		float span=last->aabb.right-first->aabb.left;
		float gap=(span-total_w)/(float)(n-1);
		float cursor=first->aabb.left+first->width+gap;
		for (size_t i=1;i<n-1;++i) {
			item_info_t *cur=&items->array[i];
			float dx=cursor-cur->aabb.left;
			translate_item(cur->item,dx,0.0f);
			cursor+=cur->width+gap;
		}
		return true;
	}
	float start,end;
	switch(action) {
	case DIST_ACTION_DIST_H_LEFT_EDGES:  start=first->aabb.left;  end=last->aabb.left;  break;
	case DIST_ACTION_DIST_H_CENTERS:     start=first->center_x;   end=last->center_x;   break;
	case DIST_ACTION_DIST_H_RIGHT_EDGES: start=first->aabb.right; end=last->aabb.right; break;
	default: return false;
	}
	float step=(end-start)/(float)(n-1);
	for (size_t i=1;i<n-1;++i) {
		item_info_t *cur=&items->array[i];
		float target=start+step*(float)i;
		float cur_ref;
		switch(action) {
		case DIST_ACTION_DIST_H_LEFT_EDGES:  cur_ref=cur->aabb.left;  break;
		case DIST_ACTION_DIST_H_CENTERS:     cur_ref=cur->center_x;   break;
		case DIST_ACTION_DIST_H_RIGHT_EDGES: cur_ref=cur->aabb.right; break;
		default: return false;
		}
		translate_item(cur->item,target-cur_ref,0.0f);
	}
	return true;
}

static bool apply_distribute_vertical(item_list_t *items, dist_action_t action)
{
	if (items->num<3) return false;
	int (*cmp)(const void*,const void*) = NULL;
	switch(action) {
	case DIST_ACTION_DIST_V_TOP_EDGES: case DIST_ACTION_DIST_V_SPACING: cmp=cmp_by_top;      break;
	case DIST_ACTION_DIST_V_CENTERS:                                     cmp=cmp_by_center_y; break;
	case DIST_ACTION_DIST_V_BOTTOM_EDGES:                                cmp=cmp_by_bottom;   break;
	default: return false;
	}
	qsort(items->array, items->num, sizeof(item_info_t), cmp);
	const size_t n=items->num;
	const item_info_t *first=&items->array[0], *last=&items->array[n-1];
	if (action==DIST_ACTION_DIST_V_SPACING) {
		float total_h=0.0f;
		for (size_t i=0;i<n;++i) total_h+=items->array[i].height;
		float span=last->aabb.bottom-first->aabb.top;
		float gap=(span-total_h)/(float)(n-1);
		float cursor=first->aabb.top+first->height+gap;
		for (size_t i=1;i<n-1;++i) {
			item_info_t *cur=&items->array[i];
			float dy=cursor-cur->aabb.top;
			translate_item(cur->item,0.0f,dy);
			cursor+=cur->height+gap;
		}
		return true;
	}
	float start,end;
	switch(action) {
	case DIST_ACTION_DIST_V_TOP_EDGES:    start=first->aabb.top;    end=last->aabb.top;    break;
	case DIST_ACTION_DIST_V_CENTERS:      start=first->center_y;    end=last->center_y;    break;
	case DIST_ACTION_DIST_V_BOTTOM_EDGES: start=first->aabb.bottom; end=last->aabb.bottom; break;
	default: return false;
	}
	float step=(end-start)/(float)(n-1);
	for (size_t i=1;i<n-1;++i) {
		item_info_t *cur=&items->array[i];
		float target=start+step*(float)i;
		float cur_ref;
		switch(action) {
		case DIST_ACTION_DIST_V_TOP_EDGES:    cur_ref=cur->aabb.top;    break;
		case DIST_ACTION_DIST_V_CENTERS:      cur_ref=cur->center_y;    break;
		case DIST_ACTION_DIST_V_BOTTOM_EDGES: cur_ref=cur->aabb.bottom; break;
		default: return false;
		}
		translate_item(cur->item,0.0f,target-cur_ref);
	}
	return true;
}

static bool aabb_overlaps(const dist_aabb_t *a, const dist_aabb_t *b)
{
	return !(a->right<=b->left||b->right<=a->left||a->bottom<=b->top||b->bottom<=a->top);
}

static bool apply_random_scatter(item_list_t *items)
{
	if (items->num<1) return false;
	static bool seeded=false;
	if (!seeded){srand((unsigned int)time(NULL));seeded=true;}
	struct obs_video_info ovi;
	if (!obs_get_video_info(&ovi)) return false;
	const float canvas_w=(float)ovi.base_width;
	const float canvas_h=(float)ovi.base_height;
	dist_aabb_t *placed=bzalloc(sizeof(dist_aabb_t)*items->num);
	size_t placed_count=0;
	const int max_tries=60;
	for (size_t i=0;i<items->num;++i) {
		item_info_t *cur=&items->array[i];
		float w=cur->width, h=cur->height;
		float max_x=canvas_w-w; if(max_x<0.0f)max_x=0.0f;
		float max_y=canvas_h-h; if(max_y<0.0f)max_y=0.0f;
		float new_left=0.0f, new_top=0.0f;
		bool fits=false;
		for (int t=0;t<max_tries;++t) {
			new_left=((float)rand()/(float)RAND_MAX)*max_x;
			new_top =((float)rand()/(float)RAND_MAX)*max_y;
			dist_aabb_t cand={new_left,new_top,new_left+w,new_top+h};
			bool collides=false;
			for (size_t j=0;j<placed_count;++j)
				if (aabb_overlaps(&cand,&placed[j])){collides=true;break;}
			if (!collides){fits=true;break;}
		}
		if (!fits){new_left=((float)rand()/(float)RAND_MAX)*max_x;new_top=((float)rand()/(float)RAND_MAX)*max_y;}
		translate_item(cur->item,new_left-cur->aabb.left,new_top-cur->aabb.top);
		placed[placed_count]=(dist_aabb_t){new_left,new_top,new_left+w,new_top+h};
		placed_count++;
	}
	bfree(placed);
	return true;
}

static bool apply_same_size(item_list_t *items, dist_action_t action)
{
	if (items->num<2) return false;
	bool width  = (action==DIST_ACTION_SAME_WIDTH  || action==DIST_ACTION_SAME_SIZE);
	bool height = (action==DIST_ACTION_SAME_HEIGHT || action==DIST_ACTION_SAME_SIZE);
	obs_sceneitem_t *ref=items->array[0].item;
	for (size_t i=1;i<items->num;++i)
		set_same_dimension(ref,items->array[i].item,width,height);
	return true;
}

bool dist_apply(obs_scene_t *scene, dist_action_t action)
{
	if (!scene) return false;
	item_list_t list={0};
	obs_scene_enum_items(scene,enum_collect_cb,&list);
	bool ok=false;
	switch(action) {
	case DIST_ACTION_ALIGN_LEFT: case DIST_ACTION_ALIGN_HCENTER: case DIST_ACTION_ALIGN_RIGHT:
	case DIST_ACTION_ALIGN_TOP:  case DIST_ACTION_ALIGN_VCENTER: case DIST_ACTION_ALIGN_BOTTOM:
		ok=apply_align(&list,action); break;
	case DIST_ACTION_DIST_H_LEFT_EDGES: case DIST_ACTION_DIST_H_CENTERS:
	case DIST_ACTION_DIST_H_RIGHT_EDGES: case DIST_ACTION_DIST_H_SPACING:
		ok=apply_distribute_horizontal(&list,action); break;
	case DIST_ACTION_DIST_V_TOP_EDGES: case DIST_ACTION_DIST_V_CENTERS:
	case DIST_ACTION_DIST_V_BOTTOM_EDGES: case DIST_ACTION_DIST_V_SPACING:
		ok=apply_distribute_vertical(&list,action); break;
	case DIST_ACTION_SAME_WIDTH: case DIST_ACTION_SAME_HEIGHT: case DIST_ACTION_SAME_SIZE:
		ok=apply_same_size(&list,action); break;
	case DIST_ACTION_RANDOM_SCATTER:
		ok=apply_random_scatter(&list); break;
	}
	release_items(&list);
	return ok;
}
