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

#include "ge-ext-ug-load.h"
#include "ge-debug.h"
#include "ge-ui-util.h"
#include "ge-util.h"
#include "ge-albums.h"
#include "ge-gridview.h"
#include "ge-data.h"

#define GE_IV_UG_NAME "org.tizen.image-viewer"
#define GE_IV_STR_LEN_MAX 32
#define GE_VIEW_MODE "View Mode"
#define GE_SETAS_TYPE "Setas type"
#define GE_VIEW_BY "View By"
#define GE_MEDIA_TYPE "Media type"
#define GE_PATH "Path"
#define GE_ARGV_IV_MEDIA_ALL "All"
#define GE_ARGV_IV_VIEW_BY_FOLER "By Folder"
#define GE_ARGV_IV_ALBUM_INDEX "Album index"
#define DEFAULT_THUMB "/opt/usr/share/media/.thumb/thumb_default.png"

typedef struct _ge_thumbs_t ge_thumbs_s;

static void __ge_appcontrol_select_result_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *user_data)
{
	GE_CHECK(user_data);
	ge_ugdata *ugd = (ge_ugdata *)user_data;
	int i;
	bool in_list = false;
	char **select_result = NULL;
	int count = 0;
	Eina_List *l = NULL;
	ge_item *data = NULL;
	int sel_count = 0;
	app_control_get_extra_data_array(reply, "Selected index", &select_result, &count);
	ge_dbg("current selection count = %d", count);

	if (select_result) {
		EINA_LIST_FOREACH(ugd->thumbs_d->medias_elist, l, data) {
			if (!data || !data->item) {
				continue;
			}
			if (!strcmp(data->item->thumb_url, DEFAULT_THUMB)) {
				continue;
			}
			in_list = false;
			for (i = 0; i < count; i++) {
				if (!strcmp(select_result[i], data->item->file_url)) {
					if (!data->checked) {
						__ge_grid_append_sel_item(ugd, data);
						data->checked = true;
					}
					in_list = true;
					sel_count++;
					break;
				}
			}
			if (!in_list) {
				if (data->checked) {
					__ge_grid_remove_sel_item(ugd, data);
					data->checked = false;
				}
			}
		}
	} else {
		EINA_LIST_FOREACH(ugd->thumbs_d->medias_elist, l, data) {
			if (!data || !data->item) {
				continue;
			}
			if (data->checked) {
				__ge_grid_remove_sel_item(ugd, data);
				data->checked = false;
			}
		}
	}

	data = eina_list_nth(ugd->thumbs_d->medias_elist, 0);
	if (data && data->album && data->album->griditem) {
		elm_gengrid_item_update(data->album->griditem);
	}
	int grid_unsupported_cnt = 0;
	grid_unsupported_cnt = ugd->thumbs_d->unsupported_cnt;
	if (sel_count != (eina_list_count(ugd->thumbs_d->medias_elist) - grid_unsupported_cnt)) {
		ugd->ck_state = EINA_FALSE;
	} else {
		ugd->ck_state = EINA_TRUE;
	}
	if ((ugd->thumbs_d->medias_cnt - grid_unsupported_cnt) <= 0) {
		ugd->ck_state = false;
	}

	Evas_Object *ck = elm_object_part_content_get(ugd->thumbs_d->layout, "select.all.area.check");
	elm_check_state_set(ck, ugd->ck_state);

	_ge_ui_update_label_text(ugd->thumbs_d->nf_it, count,
	                         ugd->albums_view_title);

	if (count > 0) {
		Evas_Object *btn = NULL;
		btn = elm_object_item_part_content_get(ugd->thumbs_d->nf_it , GE_NAVIFRAME_TITLE_RIGHT_BTN);
		if (btn == NULL) {
			ge_dbgE("Failed to get part information");
			return;
		}
		elm_object_disabled_set(btn, EINA_FALSE);
		_ge_add_remove_done_button_cb(btn, ugd, true);
	} else {
		Evas_Object *btn = NULL;
		btn = elm_object_item_part_content_get(ugd->thumbs_d->nf_it , GE_NAVIFRAME_TITLE_RIGHT_BTN);
		if (btn == NULL) {
			ge_dbgE("Failed to get part information");
			return;
		}
		elm_object_disabled_set(btn, EINA_TRUE);
		_ge_add_remove_done_button_cb(btn, ugd, false);
	}

	elm_gengrid_item_update(ugd->thumbs_d->album->griditem);

	if (select_result) {
		for (i = 0; i < count; i++) {
			if (select_result[i]) {
				free(select_result[i]);
			}
		}
		free(select_result);
	}
}

