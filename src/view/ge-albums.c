/*
* Copyright (c) 2000-2015 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#include "ge-albums.h"
#include "ge-gridview.h"
#include "ge-ui-util.h"
#include "ge-util.h"
#include "ge-data.h"
#include "ge-icon.h"
#include "ge-tile.h"
#include "ge-rotate-bg.h"
#include "ge-button.h"
#include "ge-main-view.h"
#include "ge-strings.h"

/* Only for local medias */
static void __ge_albums_create_thumb_cb(media_content_error_e error,
					const char *path, void *user_data)
{
	if (GE_FILE_EXISTS(path) && error == MEDIA_CONTENT_ERROR_NONE) {
		GE_CHECK(user_data);
		ge_cluster *album= (ge_cluster *)user_data;
		album->cover->item->b_create_thumb = false;
		GE_CHECK(album->griditem);
		elm_gengrid_item_update(album->griditem);
	} else {
		ge_dbgE("Error number[%d]", error);
	}
}

/* Use file to create new thumb if possible */
static int __ge_albums_create_thumb(ge_item *gitem, ge_cluster *album)
{
	GE_CHECK_VAL(gitem, -1);
	GE_CHECK_VAL(gitem->item, -1);
	GE_CHECK_VAL(gitem->item->file_url, -1);
	GE_CHECK_VAL(album, -1);

	if (GE_FILE_EXISTS(gitem->item->file_url)) {
		_ge_data_create_thumb(gitem->item, __ge_albums_create_thumb_cb,
				      album);
		return 0;
	}
	return -1;
}

static void __ge_split_albums_realized(void *data, Evas_Object *obj, void *ei)
{
	GE_CHECK(data);
	ge_ugdata *ugd = (ge_ugdata *)data;
	GE_CHECK(ei);
	Elm_Object_Item *it = (Elm_Object_Item *)ei;
	ge_cluster *album = elm_object_item_data_get(it);
	GE_CHECK(album);
	GE_CHECK(album->cluster);
	GE_CHECK(album->cover);
	GE_CHECK(album->cover->item);
	ge_dbg("realized");
	if (ugd->thumbs_d && ugd->thumbs_d->selected_uuid) {
		if (!strcmp(ugd->thumbs_d->selected_uuid, album->cluster->uuid)) {
			ugd->selected_griditem = it;
			elm_object_item_signal_emit((Elm_Object_Item *)ei, "elm,state,focused", "elm");
		} else {
			elm_object_item_signal_emit((Elm_Object_Item *)ei, "elm,state,unfocused", "elm");
		}
	}
	int sel_cnt;
	_ge_data_get_album_sel_cnt(ugd, album->cluster->uuid, &sel_cnt);
	if (sel_cnt > 0) {
		elm_object_item_signal_emit((Elm_Object_Item *)ei,
				"elm,state,elm.text.badge,visible",
				"elm");
		album->sel_cnt = sel_cnt;
	} else {
		album->sel_cnt = 0;
		elm_object_item_signal_emit((Elm_Object_Item *)ei,
				"elm,state,elm.text.badge,hidden",
				"elm");
	}
}

static void __ge_albums_realized(void *data, Evas_Object *obj, void *ei)
{
	GE_CHECK(ei);
	Elm_Object_Item *it = (Elm_Object_Item *)ei;
	ge_cluster *album = elm_object_item_data_get(it);
	GE_CHECK(album);
	GE_CHECK(album->cluster);
	GE_CHECK(album->cover);
	GE_CHECK(album->cover->item);

	ge_dbg("realized");
	if (!GE_FILE_EXISTS(album->cover->item->thumb_url) &&
	    GE_FILE_EXISTS(album->cover->item->file_url) &&
	    (album->cluster->type == GE_PHONE ||
	     album->cluster->type == GE_MMC ||
	     album->cluster->type == GE_ALL))
		__ge_albums_create_thumb(album->cover, album);

	GE_CHECK(album->ugd);
	ge_ugdata *ugd = album->ugd;
	if (ugd->b_multifile) {
		if (album->sel_cnt > 0)
			elm_object_item_signal_emit(album->griditem,
						    "elm,state,elm.text.badge,visible",
						    "elm");
		else
			elm_object_item_signal_emit(album->griditem,
						    "elm,state,elm.text.badge,hidden",
						    "elm");
	}
}

static void __ge_albums_unrealized(void *data, Evas_Object *obj, void *ei)
{
	ge_dbg("unrealized");
	GE_CHECK(ei);
	Elm_Object_Item *it = (Elm_Object_Item *)ei;
	ge_cluster *album = elm_object_item_data_get(it);
	GE_CHECK(album);
	GE_CHECK(album->cluster);
	GE_CHECK(album->cover);
	GE_CHECK(album->cover->item);

	/* Checking for local files only */
	if (album->cluster->type == GE_PHONE ||
	    album->cluster->type == GE_MMC ||
	    album->cluster->type == GE_ALL) {
		if (album->cover->item->b_create_thumb) {
			_ge_data_cancel_thumb(album->cover->item);

			ge_dbgW("in the if part");
		}
	}
}

static int __ge_albums_open_album(ge_cluster *album)
{
	GE_CHECK_VAL(album, -1);
	GE_CHECK_VAL(album->cluster, -1);
	GE_CHECK_VAL(album->ugd, -1);
	ge_ugdata *ugd = album->ugd;
	ge_sdbg("Album: %s", album->cluster->display_name);

	if (_ge_get_view_mode(ugd) != GE_VIEW_ALBUMS) {
		ge_dbgE("Wrong mode!");
		ugd->view_mode = GE_VIEW_ALBUMS;
		//return -1;
		ge_dbgE("new mode album view is assigned");
	}

	if (ugd->album_select_mode == GE_ALBUM_SELECT_T_ONE) {
		ge_dbg("One album selected, return album id");
		app_control_add_extra_data(ugd->service,
				       GE_ALBUM_SELECT_RETURN_PATH,
				       album->cluster->path);
		ge_dbg("return folder-path: %s", album->cluster->path);
		ug_send_result_full(ugd->ug, ugd->service, APP_CONTROL_RESULT_SUCCEEDED);
		if (!ugd->is_attach_panel) {
			ug_destroy_me(ugd->ug);
			ugd->ug = NULL;
		}
		return 0;
	}

	/* Add thumbnails view */
	_ge_grid_create_thumb(album);
	ge_dbg("Done");
	return 0;
}