static void _ge_ext_destroy_me(ge_ugdata *ugd)
{
	ge_dbg("");
	GE_CHECK(ugd);

#ifdef _UG_UI_CONVERSION
	GE_CHECK(ugd->ug_called_by_me);
	GE_CHECK(ugd->ug);

	if (ugd->ug_called_by_me) {
		ge_dbg("Destroy ug_called_by_me");
		ug_destroy(ugd->ug_called_by_me);
		ugd->ug_called_by_me = NULL;
	} else {
		ge_dbg("ug_called_by_me does not exist!");
	}
#endif

	GE_CHECK(ugd->service);
	bool send_result = false;
	app_control_h reply = NULL;
	app_control_create(&reply);
	if (!reply) {
		ge_dbg("reply handle not created");
		return;
	}

	if (ugd->file_select_mode == GE_FILE_SELECT_T_SLIDESHOW) {
		ugd->b_destroy_me = false;
	}

	if (!ugd->b_destroy_me) {
		ge_dbg("gallery ug is still alive");
		return;
	}
	if (ugd->file_select_mode == GE_FILE_SELECT_T_SETAS) {
		if (ugd->file_setas_image_path) {
			ge_dbg("GE_SETAS_IMAGE_PATH:%s", ugd->file_setas_image_path);
			app_control_add_extra_data(reply,
			                           GE_SETAS_IMAGE_PATH,
			                           ugd->file_setas_image_path);

			GE_FREE(ugd->file_setas_image_path);
			send_result = true;
		}

		if (ugd->file_setas_crop_image_path &&
		        (ugd->file_select_setas_mode == GE_SETAS_T_CALLERID ||
		         ugd->file_select_setas_mode == GE_SETAS_T_CROP_WALLPAPER)) {
			ge_dbg("GE_SETAS_CALLERID_CROP_IMAGE_PATH:%s",
			       ugd->file_setas_crop_image_path);
			app_control_add_extra_data(reply,
			                           APP_CONTROL_DATA_SELECTED,
			                           ugd->file_setas_crop_image_path);

			GE_FREE(ugd->file_setas_crop_image_path);
			send_result = true;
		}

		if (send_result) {
			ge_dbg("Call ug_send_result_full() to send result.");
			bool reply_requested = false;

#ifdef _UG_UI_CONVERSION
			ug_send_result_full(ugd->ug, ugd->service, APP_CONTROL_RESULT_SUCCEEDED);
#endif
			app_control_is_reply_requested(ugd->service, &reply_requested);
			if (reply_requested) {
				ge_sdbg("send reply to caller");
				app_control_reply_to_launch_request(reply, ugd->service, APP_CONTROL_RESULT_SUCCEEDED);
				app_control_destroy(reply);
			}
		}
	}

	if (ugd->b_destroy_me) {
		ge_dbg("Setting is appllied, destroy gallery UG.");
		ugd->b_destroy_me = false;
		app_control_destroy(ugd->service);
		ui_app_exit();
		/* Destroy self */
#ifdef _UG_UI_CONVERSION
		if (!ugd->is_attach_panel) {
			ug_destroy_me(ugd->ug);
		}
#endif
	} else {
		ge_dbg("Cancel button tapped, back to thumbnails view.");
	}
}

#if _UG_UI_CONVERSION
static void _ge_ext_iv_layout_cb(ui_gadget_h ug, enum ug_mode mode, void* priv)
{
	ge_dbg("");
	GE_CHECK(priv);
	GE_CHECK(ug);

	Evas_Object *base = (Evas_Object *)ug_get_layout(ug);
	if (!base) {
		ge_dbgE("ug_get_layout failed!");
		app_control_destroy(); //to do passing parameter
//		ug_destroy(ug);
		return;
	}

	evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	/* Disable effect to avoid BS caused by ui-gadget to
	     unset ug layout after deleting it */
	ug_disable_effect(ug);
	evas_object_show(base);
}

static void _ge_ext_iv_result_cb(ui_gadget_h ug, app_control_h result, void *priv)
{
	ge_dbg("");
	GE_CHECK(priv);
	GE_CHECK(result);
	ge_ugdata *ugd = (ge_ugdata *)priv;
	char *path = NULL;
	char *status = NULL;

	if (ugd->file_select_mode == GE_FILE_SELECT_T_SETAS) {
		/*If set wallpaper success, homescreen_path should not be null.
		And if setting wallpaper was canceled in IV, gallery-efl doesn't exit immediately*/
		app_control_get_extra_data(result, GE_BUNDLE_HOMESCREEN_PATH,
		                           &path);
		if (NULL == path)
			app_control_get_extra_data(result, GE_BUNDLE_LOCKSCREEN_PATH,
			                           &path);
		ge_dbg("SETAS_IMAGE_PATH");
		app_control_get_extra_data(result, "Result", &status);
		ugd->file_select_setas_mode = 0;
		if (strcmp(status, "Cancel")) {
			ugd->file_select_setas_mode = 1;
		}

		if (path) {
			ge_dbg(":%s", path);
			ugd->b_destroy_me = true;
			ugd->file_setas_image_path = path;
		} else {
			ugd->b_destroy_me = false;
		}
		/*If has got homescreen_path, setats_mode should not be callerid and
		crop wallpaper*/
		if (path == NULL &&
		        (ugd->file_select_setas_mode == GE_SETAS_T_CALLERID ||
		         ugd->file_select_setas_mode == GE_SETAS_T_CROP_WALLPAPER)) {
			app_control_get_extra_data(result, APP_CONTROL_DATA_SELECTED,
			                           &path);
			ge_dbg("CALLERID_CROP_IMAGE_PATH");
			if (path) {
				ge_dbg(":%s", path);
				ugd->b_destroy_me = true;
				ugd->file_setas_crop_image_path = path;
			} else {
				ugd->b_destroy_me = false;
			}
		}
	}

	char *error_state = NULL;
	app_control_get_extra_data(result, GE_IMAGEVIEWER_RETURN_ERROR,
	                           &error_state);
	if (error_state) {
		ge_dbg("error string : %s", error_state);

		if (!g_strcmp0(error_state, "not_supported_file_type")) {
			ugd->b_destroy_me = false;
			if (ugd->ug_path == NULL) {
				ge_dbgE("current item is NULL");
				GE_FREE(error_state);
				return;
			}
			app_control_h service;
			app_control_create(&service);
			GE_CHECK(service);
			app_control_set_operation(service, APP_CONTROL_OPERATION_VIEW);
			app_control_set_uri(service, ugd->ug_path);
			app_control_send_launch_request(service, NULL, NULL);
			app_control_destroy(service);
		}
		GE_FREE(error_state);
	}
}

static void _ge_ext_iv_destroy_cb(ui_gadget_h ug, void *priv)
{
	ge_dbg("");
	GE_CHECK(priv);
	_ge_ext_destroy_me((ge_ugdata *)priv);
}

static void __ge_ext_iv_end_cb(ui_gadget_h ug, void *priv)
{
	ge_dbg("");
	GE_CHECK(priv);
	ge_ugdata *ugd = (ge_ugdata *)priv;

	if (ugd->file_select_setas_mode == 1) {
		_ge_grid_sel_one(ugd, ugd->file_select_setas_path);
	}

	if (ugd->b_hide_indicator) {
		_ge_ui_hide_indicator((ge_ugdata *)priv);
	}
}