static int __ge_split_albums_open_album(ge_cluster *album)
{
	GE_CHECK_VAL(album, -1);
	GE_CHECK_VAL(album->cluster, -1);
	GE_CHECK_VAL(album->ugd, -1);
	ge_sdbg("Album: %s", album->cluster->display_name);

	/* Add thumbnails view */
	_ge_grid_create_split_view_thumb(album);
	ge_dbg("Done");
	return 0;
}

/* Add idler to make mouse click sound, other sound couldn' be played */
static Eina_Bool __ge_albums_sel_idler_cb(void *data)
{
	ge_dbg("Select album ---");
	GE_CHECK_FALSE(data);
	ge_cluster *album_item = (ge_cluster*)data;
	GE_CHECK_FALSE(album_item->cluster);
	GE_CHECK_FALSE(album_item->ugd);
	ge_ugdata *ugd = album_item->ugd;
	if (ugd->ug == NULL) {
		ge_dbg("UG already destroyed, return!");
		goto GE_ALBUMS_DONE;
	}

	if (album_item->cover) {
		_ge_data_util_free_item(album_item->cover);
		album_item->cover = NULL;
	}

	__ge_albums_open_album(album_item);

 GE_ALBUMS_DONE:

	ecore_idler_del(ugd->sel_album_idler);
	ugd->sel_album_idler = NULL;
	ge_dbg("Select album +++");
	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool __ge_split_albums_sel_idler_cb(void *data)
{
	ge_dbg("Select album ---");
	GE_CHECK_FALSE(data);
	ge_cluster *album_item = (ge_cluster*)data;
	GE_CHECK_FALSE(album_item->cluster);
	GE_CHECK_FALSE(album_item->ugd);
	ge_ugdata *ugd = album_item->ugd;
	if (ugd->ug == NULL) {
		ge_dbg("UG already destroyed, return!");
		goto GE_ALBUMS_DONE;
	}
	if (album_item->cover) {
		_ge_data_util_free_item(album_item->cover);
		album_item->cover = NULL;
	}
	__ge_split_albums_open_album(album_item);

 GE_ALBUMS_DONE:
	ecore_idler_del(ugd->sel_album_idler);
	ugd->sel_album_idler = NULL;
	ge_dbg("Select album +++");
	return ECORE_CALLBACK_CANCEL;
}

static void __ge_albums_sel_cb(void *data, Evas_Object *obj, void *ei)
{
	GE_CHECK(data);
	ge_cluster *album_item = (ge_cluster*)data;
	GE_CHECK(album_item->cluster);
	GE_CHECK(album_item->ugd);
	ge_ugdata *ugd = album_item->ugd;
	if (ugd->ug == NULL) {
		ge_dbg("UG already destroyed, return!");
		return;
	}
	ugd->album_item = album_item;
	ge_dbg("");
	if (ugd->sel_album_idler) {
		ge_dbg("Has selected an album");
		return;
	}

	elm_gengrid_item_selected_set(ei, EINA_FALSE);

	Ecore_Idler *idl = NULL;
	idl = ecore_idler_add(__ge_albums_sel_idler_cb, data);
	ugd->sel_album_idler = idl;
	/* Save scroller position before clearing gengrid */
	_ge_ui_save_scroller_pos(obj);
}

static void __ge_split_albums_sel_cb(void *data, Evas_Object *obj, void *ei)
{
	GE_CHECK(data);
	ge_cluster *album_item = (ge_cluster*)data;
	GE_CHECK(album_item->cluster);
	GE_CHECK(album_item->ugd);
	ge_ugdata *ugd = album_item->ugd;
	if (ugd->ug == NULL) {
		ge_dbg("UG already destroyed, return!");
		return;
	}
	ugd->album_item = album_item;
	ge_dbg("");
	if (ugd->sel_album_idler) {
		ge_dbg("Has selected an album");
		return;
	}
	elm_gengrid_item_selected_set(ei, EINA_FALSE);
	Ecore_Idler *idl = NULL;
	idl = ecore_idler_add(__ge_split_albums_sel_idler_cb, data);
	ugd->sel_album_idler = idl;
	/* Save scroller position before clearing gengrid */
	_ge_ui_save_scroller_pos(obj);
}

static char *__ge_albums_get_text(void *data, Evas_Object *obj, const char *part)
{
	GE_CHECK_NULL(part);
	GE_CHECK_NULL(data);
	ge_cluster *album = (ge_cluster *)data;
	GE_CHECK_NULL(album->cluster);
	GE_CHECK_NULL(album->cluster->uuid);
	GE_CHECK_NULL(album->ugd);
	ge_ugdata *ugd = album->ugd;
	char buf[GE_FILE_PATH_LEN_MAX] = { 0, };

	if (!g_strcmp0(part, "elm.text.name")) {
		GE_CHECK_NULL(album->cluster->display_name);
		if (_ge_data_is_root_path(album->cluster->path)) {
			snprintf(buf, sizeof(buf), "%s", GE_ALBUM_ROOT_NAME);
			buf[strlen(buf)] = '\0';
		} else if (album->cluster->display_name &&
			  strlen(album->cluster->display_name)) {
			char *new_name = _ge_ui_get_i18n_album_name(album->cluster);
			snprintf(buf, sizeof(buf), "%s", new_name);
			buf[strlen(buf)] = '\0';
		}

		/* Show blue folder name */
		if (!g_strcmp0(album->cluster->uuid, GE_ALBUM_ALL_ID)) {
			Elm_Object_Item *grid_it = album->griditem;
		    	Evas_Object *it_obj = NULL;
			it_obj = elm_object_item_widget_get(grid_it);
			GE_CHECK_NULL(it_obj);
			edje_object_signal_emit(it_obj, "elm,name,show,blue",
						"elm");
			edje_object_message_signal_process(it_obj);
		}
	} else if (!g_strcmp0(part, "elm.text.date")) {
		if (album->cover) {
			_ge_data_util_free_item(album->cover);
			album->cover = NULL;
		}

		ge_item *item = NULL;
		_ge_data_get_album_cover(ugd, album, &item,
					 MEDIA_CONTENT_ORDER_DESC);
		if (item == NULL || item->item == NULL) {
			album->cover_thumbs_cnt = 0;
			_ge_data_util_free_item(item);
			return NULL;
		}

		album->cover = item;
		album->cover_thumbs_cnt = GE_ALBUM_COVER_THUMB_NUM;
	} else if (!g_strcmp0(part, "elm.text.count")) {
		_ge_data_update_items_cnt(ugd, album);
		snprintf(buf, sizeof(buf), "%d", (int)(album->cluster->count));
		buf[strlen(buf)] = '\0';
	} else if (!g_strcmp0(part, "elm.text.badge") && ugd->b_multifile) {
		int sel_cnt = 0;
		_ge_data_get_album_sel_cnt(ugd, album->cluster->uuid, &sel_cnt);
		ge_dbg("count :%d", sel_cnt);
		if (sel_cnt > 0) {
			album->sel_cnt = sel_cnt;
			snprintf(buf, sizeof(buf), "%d", sel_cnt);
		} else {
			album->sel_cnt = 0;
		}
	}
	return strdup(buf);
}

static char *__ge_split_albums_get_text(void *data, Evas_Object *obj, const char *part)
{
	GE_CHECK_NULL(part);
	GE_CHECK_NULL(data);
	ge_cluster *album = (ge_cluster *)data;
	GE_CHECK_NULL(album->cluster);
	GE_CHECK_NULL(album->cluster->uuid);
	GE_CHECK_NULL(album->ugd);
	ge_ugdata *ugd = album->ugd;
	char buf[GE_FILE_PATH_LEN_MAX] = { 0, };

	if (!g_strcmp0(part, "elm.text.date")) {
		if (album->cover) {
			_ge_data_util_free_item(album->cover);
			album->cover = NULL;
		}

		ge_item *item = NULL;
		_ge_data_get_album_cover(ugd, album, &item,
				MEDIA_CONTENT_ORDER_DESC);
		if (item == NULL || item->item == NULL) {
			album->cover_thumbs_cnt = 0;
			_ge_data_util_free_item(item);
			return NULL;
		}

		album->cover = item;
		album->cover_thumbs_cnt = GE_ALBUM_COVER_THUMB_NUM;
	} else if (!g_strcmp0(part, "elm.text.count")) {
		_ge_data_update_items_cnt(ugd, album);
		snprintf(buf, sizeof(buf), "%d", (int)(album->cluster->count));
		buf[strlen(buf)] = '\0';
	} else if (!g_strcmp0(part, "elm.text.badge") && ugd->b_multifile) {
		int sel_cnt = 0;
		_ge_data_get_album_sel_cnt(ugd, album->cluster->uuid, &sel_cnt);
		ge_dbg("count :%d", sel_cnt);
		if (sel_cnt > 0) {
			elm_object_item_signal_emit(album->griditem,
					"elm,state,elm.text.badge,visible",
					"elm");
			album->sel_cnt = sel_cnt;
			snprintf(buf, sizeof(buf), "%d", sel_cnt);
		} else {
			album->sel_cnt = 0;
			elm_object_item_signal_emit(album->griditem,
					"elm,state,elm.text.badge,hidden",
					"elm");
		}
	}
	return strdup(buf);
}

static ge_icon_type __ge_albums_set_bg_file(Evas_Object *bg, void *data)
{
	GE_CHECK_VAL(data, -1);
	ge_item *git =  (ge_item *)data;
	GE_CHECK_VAL(git->album, -1);
	ge_cluster *album = git->album;
	char *bg_path = GE_ICON_NO_THUMBNAIL;
	ge_icon_type ret_val = GE_ICON_CORRUPTED_FILE;

	if (git == NULL || git->item == NULL) {
		ge_dbgE("Invalid item :%p", git);
		goto GE_ALBUMS_FAILED;
	}

	ret_val= GE_ICON_NORMAL;
	if (GE_FILE_EXISTS(git->item->thumb_url))
		bg_path = git->item->thumb_url;
	else if (album && (album->cluster->type == GE_MMC ||
		 album->cluster->type == GE_PHONE ||
		 album->cluster->type == GE_ALL))
		__ge_albums_create_thumb(git, album);
	else
		ret_val = -1;

 GE_ALBUMS_FAILED:

#ifdef _USE_ROTATE_BG_GE
	_ge_rotate_bg_set_image_file(bg, bg_path);
#else
	elm_bg_file_set(bg, bg_path, NULL);
#endif

	return ret_val;
}

static Evas_Object *__ge_albums_get_type_icon(Evas_Object *obj, ge_cluster *album)
{
	GE_CHECK_NULL(album);
	GE_CHECK_NULL(album->cluster);
	GE_CHECK_NULL(obj);
	Evas_Object *_obj = NULL;

	if (_ge_data_is_camera_album(album->cluster))
		_obj = _ge_tile_show_part_type_icon(obj,
						    GE_TILE_TYPE_CAMERA);
	else if (_ge_data_is_default_album(GE_ALBUM_DOWNLOADS_NAME, album->cluster))
		_obj = _ge_tile_show_part_type_icon(obj,
						    GE_TILE_TYPE_DOWNLOAD);
	else
		_obj = _ge_tile_show_part_type_icon(obj,
						    GE_TILE_TYPE_FOLDER);
	return _obj;
}

static Evas_Object *__ge_albums_get_sd_icon(Evas_Object *obj, ge_cluster *album)
{
	GE_CHECK_NULL(album);
	GE_CHECK_NULL(album->cluster);
	GE_CHECK_NULL(obj);
	Evas_Object *_obj = NULL;

	_obj = _ge_tile_show_part_type_icon(obj,
							GE_TILE_TYPE_MMC);
	return _obj;
}

static Evas_Object *__ge_albums_get_content(void *data, Evas_Object *obj, const char *part)
{
	GE_CHECK_NULL(part);
	GE_CHECK_NULL(strlen(part));
	GE_CHECK_NULL(data);
	ge_cluster *album = (ge_cluster *)data;
	GE_CHECK_NULL(album->cluster);
	GE_CHECK_NULL(album->cluster->uuid);

	Evas_Object *_obj = NULL;
	if (!g_strcmp0(part, GE_TILE_ICON)) {
		_obj = _ge_tile_show_part_icon(obj, part,
					       album->cover_thumbs_cnt,
					       __ge_albums_set_bg_file,
					       (void *)album->cover);
	} else if (!g_strcmp0(part, GE_TILE_TYPE_ICON)) {
		_obj = __ge_albums_get_type_icon(obj, album);
	}
	else if (!g_strcmp0(part, GE_SD_CARD_TYPE_ICON)) {
		if (album->cluster->type == GE_MMC) {
			_obj = __ge_albums_get_sd_icon(obj, album);
		}
	}
	return _obj;
}

static void
_ge_count_set(Evas_Object *layout, const char *text)
{
	Evas_Object *edje;
	Edje_Message_Float *msg;
	if (text) {
		elm_layout_text_set(layout, "elm.sub.text", text);
	}
	else {
		elm_layout_text_set(layout, "elm.sub.text", "");
	}
	edje = elm_layout_edje_get(layout);
	msg = calloc(1, sizeof(Edje_Message_Float));
	if (msg) {
		msg->val = elm_config_scale_get();
		edje_object_message_send(edje, EDJE_MESSAGE_FLOAT, 0, msg);
	}
}

static Evas_Object *__ge_split_albums_get_content(void *data, Evas_Object *obj, const char *part)
{
	GE_CHECK_NULL(part);
	GE_CHECK_NULL(strlen(part));
	GE_CHECK_NULL(data);
	ge_cluster *album = (ge_cluster *)data;
	GE_CHECK_NULL(album->cluster);
	GE_CHECK_NULL(album->cluster->uuid);
	GE_CHECK_NULL(album->ugd);
	ge_ugdata *ugd = album->ugd;
	ge_dbg("");

	Evas_Object *_obj = NULL;
	if (!g_strcmp0(part, "elm.swallow.name_layout")) {
		GE_CHECK_NULL(album->cluster->display_name);
		char cnt[GE_FILE_PATH_LEN_MAX] = { 0, };
		char *i18n_name = NULL;
		if (_ge_data_is_root_path(album->cluster->path)) {
			i18n_name = GE_ALBUM_ROOT_NAME;
		} else if (album->cluster->display_name &&
				strlen(album->cluster->display_name)) {
			i18n_name = _ge_ui_get_i18n_album_name(album->cluster);
		}
		_ge_data_update_items_cnt(ugd, album);
		Evas_Object *layout = elm_layout_add(obj);
		elm_layout_file_set(layout, GE_EDJ_FILE, "ceter_algined_text_layout_split_view");
		elm_layout_text_set(layout, "elm.text", _ge_str(i18n_name));
		snprintf(cnt, sizeof(cnt), "(%d)", (int)(album->cluster->count));
		_ge_count_set(layout, cnt);
		return layout;
	} else if (!g_strcmp0(part, GE_TILE_ICON)) {
		_obj = _ge_tile_show_part_icon(obj, part,
				album->cover_thumbs_cnt,
				__ge_albums_set_bg_file,
				(void *)album->cover);
	} else if (!g_strcmp0(part, GE_SD_CARD_TYPE_ICON)) {
		if (album->cluster->type == GE_MMC) {
			_obj = __ge_albums_get_sd_icon(obj, album);
		}
	}
	return _obj;
}

static int __ge_albums_append(ge_ugdata *ugd, ge_cluster *album)
{
	GE_CHECK_VAL(album, -1);
	GE_CHECK_VAL(album->cluster, -1);
	GE_CHECK_VAL(ugd, -1);

	album->griditem = elm_gengrid_item_append(ugd->albums_view,
						  ugd->album_gic, album,
						  __ge_albums_sel_cb, album);
	ge_sdbg("Append [%s], id[%s]", album->cluster->display_name,
	       album->cluster->uuid);
	_ge_tile_update_item_size(ugd, ugd->albums_view, ugd->rotate_mode,
				  false);
	album->index = elm_gengrid_items_count(ugd->albums_view);
	return 0;
}

int __ge_split_albums_append(ge_ugdata *ugd, ge_cluster *album)
{
	GE_CHECK_VAL(album, -1);
	GE_CHECK_VAL(album->cluster, -1);
	GE_CHECK_VAL(ugd, -1);

	album->griditem = elm_gengrid_item_append(ugd->albums_view,
			ugd->album_gic, album,
			__ge_split_albums_sel_cb, album);
	ge_sdbg("Append [%s], id[%s]", album->cluster->display_name,
			album->cluster->uuid);
	_ge_tile_update_item_size(ugd, ugd->albums_view, ugd->rotate_mode,
			false);
	album->index = elm_gengrid_items_count(ugd->albums_view);
	return 0;
}

#if 0
static Eina_Bool __ge_albums_append_idler_cb(void *data)
{
	ge_dbg("Append album ---");
	GE_CHECK_VAL(data, -1);
	ge_ugdata *ugd = (ge_ugdata *)data;
	GE_CHECK_VAL(ugd->cluster_list, -1);

	int old_cnt = eina_list_count(ugd->cluster_list->clist);
	_ge_data_get_clusters(ugd, ugd->cluster_list->data_type);
	ugd->cluster_list->data_type = GE_ALBUM_DATA_NONE;
	int new_cnt = eina_list_count(ugd->cluster_list->clist);
	if (old_cnt != new_cnt)
		_ge_tile_update_item_size(ugd, ugd->albums_view,
					  ugd->rotate_mode, false);
	ecore_idler_del(ugd->album_idler);
	ugd->album_idler = NULL;
	ge_dbg("Append album +++");
	return ECORE_CALLBACK_CANCEL;
}
#endif

Eina_Bool __ge_split_albums_append_idler_cb(void *data)
{
	ge_dbg("Append album ---");
	GE_CHECK_VAL(data, -1);
	ge_ugdata *ugd = (ge_ugdata *)data;
	GE_CHECK_VAL(ugd->cluster_list, -1);

	int old_cnt = eina_list_count(ugd->cluster_list->clist);
	ge_dbg("Albums list length old: %d", old_cnt);
	_ge_data_get_clusters(ugd, ugd->cluster_list->data_type);
	ugd->cluster_list->data_type = GE_ALBUM_DATA_NONE;
	int new_cnt = eina_list_count(ugd->cluster_list->clist);
	ge_dbg("Albums list length new: %d", new_cnt);
	ecore_idler_del(ugd->album_idler);
	ugd->album_idler = NULL;
	ge_dbg("Append album +++");
	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool __ge_albums_create_idler_cb(void *data)
{
	ge_dbg("Create album ---");
	GE_CHECK_VAL(data, -1);
	ge_ugdata *ugd = (ge_ugdata *)data;
	int length = eina_list_count(ugd->cluster_list->clist);
	int i = 0;
	ge_cluster *album = NULL;
	ge_dbg("Albums list length: %d", length);

	/* First 8 albums is got from DB, and plus 'All albums', it's 9 totally */
	for (i = GE_ALBUMS_FIRST_COUNT + 1; i < length; i++) {
		album = eina_list_nth(ugd->cluster_list->clist, i);
		__ge_albums_append(ugd, album);
	}
	/* Restore previous position of scroller */
	_ge_ui_restore_scroller_pos(ugd->albums_view);

	ecore_idler_del(ugd->album_idler);
	ugd->album_idler = NULL;
	ge_dbg("Create album +++");
	return ECORE_CALLBACK_CANCEL;
}

Eina_Bool __ge_split_view_albums_create_idler_cb(void *data)
{
	ge_dbg("Create album ---");
	GE_CHECK_VAL(data, -1);
	ge_ugdata *ugd = (ge_ugdata *)data;
	int length = eina_list_count(ugd->cluster_list->clist);
	int i = 0;
	ge_cluster *album = NULL;
	ge_dbg("Albums list length: %d", length);

	/* First 8 albums is got from DB, and plus 'All albums', it's 9 totally */
	for (i = GE_ALBUMS_FIRST_COUNT + 1; i < length; i++) {
		album = eina_list_nth(ugd->cluster_list->clist, i);
		__ge_split_albums_append(ugd, album);
	}
	/* Restore previous position of scroller */
	_ge_ui_restore_scroller_pos(ugd->albums_view);

	ecore_idler_del(ugd->album_idler);
	ugd->album_idler = NULL;
	ge_dbg("Create album +++");
	return ECORE_CALLBACK_CANCEL;
}

int __ge_split_view_append_albums(ge_ugdata *ugd, Evas_Object *parent, bool is_update)
{
	GE_CHECK_VAL(parent, -1);
	GE_CHECK_VAL(ugd, -1);
	int i = 0;
	ge_cluster *album = NULL;
	int length = 0;
	if (elm_gengrid_items_count(parent) > 0) {
		/* Save scroller position before clearing gengrid */
		_ge_ui_save_scroller_pos(parent);
		elm_gengrid_clear(parent);
	}
	if (is_update)
		_ge_data_get_clusters(ugd, GE_ALBUM_DATA_LOCAL);
	if (ugd->cluster_list && ugd->cluster_list->clist)
		length = eina_list_count(ugd->cluster_list->clist);
	else
		return -1;
	ge_dbg("Albums list length: %d", length);

	if (ugd->th)
		elm_object_theme_set(parent, ugd->th);

	int grid_cnt = 0;
	Elm_Gengrid_Item_Class *gic = elm_gengrid_item_class_new();
	if (gic == NULL) {
		return -1;
	}

	gic->item_style = "gallery_efl/albums_split_view";
	gic->func.text_get = __ge_split_albums_get_text;
	gic->func.content_get = __ge_split_albums_get_content;

	for (i = 0; i < length; i++) {
		album = eina_list_nth(ugd->cluster_list->clist, i);
		if (!album || !album->cluster || !album->cluster->display_name) {
			ge_dbgE("Invalid parameter, return ERROR code!");
			elm_gengrid_clear(parent);

			if (gic) {
				elm_gengrid_item_class_free(gic);
				gic = NULL;
			}
			return -1;
		}

		if (album->cluster->type == GE_ALL) {
			continue;
		}

		album->griditem = elm_gengrid_item_append(parent,
				gic,
				album,
				__ge_split_albums_sel_cb,
				album);

		if (ugd->selected_album == NULL && !strcmp(album->cluster->uuid, ugd->album_item->cluster->uuid)) {
			album->select = true;
			ugd->selected_album = (char *)malloc(strlen(ugd->album_item->cluster->uuid) + 1);
			if (ugd->selected_album) {
				strncpy(ugd->selected_album, ugd->album_item->cluster->uuid, strlen(ugd->album_item->cluster->uuid));
			}
		} else {
			album->select = false;
		}

		if (ugd->selected_album != NULL && !strcmp(album->cluster->uuid, ugd->selected_album)) {
			album->select = true;
		} else {
			album->select = false;
		}

		album->index = grid_cnt;
		grid_cnt++;
		ge_sdbg("Append [%s], id=%s.", album->cluster->display_name,
				album->cluster->uuid);
	}
	/* Restore previous position of scroller */
	_ge_ui_restore_scroller_pos(parent);

	if (gic) {
		elm_gengrid_item_class_free(gic);
		gic = NULL;
	}
	/* NOT jsut for updating view, but for updating view and data together */
	if (ugd->cluster_list->data_type == GE_ALBUM_DATA_LOCAL ||
			ugd->cluster_list->data_type == GE_ALBUM_DATA_WEB) {
		if (ugd->album_idler) {
			ecore_idler_del(ugd->album_idler);
			ugd->album_idler = NULL;
		}
		Ecore_Idler *idl = NULL;
		ugd->album_idler = idl;
	}
	if (grid_cnt) {
		return 0;
	} else {
		ge_dbgW("None albums appended to view!");
		return -1;
	}
}

static int __ge_albums_append_albums(ge_ugdata *ugd, Evas_Object *parent, bool is_update)
{
	GE_CHECK_VAL(parent, -1);
	GE_CHECK_VAL(ugd, -1);
	int i = 0;
	ge_cluster *album = NULL;
	int length = 0;

	if (elm_gengrid_items_count(parent) > 0) {
		/* Save scroller position before clearing gengrid */
		_ge_ui_save_scroller_pos(parent);
		elm_gengrid_clear(parent);
	}
	if (is_update)
		_ge_data_get_clusters(ugd, GE_ALBUM_DATA_LOCAL);
	if (ugd->cluster_list && ugd->cluster_list->clist)
		length = eina_list_count(ugd->cluster_list->clist);
	else
		return -1;
	ge_dbg("Albums list length: %d", length);

	if (ugd->th)
		elm_object_theme_set(parent, ugd->th);

	/* Jus for updating view, not updating data and view together */
	if (ugd->cluster_list->data_type == GE_ALBUM_DATA_NONE &&
	    length > GE_ALBUMS_FIRST_COUNT + 1) {
		length = GE_ALBUMS_FIRST_COUNT + 1;
		if (ugd->album_idler) {
			ecore_idler_del(ugd->album_idler);
			ugd->album_idler = NULL;
		}
		Ecore_Idler *idl = NULL;
		idl = ecore_idler_add(__ge_albums_create_idler_cb, ugd);
		ugd->album_idler = idl;
	}

	int grid_cnt = 0;
	for (i = 0; i < length; i++) {
		album = eina_list_nth(ugd->cluster_list->clist, i);
		GE_CHECK_VAL(album, -1);
		GE_CHECK_VAL(album->cluster, -1);
		GE_CHECK_VAL(album->cluster->display_name, -1);

		if (album->cluster->type == GE_ALL)
			continue;

		album->griditem = elm_gengrid_item_append(parent,
							  ugd->album_gic,
							  album,
							  __ge_albums_sel_cb,
							  album);
		album->index = grid_cnt;
		grid_cnt++;
		ge_sdbg("Append [%s], id=%s.", album->cluster->display_name,
		       album->cluster->uuid);
	}
	/* Restore previous position of scroller */
	_ge_ui_restore_scroller_pos(parent);

	/* NOT jsut for updating view, but for updating view and data together */
	if (ugd->cluster_list->data_type == GE_ALBUM_DATA_LOCAL ||
	    ugd->cluster_list->data_type == GE_ALBUM_DATA_WEB) {
		if (ugd->album_idler) {
			ecore_idler_del(ugd->album_idler);
			ugd->album_idler = NULL;
		}
		Ecore_Idler *idl = NULL;
		ugd->album_idler = idl;
	}

	if (grid_cnt) {
		_ge_tile_update_item_size(ugd, parent, ugd->rotate_mode, false);
		return 0;
	} else {
		ge_dbgW("None albums appended to view!");
		return -1;
	}
}

static int __ge_albums_del_cbs(Evas_Object *view)
{
	if (view == NULL)
		return -1;
	ge_dbg("Delete albums callbacks!");
	evas_object_smart_callback_del(view, "realized",
				       __ge_albums_realized);
	evas_object_smart_callback_del(view, "unrealized",
				       __ge_albums_unrealized);
	return 0;
}

static int __ge_albums_rotate_view(ge_ugdata *ugd)
{
	if (ugd->albums_view && ugd->albums_view != ugd->nocontents) {
		_ge_tile_update_item_size(ugd, ugd->albums_view,
					  ugd->rotate_mode, false);
		return 0;
	}
	return -1;
}

/* Free data after layout deleted */
static void __ge_albums_del_layout_cb(void *data, Evas *e, Evas_Object *obj,
					 void *ei)
{
	ge_dbg("Delete layout ---");
/*	evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL,
				       __ge_albums_del_layout_cb);*/
	GE_CHECK(data);
	ge_ugdata *ugd = (ge_ugdata *)data;

	if (ugd->album_idler) {
		ecore_idler_del(ugd->album_idler);
		ugd->album_idler = NULL;
	}
	if (ugd->sel_album_idler) {
		ecore_idler_del(ugd->sel_album_idler);
		ugd->sel_album_idler = NULL;
	}
	if (ugd->albums_view && ugd->albums_view != ugd->nocontents) {
		elm_gengrid_clear(ugd->albums_view);
		__ge_albums_del_cbs(ugd->albums_view);
		_ge_ui_del_scroller_pos(ugd->albums_view);
	}
	ugd->albums_view = NULL;
	ugd->albums_view_ly = NULL;
	if (ugd->album_gic) {
		elm_gengrid_item_class_free(ugd->album_gic);
		ugd->album_gic = NULL;
	}
	ugd->rotate_cbs = eina_list_remove(ugd->rotate_cbs,
					   __ge_albums_rotate_view);
	/* Clear view data */
	_ge_data_free_sel_albums(ugd);
	_ge_data_free_clusters(data);
	ge_dbg("Delete layout +++");
}

static void _ge_grid_move_stop_cb(void *data, Evas_Object *obj, void *ei)
{
	ge_dbg("");
	GE_CHECK(data);
	ge_ugdata *ugd = (ge_ugdata *)data;
	int x, y, w, h;
	int ret;
	elm_scroller_region_get(obj, &x, &y, &w, &h);

	app_control_h app_control = NULL;
	ret = app_control_create(&app_control);
	if (ret == APP_CONTROL_ERROR_NONE) {
		if (y == 0) {
			app_control_add_extra_data(app_control, ATTACH_PANEL_FLICK_MODE_KEY, ATTACH_PANEL_FLICK_MODE_ENABLE);
		} else {
			app_control_add_extra_data(app_control, ATTACH_PANEL_FLICK_MODE_KEY, ATTACH_PANEL_FLICK_MODE_DISABLE);
		}
		ret = ug_send_result_full(ugd->ug, app_control, APP_CONTROL_RESULT_SUCCEEDED);
	}
	app_control_destroy(app_control);
}
Evas_Object* __ge_add_albums_split_view(ge_ugdata *ugd, Evas_Object *parent)
{
	ge_dbg("");
	GE_CHECK_NULL(parent);
	GE_CHECK_NULL(ugd);
	Evas_Object *grid = elm_gengrid_add(parent);
	GE_CHECK_NULL(grid);
	_ge_ui_reset_scroller_pos(grid);
	elm_gengrid_align_set(grid, 0.5f, 0.0);
	elm_gengrid_horizontal_set(grid, EINA_FALSE);
	elm_scroller_bounce_set(grid, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(grid, ELM_SCROLLER_POLICY_OFF,
			ELM_SCROLLER_POLICY_AUTO);
	elm_gengrid_multi_select_set(grid, EINA_TRUE);
	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(grid, "realized", __ge_split_albums_realized,
			ugd);
	if (__ge_split_view_append_albums(ugd, grid, false) != 0) {
		ge_dbgW("Failed to append album items!");
		__ge_albums_del_cbs(grid);
		evas_object_del(grid);
		grid = NULL;
	} else {
		evas_object_show(grid);
	}
	return grid;
}

static Evas_Object* __ge_albums_add_view(ge_ugdata *ugd, Evas_Object *parent)
{
	ge_dbg("");
	GE_CHECK_NULL(parent);
	GE_CHECK_NULL(ugd);

	Evas_Object *grid = elm_gengrid_add(parent);
	GE_CHECK_NULL(grid);

#ifdef _USE_CUSTOMIZED_GENGRID_STYLE
	elm_object_style_set(grid, GE_GENGRID_STYLE_GALLERY);
#endif
	_ge_ui_reset_scroller_pos(grid);
	elm_gengrid_align_set(grid, 0.0, 0.0);
#ifdef _USE_SCROL_HORIZONRAL
	/* horizontal scrolling */
	elm_gengrid_horizontal_set(grid, EINA_TRUE);
	elm_scroller_bounce_set(grid, EINA_TRUE, EINA_FALSE);
#else
	elm_gengrid_horizontal_set(grid, EINA_FALSE);
	elm_scroller_bounce_set(grid, EINA_FALSE, EINA_TRUE);
#endif
	elm_scroller_policy_set(grid, ELM_SCROLLER_POLICY_OFF,
				ELM_SCROLLER_POLICY_AUTO);
	elm_gengrid_multi_select_set(grid, EINA_TRUE);
	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(grid, "unrealized",
				       __ge_albums_unrealized, ugd);
	evas_object_smart_callback_add(grid, "realized", __ge_albums_realized,
				       ugd);
	if (ugd->is_attach_panel && (ugd->attach_panel_display_mode != ATTACH_PANEL_FULL_MODE)) {
		elm_scroller_movement_block_set(grid, ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL);
	}
	if (ugd->is_attach_panel) {
		evas_object_smart_callback_add(grid, "scroll,anim,stop",
				_ge_grid_move_stop_cb, ugd);
		evas_object_smart_callback_add(grid, "scroll,drag,stop",
				_ge_grid_move_stop_cb, ugd);
	}

	if (__ge_albums_append_albums(ugd, grid, false) != 0) {
		ge_dbgW("Failed to append album items!");
		__ge_albums_del_cbs(grid);
		evas_object_del(grid);
		grid = NULL;
		goto ALBUMS_FAILED;
	} else {
		evas_object_show(grid);
	}

	return grid;

ALBUMS_FAILED:
	/* Show no contents if none albums exist */
	ge_dbgW("@@@@@@@  To create nocontents view  @@@@@@@@");
	ugd->nocontents = ge_ui_create_nocontents(ugd);
	ugd->albums_view = ugd->nocontents;
	evas_object_show(ugd->nocontents);

	return ugd->nocontents;
}

static Eina_Bool __ge_main_back_cb(void *data, Elm_Object_Item *it)
{
	ge_dbg("");
	GE_CHECK_FALSE(data);
	ge_ugdata *ugd = (ge_ugdata *)data;

	void *pop_cb = evas_object_data_get(ugd->naviframe,
					    GE_NAVIFRAME_POP_CB_KEY);
	if (pop_cb) {
		Eina_Bool (*_pop_cb) (void *ugd);
		_pop_cb = pop_cb;

		if (_pop_cb(ugd)) {
			/* Just pop edit view, dont destroy me */
			return EINA_FALSE;
		}
	}

	app_control_add_extra_data(ugd->service, GE_FILE_SELECT_RETURN_COUNT, "0");
	app_control_add_extra_data(ugd->service, GE_FILE_SELECT_RETURN_PATH, NULL);
	app_control_add_extra_data(ugd->service, APP_CONTROL_DATA_SELECTED, NULL);
	ug_send_result_full(ugd->ug, ugd->service, APP_CONTROL_RESULT_FAILED);
	//elm_naviframe_item_pop(it);
	if (!ugd->is_attach_panel) {
		ug_destroy_me(ugd->ug);
		ge_dbg("ug_destroy_me");
	}
	/*If return ture, ug will pop naviframe first.*/
	return EINA_FALSE;
}


int _ge_albums_create_view(ge_ugdata *ugd)
{
	/* Set view mode */
	_ge_set_view_mode(ugd, GE_VIEW_ALBUMS);
	ugd->tab_mode = GE_CTRL_TAB_ALBUMS;
	/* Get data */
	_ge_data_get_clusters(ugd, GE_ALBUM_DATA_LOCAL);
	ugd->cluster_list->append_cb = __ge_albums_append;

	ugd->album_gic = elm_gengrid_item_class_new();
	if (ugd->album_gic == NULL)
		goto GE_ALBUMS_FAILED;
	ugd->album_gic->item_style = "gallery_efl/albums_view";
	ugd->album_gic->func.text_get = __ge_albums_get_text;
	ugd->album_gic->func.content_get = __ge_albums_get_content;

	/* Register delete callback */
	evas_object_event_callback_add(ugd->albums_view_ly,
				       EVAS_CALLBACK_DEL,
				       __ge_albums_del_layout_cb, ugd);
	/* Create albums view */
	ugd->albums_view = __ge_albums_add_view(ugd, ugd->albums_view_ly);
	if (ugd->albums_view == NULL)
		goto GE_ALBUMS_FAILED;

	ge_dbgE("!!!!! album view !!!!!!!! is pushed in the naviframe instead of content set");
	ugd->nf_it = elm_naviframe_item_push(ugd->naviframe, GE_ALBUM_NAME, NULL, NULL, ugd->albums_view, NULL);

		if (ugd->nf_it != NULL) {
			ge_dbgE("!!!! album view !!!!! is push successfully in the nf");
		}
		else
			ge_dbgE("!!!!!! album view !!!!!1111 failed to push in nf ");


	elm_naviframe_item_pop_cb_set(ugd->nf_it, __ge_main_back_cb, ugd);

	ugd->rotate_cbs = eina_list_append(ugd->rotate_cbs,
					   __ge_albums_rotate_view);
	return 0;

 GE_ALBUMS_FAILED:

	GE_IF_DEL_OBJ(ugd->albums_view_ly);
	if (ugd->album_gic) {
		elm_gengrid_item_class_free(ugd->album_gic);
		ugd->album_gic = NULL;
	}
	return -1;
}

int _ge_albums_update_view(ge_ugdata *ugd)
{
	ge_dbg("");
	GE_CHECK_VAL(ugd, -1);
	GE_CHECK_VAL(ugd->cluster_list, -1);
	int sel_cnt = 0;

	/* Reset view mode */
	_ge_set_view_mode(ugd, GE_VIEW_ALBUMS);
	/* Changed to show no contents if needed */
	if (!ugd->cluster_list->clist ||
			(eina_list_count(ugd->cluster_list->clist) == 0)) {
		_ge_data_get_clusters(ugd, GE_ALBUM_DATA_LOCAL);
		ugd->cluster_list->append_cb = __ge_albums_append;
		if (!ugd->cluster_list->clist ||
					(eina_list_count(ugd->cluster_list->clist) == 0)) {
		ge_dbgW("Clusters list is empty!");
		goto ALBUMS_FAILED;
		}
	}

	if (ugd->nocontents && ugd->nocontents == ugd->albums_view) {
		/* It is nocontents, unset it first then create albums view*/
		evas_object_del(ugd->nocontents);
		ugd->nocontents = NULL;

		ugd->albums_view = __ge_albums_add_view(ugd, ugd->naviframe);
		GE_CHECK_VAL(ugd->albums_view, -1);
		Evas_Object *tmp = NULL;
		tmp=elm_object_part_content_get(ugd->albums_view_ly, "contents");
		if (tmp!=NULL) {
			ge_dbgE("tmp was not empty");
			elm_object_part_content_unset(ugd->albums_view_ly, "contents");
			evas_object_hide(tmp);
		}

		elm_object_part_content_set(ugd->albums_view_ly, "contents",
				ugd->albums_view);

		ge_dbgE("!!!!! album view !!!!!!!! is pushed in the naviframe instead of content set");
		ugd->nf_it = elm_naviframe_item_push(ugd->naviframe, GE_ALBUM_NAME, NULL, NULL, ugd->albums_view, NULL );

		if (ugd->nf_it != NULL) {
			ge_dbgE("!!!! album view !!!!! is push successfully in the nf");
		}
		else
			ge_dbgE("!!!!!! album view !!!!!1111 failed to push in nf ");



		evas_object_show(ugd->albums_view);
	} else {
		if (__ge_albums_append_albums(ugd, ugd->albums_view, true) != 0) {
			goto ALBUMS_FAILED;
		}
	}

	sel_cnt = _ge_data_get_sel_cnt(ugd);
	if (ugd->done_it != NULL) {
		if (sel_cnt > 0 && (ugd->max_count < 0 || sel_cnt <= ugd->max_count))
			_ge_ui_disable_item(ugd->done_it, false);
		else
			_ge_ui_disable_item(ugd->done_it, true);
	} else {
		ge_dbgW("done item is NULL");
	}

	return 0;

	ALBUMS_FAILED:

	sel_cnt = _ge_data_get_sel_cnt(ugd);
	if (ugd->done_it != NULL) {
		if (sel_cnt > 0 && (ugd->max_count < 0 || sel_cnt <= ugd->max_count))
			_ge_ui_disable_item(ugd->done_it, false);
		else
			_ge_ui_disable_item(ugd->done_it, true);
	} else {
		ge_dbgW("done item is NULL");
	}

	if (ugd->albums_view && ugd->albums_view != ugd->nocontents)
		__ge_albums_del_cbs(ugd->albums_view);

	evas_object_del(ugd->albums_view);

	ge_dbgW("@@@@@@@  To create nocontents view @@@@@@@@");
	ugd->nocontents = ge_ui_create_nocontents(ugd);
	ugd->albums_view = ugd->nocontents;
	GE_CHECK_VAL(ugd->albums_view, -1);
	evas_object_show(ugd->albums_view);

	elm_object_part_content_set(ugd->albums_view_ly, "contents",
			ugd->albums_view);


	ge_dbgE("!!!!! album view !!!!!!!! is pushed in the naviframe instead of content set");
	ugd->nf_it = elm_naviframe_item_push(ugd->naviframe, GE_ALBUM_NAME, NULL, NULL,ugd->albums_view, NULL );

	if (ugd->nf_it != NULL) {
		ge_dbgE("!!!! album view !!!!! is push successfully in the nf");
	}
	else
		ge_dbgE("!!!!!! album view !!!!!1111 failed to push in nf ");

	return -1;
}