#endif

static char **__ge_ext_get_select_index(ge_ugdata *ugd, int *size)
{
	GE_CHECK_NULL(ugd);
	GE_CHECK_NULL(ugd->selected_elist);
	char *index = NULL;
	int i = 0;
	char **media_index = NULL;
	int pos = 0;
	int sel_cnt = 0;
	ge_item *git = NULL;
	Eina_List *l = NULL;
	ge_dbg("Media count: %d", eina_list_count(ugd->selected_elist));

	sel_cnt = eina_list_count(ugd->selected_elist);
	ge_dbg("Item count: %d", sel_cnt);
	media_index = (char **)calloc(sel_cnt, sizeof(char *));
	GE_CHECK_NULL(media_index);

	EINA_LIST_FOREACH(ugd->selected_elist, l, git) {
		index = (char *)calloc(1, GE_IV_STR_LEN_MAX);
		if (git == NULL || index == NULL) {
			for (pos = 0; pos < i; ++pos) {
				GE_FREEIF(media_index[pos]);
			}

			GE_FREEIF(index);
			GE_FREE(media_index);
			return NULL;
		}
		ge_dbg("Sequence: %d", git->sequence - 1);
		snprintf(index, GE_IV_STR_LEN_MAX, "%d", git->sequence - 1);
		media_index[i++] = index;
		index = NULL;
		git = NULL;
	}


	if (size) {
		*size = sel_cnt;
	}

	return media_index;
}

/* Slideshow selected images */
static int __ge_ext_slideshow_selected(ge_ugdata *ugd, app_control_h service)
{
	GE_CHECK_VAL(service, -1);
	GE_CHECK_VAL(ugd, -1);
	char **media_index = NULL;
	int media_size = 0;
#define GE_SELECTED_FILES "Selected index"
#define GE_INDEX "Index"
#define GE_INDEX_VALUE "1"

	media_index = __ge_ext_get_select_index(ugd, &media_size);
	if (media_index == NULL) {
		ge_dbgE("Invalid select index!");
		return -1;
	}
	ge_dbg("Set selected medias, media_index[%p], size[%d]", media_index,
	       media_size);
	app_control_add_extra_data_array(service, GE_SELECTED_FILES,
	                                 (const char **)media_index, media_size);
	/*free space of the medias index*/
	int i = 0;
	for (i = 0; i < media_size; ++i) {
		ge_dbg("Set selected medias, media_index[%s]", media_index[i]);
		GE_FREEIF(media_index[i]);
	}
	GE_FREE(media_index);
	media_index = NULL;

	app_control_add_extra_data(service, GE_INDEX, GE_INDEX_VALUE);
	return 0;
}

static int __ge_ext_set_slideshow_data(ge_ugdata *ugd, char *file_url,
                                       app_control_h service)
{
	GE_CHECK_VAL(service, -1);
	GE_CHECK_VAL(file_url, -1);
	GE_CHECK_VAL(ugd, -1);

	app_control_add_extra_data(service, GE_PATH, file_url);
	app_control_add_extra_data(service, GE_VIEW_MODE, "SLIDESHOW");
	app_control_add_extra_data(service, "Sort By", "DateDesc");
	if (ugd->file_type_mode == GE_FILE_T_IMAGE) {
		app_control_add_extra_data(service, GE_MEDIA_TYPE, "Image");
	} else if (ugd->file_type_mode == GE_FILE_T_VIDEO) {
		app_control_add_extra_data(service, GE_MEDIA_TYPE, "Video");
	} else {
		app_control_add_extra_data(service, GE_MEDIA_TYPE, "All");
	}
	if (__ge_ext_slideshow_selected(ugd, service) != 0) {
		ge_dbgE("Create UG failed!");
		return -1;
	}

	switch (ugd->slideshow_viewby) {
	case GE_VIEW_BY_ALL:
	case GE_VIEW_BY_ALBUMS:
		if (ugd->slideshow_album_id == NULL) {
			ge_dbgE("Create UG failed!");
			return -1;
		}
		app_control_add_extra_data(service, "Album index", ugd->slideshow_album_id);
		if (!g_strcmp0(ugd->slideshow_album_id, GE_ALBUM_ALL_ID)) {
			app_control_add_extra_data(service, GE_VIEW_BY, "All");
		} else {
			app_control_add_extra_data(service, GE_VIEW_BY, "By Folder");
		}
		break;
	default:
		return -1;
	}
	return 0;
}

static int __ge_ext_set_setas_data(ge_ugdata *ugd, char *file_url,
                                   app_control_h service)
{
	GE_CHECK_VAL(service, -1);
	GE_CHECK_VAL(file_url, -1);
	GE_CHECK_VAL(ugd, -1);

	if (file_url) {
		app_control_add_extra_data(service, GE_PATH, file_url);
		GE_FREEIF(ugd->ug_path);
		ugd->ug_path = strdup(file_url);
	}
	app_control_add_extra_data(service, GE_VIEW_MODE, "SETAS");

	if (ugd->file_select_setas_mode == GE_SETAS_T_WALLPAPER) {
		app_control_add_extra_data(service, GE_SETAS_TYPE, "Wallpaper");
	} else if (ugd->file_select_setas_mode == GE_SETAS_T_LOCKPAPER) {
		app_control_add_extra_data(service, GE_SETAS_TYPE, "Lockscreen");
	} else if (ugd->file_select_setas_mode == GE_SETAS_T_WALLPAPER_LOCKPAPER) {
		app_control_add_extra_data(service, GE_SETAS_TYPE, "Wallpaper & Lockscreen");
	} else if (ugd->file_select_setas_mode == GE_SETAS_T_CROP_WALLPAPER) {
		app_control_add_extra_data(service, GE_SETAS_TYPE, "Wallpaper Crop");
		app_control_add_extra_data(service, "Fixed ratio", "TRUE");

		int x = 0;
		int y = 0;
		int w = 0;
		int h = 0;

#ifdef _UG_UI_CONVERSION
		elm_win_screen_size_get((Evas_Object *)ug_get_window(), &x, &y, &w, &h);
#endif
		elm_win_screen_size_get(ugd->win, &x, &y, &w, &h);
		char *reso_str = (char *)calloc(1, GE_IV_STR_LEN_MAX);
		if (reso_str == NULL) {
			ge_dbgE("Calloc failed!");
			return -1;
		}
		snprintf(reso_str, GE_IV_STR_LEN_MAX, "%dx%d", w, h);
		ge_dbgW("Window Resolution: %dx%d, %s", w, h, reso_str);
		app_control_add_extra_data(service, "Resolution", reso_str);
		GE_FREE(reso_str);
	} else if (ugd->file_select_setas_mode == GE_SETAS_T_CALLERID) {
		app_control_add_extra_data(service, GE_SETAS_TYPE, "CallerID");
	}
	return 0;
}

static int __ge_ext_albums_set_iv_select_mode(void *data, app_control_h service, int type)
{
	GE_CHECK_VAL(service, GE_UG_FAIL);
	GE_CHECK_VAL(data, GE_UG_FAIL);
	ge_ugdata *ugd = (ge_ugdata *)data;
	ge_cluster *cur_album = ugd->album_item;
	GE_CHECK_VAL(ugd->album_item, GE_UG_FAIL);
	GE_CHECK_VAL(ugd->thumbs_d, GE_UG_FAIL);
	ge_dbg("type: %d", type);

	/* 2.0, Local file */
	app_control_add_extra_data(service, GE_VIEW_MODE, "SELECT");
	app_control_add_extra_data(service, GE_MEDIA_TYPE, GE_ARGV_IV_MEDIA_ALL);
	app_control_set_mime(service, "image/*");

	if ((cur_album && cur_album->cluster && cur_album->cluster->uuid) || ugd->thumbs_d->selected_uuid) {
		app_control_add_extra_data(service, GE_VIEW_BY, GE_ARGV_IV_VIEW_BY_FOLER);
		app_control_add_extra_data(service, GE_ARGV_IV_ALBUM_INDEX,
		                           ugd->thumbs_d->selected_uuid);
	}
	return GE_UG_SUCCESS;
}

static int _ge_ext_load_iv_selected_list(app_control_h service, void *data, ge_media_s *item)
{
	GE_CHECK_VAL(data, GE_UG_FAIL);
	GE_CHECK_VAL(service, GE_UG_FAIL);
	GE_CHECK_VAL(item, GE_UG_FAIL);
	ge_ugdata *ugd = (ge_ugdata *)data;
	int i;
	int count = _ge_data_get_sel_cnt(ugd);

	char **value = NULL;
	if (count > 0) {
		(value) = (char**)malloc(count * sizeof(char *));
		if (!value) {
			return GE_UG_FAIL;
		}
	}
	Eina_List *sel_ids = NULL;
	_ge_data_get_sel_item(ugd, &sel_ids);
	if (!sel_ids) {
		ge_dbgW("Invalid selection!!");

		if (value) {
			free(value);
		}
		return GE_UG_FAIL;
	}

	ge_sel_item_s *sit = NULL;

	for (i = 0; i < count; i++) {
		sit = eina_list_nth(sel_ids, i);
		if (sit) {
			if (sit->file_url) {
				(value)[i] = strdup(sit->file_url);
			}
		}
	}

	if (count > 0) {
		app_control_add_extra_data_array(service, "Selected index",
		                                 (const char **)value, count);
	}

	if (value) {
		for (i = 0; i < count; i++) {
			free(value[i]);
		}
		free(value);
	}

	return GE_UG_SUCCESS;
}

int _ge_ext_load_iv_ug_select_mode(void *data, ge_media_s *item, ge_ext_iv_type type)
{
	GE_CHECK_VAL(item, GE_UG_FAIL);
	GE_CHECK_VAL(data, GE_UG_FAIL);
	ge_ugdata *ugd = (ge_ugdata *)data;
	app_control_h service = NULL;

	if (ugd->uginfo.ug) {
		ge_dbgE("Already exits some UG called by me, type(%d)!", type);
		return GE_UG_FAIL;
	}

	app_control_create(&service);
	GE_CHECK_VAL(service, GE_UG_FAIL);

	ugd->uginfo.ug_type = GE_UG_IMAGEVIEWER;
	ugd->uginfo.iv_type = type;

	int ret = GE_UG_FAIL;
	ret = __ge_ext_albums_set_iv_select_mode(data, service, type);

	if (ret == GE_UG_FAIL) {
		ge_dbgW("Service data setting failed!");
		app_control_destroy(service);
		return GE_UG_FAIL;
	}

	if (!item->file_url) {
		ge_dbgW("Invalid file selected!!");
		app_control_destroy(service);
		return GE_UG_FAIL;
	}
	app_control_add_extra_data(service, GE_PATH, item->file_url);
	_ge_ext_load_iv_selected_list(service, ugd, item);
	ret = app_control_set_launch_mode(service, APP_CONTROL_LAUNCH_MODE_GROUP);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ge_dbgE("set launch mode failed");
	}
	ret = app_control_set_app_id(service, GE_IV_UG_NAME);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ge_dbgE("set appid failed");
	}

	if (ugd->file_type_mode == GE_FILE_T_IMAGE) {
		ret = app_control_add_extra_data(service, GE_MEDIA_TYPE, "Image");
	} else if (ugd->file_type_mode == GE_FILE_T_VIDEO) {
		ret = app_control_add_extra_data(service, GE_MEDIA_TYPE, "Video");
	} else {
		ret = app_control_add_extra_data(service, GE_MEDIA_TYPE, "All");
	}

	if (ret != APP_CONTROL_ERROR_NONE) {
		ge_dbgE("Setting the Media type failed");
	}
	ret = app_control_send_launch_request(service, __ge_appcontrol_select_result_cb, (void *)ugd);

	app_control_destroy(service);

	if (ret != APP_CONTROL_ERROR_NONE) {
		ge_dbgE("ug_create failed!");
		return GE_UG_FAIL;
	} else {
		elm_object_tree_focus_allow_set(ugd->ly_main, EINA_FALSE);
		ugd->uginfo.b_ug_launched = true;
		return GE_UG_SUCCESS;
	}
}

static void
__ge_gallery_ug_result_cb(app_control_h request, app_control_h result, app_control_result_e reply, void *data)
{

	ge_dbg("");
	GE_CHECK(data);
	GE_CHECK(result);
	ge_ugdata *ugd = (ge_ugdata *)data;
	char *path = NULL;
	char *status = NULL;

	if (ugd->file_select_mode == GE_FILE_SELECT_T_SETAS) {
		/*If set wallpaper success, homescreen_path should not be null.
		And if setting wallpaper was canceled in IV, gallery-efl doesn't exit immediately*/
		app_control_get_extra_data(result, GE_BUNDLE_HOMESCREEN_PATH,
		                           &path);
		if (NULL == path)
			app_control_get_extra_data(result, GE_BUNDLE_LOCKSCREEN_PATH,
			                           &path);
		ge_dbg("SETAS_IMAGE_PATH");
		app_control_get_extra_data(result, "Result", &status);
		ugd->file_select_setas_mode = 0;
		if (strcmp(status, "Cancel")) {
			ugd->file_select_setas_mode = 1;
		}

		if (path) {
			ge_dbg(":%s", path);
			ugd->b_destroy_me = true;
			ugd->file_setas_image_path = path;
		} else {
			ugd->b_destroy_me = false;
		}
		/*If has got homescreen_path, setats_mode should not be callerid and
		crop wallpaper*/
		if (path == NULL &&
		        (ugd->file_select_setas_mode == GE_SETAS_T_CALLERID ||
		         ugd->file_select_setas_mode == GE_SETAS_T_CROP_WALLPAPER)) {
			app_control_get_extra_data(result, APP_CONTROL_DATA_SELECTED,
			                           &path);
			ge_dbg("CALLERID_CROP_IMAGE_PATH");
			if (path) {
				ge_dbg(":%s", path);
				ugd->b_destroy_me = true;
				ugd->file_setas_crop_image_path = path;
			} else {
				ugd->b_destroy_me = false;
			}
		}
	}

	char *error_state = NULL;
	app_control_get_extra_data(result, GE_IMAGEVIEWER_RETURN_ERROR,
	                           &error_state);
	if (error_state) {
		ge_dbg("error string : %s", error_state);

		if (!g_strcmp0(error_state, "not_supported_file_type")) {
			ugd->b_destroy_me = false;
			if (ugd->ug_path == NULL) {
				ge_dbgE("current item is NULL");
				GE_FREE(error_state);
				return;
			}
			app_control_h service;
			app_control_create(&service);
			GE_CHECK(service);
			app_control_set_operation(service, APP_CONTROL_OPERATION_VIEW);
			app_control_set_uri(service, ugd->ug_path);
			app_control_send_launch_request(service, NULL, NULL);
			app_control_destroy(service);
		}
		GE_FREE(error_state);
	}
	_ge_ext_destroy_me((ge_ugdata *)data);
}

int _ge_ext_load_iv_ug(ge_ugdata *ugd, char *file_url, char *album_index, int image_index)
{
	GE_CHECK_VAL(file_url, -1);
	GE_CHECK_VAL(ugd, -1);
#ifdef _UG_UI_CONVERSION
	struct ug_cbs cbs;
	ui_gadget_h ug = NULL;
	if (ugd->ug_called_by_me) {
		ge_dbgW("Already exits some UG called by me!");
		return -1;
	}

	memset(&cbs, 0x00, sizeof(struct ug_cbs));
	cbs.layout_cb = _ge_ext_iv_layout_cb;
	cbs.result_cb = _ge_ext_iv_result_cb;
	cbs.destroy_cb = _ge_ext_iv_destroy_cb;
	cbs.end_cb = __ge_ext_iv_end_cb;
	cbs.priv = ugd;
#endif
	app_control_h service = NULL;

	app_control_create(&service);
	GE_CHECK_VAL(service, -1);

	if (ugd->file_select_mode == GE_FILE_SELECT_T_SLIDESHOW) {
		if (__ge_ext_set_slideshow_data(ugd, file_url, service) < 0) {
			ge_dbgE("Set slideshow data failed!");
			app_control_destroy(service);
			return -1;
		}
	} else {
		if (__ge_ext_set_setas_data(ugd, file_url, service) < 0) {
			ge_dbgE("Set setas data failed!");
			app_control_destroy(service);
			return -1;
		}
	}

	evas_object_smart_callback_call(ugd->naviframe, "gallery,freeze,resize", (void *)1);
	app_control_add_extra_data(service, "View By", "All");
	app_control_add_extra_data(service, "Album index", album_index);
	app_control_add_extra_data(service, "Path", file_url);
	app_control_add_extra_data(service, "Sort By", "Name");
	char image_index_str[12];
	eina_convert_itoa(image_index, image_index_str);
	app_control_add_extra_data(service, "Index", image_index_str);

	app_control_set_app_id(service, GE_IV_UG_NAME);

#ifdef _UG_UI_CONVERSION
	ug = ug_create(ugd->ug, GE_IV_UG_NAME, UG_MODE_FULLVIEW, service, &cbs);
	ugd->ug_called_by_me = ug;

	if (!ug) {
		ge_dbgE("Create UG failed!");
		return -1;
	} else {
		ge_dbg("Create UG successully");
		return 0;
	}
#endif

	int ret = app_control_send_launch_request(service, __ge_gallery_ug_result_cb, ugd);
	app_control_destroy(service);
	if (ret == 0) {
		ge_dbg("Launched ug-image-viewer-efl successfully.");
		return 0;
	} else {
		ge_dbgE("Launching ug-image-viewer-efl Failed.");
		return -1;
	}
}

int _ge_ext_load_iv_ug_for_help(ge_ugdata *ugd, const char *uri)
{
	GE_CHECK_VAL(ugd, -1);

#ifdef _UG_UI_CONVERSION
	struct ug_cbs cbs;
	ui_gadget_h ug = NULL;

	if (ugd->ug_called_by_me) {
		ge_dbgW("Already exits some UG called by me!");
		return -1;
	}

	memset(&cbs, 0x00, sizeof(struct ug_cbs));
	cbs.layout_cb = _ge_ext_iv_layout_cb;
	cbs.result_cb = _ge_ext_iv_result_cb;
	cbs.destroy_cb = _ge_ext_iv_destroy_cb;
	cbs.end_cb = __ge_ext_iv_end_cb;
	cbs.priv = ugd;
#endif

	app_control_h service = NULL;
	app_control_create(&service);
	GE_CHECK_VAL(service, -1);

	/* Set "HELP" to "View Mode" */
	app_control_add_extra_data(service, GE_VIEW_MODE, "HELP");

	/* Set help uri to file path */
	app_control_add_extra_data(service, GE_PATH, uri);


	app_control_set_app_id(service,  GE_IV_UG_NAME);
	int ret = app_control_send_launch_request(service, __ge_gallery_ug_result_cb, ugd);
	app_control_destroy(service);
	if (ret == 0) {
		ge_dbg("Launched ug-image-viewer-efl successfully.");
		return 0;
	} else {
		ge_dbgE("Launching ug-image-viewer-efl Failed.");
		return -1;
	}

#ifdef _UG_UI_CONVERSION
	ug = ug_create(ugd->ug, GE_IV_UG_NAME, UG_MODE_FULLVIEW, service, &cbs);
	ugd->ug_called_by_me = ug;
	if (ug != NULL) {
		ge_dbg("Create UG successully");
		return 0;
	} else {
		ge_dbgE("Create UG failed!");
		return -1;
	}
#endif
}
