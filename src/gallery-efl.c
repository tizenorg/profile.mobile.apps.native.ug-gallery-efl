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

#ifndef UG_MODULE_API
#define UG_MODULE_API __attribute__ ((visibility("default")))
#endif

#include <Evas.h>
#include <app.h>
#include "gallery-efl.h"
#include "ge-albums.h"
#include "ge-gridview.h"
#include "ge-data.h"
#include "ge-ui-util.h"
#include "ge-util.h"
#include "ge-db-update.h"
#include "ge-ext-ug-load.h"
#include "ge-lang.h"
#include "ge-main-view.h"

#define GE_BG_COLOR_DEFAULT 0
#define GE_BG_COLOR_A 255

static int __ge_create_main_view(ge_ugdata *ugd)
{
	GE_CHECK_VAL(ugd, -1);
	GE_CHECK_VAL(ugd->naviframe, -1);

	if (ugd->file_select_mode == GE_FILE_SELECT_T_SLIDESHOW) {
		ge_dbgE("file select mode is slideshow>>>>>>> grid view");
		_ge_grid_create_main_view(ugd);
	} else {

		ge_dbgE("no slide show mode>>>>>>>>>");
		_ge_main_create_view(ugd);
	}
	return 0;
}

static Evas_Object *_ge_create_bg(Evas_Object *parent)
{
	ge_dbg("");
	GE_CHECK_NULL(parent);
	Evas_Object *bg = NULL;

	/* Show special color of background */
	bg = evas_object_rectangle_add(evas_object_evas_get(parent));
	evas_object_color_set(bg, GE_BG_COLOR_DEFAULT, GE_BG_COLOR_DEFAULT,
	                      GE_BG_COLOR_DEFAULT, GE_BG_COLOR_A);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND,
	                                 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(bg);

	return bg;
}

/**
 * Add windown rotation for PLM bug: P130605-7106.
 * Before we got rotation event from caller,
 * but it took much time, so view updating/rotation is visible for user
 * according to bug description.
 *
 * After we add window rotation callback to get rotation event as fast as possible,
 * so view updating/rotation is invisible for user.
*/
static void __ge_win_rot_changed_cb(void *data, Evas_Object *obj,
					 void *event)
{
	GE_CHECK(data);
	GE_CHECK(obj);
	ge_ugdata *ugd = (ge_ugdata *)data;

	/* changed_ang value is 0 90 180 270 */
#if _UG_UI_CONVERSION
	enum ug_event evt = UG_EVENT_NONE;
	switch (changed_ang) {
	case APP_DEVICE_ORIENTATION_0:
		evt = GE_ROTATE_PORTRAIT;
		break;
	case APP_DEVICE_ORIENTATION_90:
		evt = GE_ROTATE_LANDSCAPE_UPSIDEDOWN;
		break;
	case APP_DEVICE_ORIENTATION_180:
		evt = GE_ROTATE_PORTRAIT_UPSIDEDOWN;
		break;
	case APP_DEVICE_ORIENTATION_270:
		evt = GE_ROTATE_LANDSCAPE;
		break;
	}
#endif
	int changed_ang = elm_win_rotation_get(obj);
	ge_dbg("New angle: %d, old angle: %d", changed_ang, ugd->rotate_mode);
	if (changed_ang == ugd->rotate_mode) {
		return;
	} else {
		ugd->rotate_mode = changed_ang;
	}

	/* Update rotate mode and view */
	int len = eina_list_count(ugd->rotate_cbs);
	ge_rotate_view_cb rotate_cb = eina_list_nth(ugd->rotate_cbs, len - 1);
	if (rotate_cb) {
		ge_dbg("To rotate view");
		rotate_cb(ugd);
	} else {
		ge_dbgE("None callbacks exists[%d]!", len);
	}
}

static int _ge_init_view(ge_ugdata *ugd)
{
	ge_dbg("");
	GE_CHECK_VAL(ugd, -1);

	/* Base Layout */
	ugd->ly_main = ge_ui_create_main_ly(ugd->win);
	GE_CHECK_VAL(ugd->ly_main, -1);

	/* Background */
//	ugd->bg = _ge_create_bg(ugd->ly_main);
//	GE_CHECK_VAL(ugd->bg, -1);

	/* Navigation Bar */
	ugd->naviframe = ge_ui_create_naviframe(ugd, ugd->ly_main);
	GE_CHECK_VAL(ugd->naviframe, -1);

	ge_dbgE("base view is pushed in the naviframe instead of content set");

	/* Albums view and its layout */
	if (__ge_create_main_view(ugd) != 0) {
		ge_dbgE("Create albums view and layout failed!");
		return -1;
	}

	return 0;
}

static int _ge_create_view(ge_ugdata *ugd)
{
	ge_dbg("");
	GE_CHECK_VAL(ugd, -1);
	/* Add new theme */
	ugd->th = elm_theme_new();
	GE_CHECK_VAL(ugd->th, -1);
	elm_theme_ref_set(ugd->th, NULL);
	elm_theme_extension_add(ugd->th, GE_EDJ_FILE);

	if (_ge_init_view(ugd) != 0) {
		ge_dbgE("Initialize view failed!");
		return -1;
	} else {
		ge_dbgE("view is intialized !!!!!!!!!!!!!");
	}

	/* Register MMC changed callback */
	if (ge_reg_db_update_noti(ugd) != 0) {
		ge_dbgE("reg_db_update_noti failed!");
		return -1;
	} else {
		ge_dbgE("reg_db is updated storage state change");
	}

	return 0;
}

static int _ge_close_view(ge_ugdata *ugd)
{
	GE_CHECK_VAL(ugd, -1);

	/************* Remove Callbacks **************/
	GE_IF_DEL_TIMER(ugd->del_timer)
	/* Remove win rotation callback */
	evas_object_smart_callback_del(ugd->win, "wm,rotation,changed",
	                               __ge_win_rot_changed_cb);
	ge_dereg_db_update_noti();

	/************* Destroy UI objects **************/
	/* Destroy UG called by me */

#ifdef _UG_UI_CONVERSION
	if (ugd->ug_called_by_me) {
		ge_dbg("Destroy ug_called_by_me");
		ug_destroy(ugd->ug_called_by_me);
		ugd->ug_called_by_me = NULL;
	}
#endif
	GE_IF_DEL_OBJ(ugd->popup)
	GE_IF_DEL_OBJ(ugd->ly_main)
	if (ugd->th) {
		elm_theme_extension_del(ugd->th, GE_EDJ_FILE);
		elm_theme_free(ugd->th);
		ugd->th = NULL;
	}
#ifdef _USE_HIDE_INDICATOR
	_ge_ui_reset_indicator(ugd);
#endif

	/************* Free data **************/
	if (ugd->service) {
		app_control_destroy(ugd->service);
		ugd->service = NULL;
	}
	GE_FREEIF(ugd->file_setas_image_path);
	GE_FREEIF(ugd->file_setas_crop_image_path);
	GE_FREEIF(ugd->slideshow_album_id);
	GE_FREEIF(ugd->ug_path);
	/*Finalize db update callback*/
	_ge_db_update_finalize(ugd);
	/* Disconnet with libmedia-info */
	_ge_data_finalize(ugd);
	ui_app_exit();
	return 0;
}

static int __ge_parse_param_setas(ge_ugdata *ugd, app_control_h service)
{
	GE_CHECK_VAL(ugd, -1);
	GE_CHECK_VAL(service, -1);
	char *setas_type = NULL;

	app_control_get_extra_data(service, GE_BUNDLE_SETAS_TYPE, &setas_type);
	if (setas_type == NULL) {
		ge_dbgW("setas-type NULL");
		return -1;
	}

	if (!strcasecmp(setas_type, GE_SETAS_WALLPAPER)) {
		ugd->file_select_setas_mode = GE_SETAS_T_WALLPAPER;
	} else if (!strcasecmp(setas_type, GE_SETAS_LOCKSCREEN)) {
		ugd->file_select_setas_mode = GE_SETAS_T_LOCKPAPER;
	} else if (!strcasecmp(setas_type, GE_SETAS_WALLPAPER_AND_LOCKSCREEN)) {
		ugd->file_select_setas_mode = GE_SETAS_T_WALLPAPER_LOCKPAPER;
	} else if (!strcasecmp(setas_type, GE_SETAS_CROP_WALLPAPER)) {
		ugd->file_select_setas_mode = GE_SETAS_T_CROP_WALLPAPER;
	} else if (!strcasecmp(setas_type, GE_SETAS_CALLERID)) {
		ugd->file_select_setas_mode = GE_SETAS_T_CALLERID;
	} else if (!strcasecmp(setas_type, "crop")) {
		ugd->file_select_setas_mode = GE_SETAS_T_CROP_WALLPAPER;
	}
	GE_FREE(setas_type);
	return 0;
}

static int __ge_parse_param_file(ge_ugdata *ugd, app_control_h service)
{
	GE_CHECK_VAL(ugd, -1);
	GE_CHECK_VAL(service, -1);
	char *file_type = NULL;
	char *mime = NULL;

	app_control_get_mime(service, &mime);
	if (mime) {
		if (!g_strcmp0(mime, GE_MIME_IMAGE_ALL)) {
			ugd->file_type_mode = GE_FILE_T_IMAGE;
		} else if (!g_strcmp0(mime, GE_MIME_VIDEO_ALL)) {
			ugd->file_type_mode = GE_FILE_T_VIDEO;
		} else {
			ugd->file_type_mode = GE_FILE_T_ALL;
		}

		return 0;
	}

	/* file-type */
	app_control_get_extra_data(service, GE_BUNDLE_FILE_TYPE, &file_type);
	if (file_type == NULL) {
		ge_dbgW("file_type is empty!");
		goto GE_DEFAULT_FILE_TYPE;
	}
	ge_dbg("file_type: %s", file_type);

	if (!strcasecmp(file_type, GE_FILE_TYPE_IMAGE)) {
		ugd->file_type_mode = GE_FILE_T_IMAGE;
		GE_FREE(file_type);
		return 0;
	} else if (!strcasecmp(file_type, GE_FILE_TYPE_VIDEO)) {
		ugd->file_type_mode = GE_FILE_T_VIDEO;
		GE_FREE(file_type);
		return 0;
	}


GE_DEFAULT_FILE_TYPE:

	ge_dbg("file type is default, set default type(ALL).");
	ugd->file_type_mode = GE_FILE_T_ALL;
	return 0;
}

static int __ge_parse_param_slideshow(ge_ugdata *ugd, app_control_h service)
{
	GE_CHECK_VAL(ugd, -1);
	GE_CHECK_VAL(service, -1);
	ugd->file_select_mode = GE_FILE_SELECT_T_SLIDESHOW;
	ugd->b_multifile = true;
	char *viewby = NULL;
	char *tmp = NULL;

	app_control_get_extra_data(service, "view-by", &viewby);
	if (viewby) {
		if (!strcasecmp(viewby, "all")) {
			ugd->slideshow_viewby = GE_VIEW_BY_ALL;
			ugd->slideshow_album_id = g_strdup(GE_ALBUM_ALL_ID);
			ge_sdbg("album id: %s", ugd->slideshow_album_id);
		} else if (!strcasecmp(viewby, "albums")) {
			ugd->slideshow_viewby = GE_VIEW_BY_ALBUMS;
			app_control_get_extra_data(service, "album-id", &tmp);
			if (tmp) {
				ugd->slideshow_album_id = g_strdup(tmp);
				GE_FREE(tmp);
			} else {
				ugd->slideshow_album_id = g_strdup(GE_ALBUM_ALL_ID);
			}
			ge_sdbg("album id: %s", ugd->slideshow_album_id);
		}
		GE_FREE(viewby);
	} else {
		ge_dbgW("vieweby does not exist! set viewby all as default");
		ugd->slideshow_viewby = GE_VIEW_BY_ALL;
		ugd->slideshow_album_id = g_strdup(GE_ALBUM_ALL_ID);
	}

	ge_sdbg("viewby: %d", ugd->slideshow_viewby);
	return 0;
}

/* analysis parameters */
static int _ge_parse_param(ge_ugdata *ugd, app_control_h service)
{
	GE_CHECK_VAL(ugd, -1);
	GE_CHECK_VAL(service, -1);
	char *launch_type = NULL;
	char *select_mode = NULL;
	char *operation = NULL;

	app_control_get_operation(service, &operation);
	ge_sdbg("operation [%s]", operation);

	app_control_get_extra_data(service, APP_CONTROL_DATA_SELECTION_MODE,
	                           &select_mode);
	ugd->limitsize = -1;
	int ret = 0;
	char *max_size = NULL;
	ret = app_control_get_extra_data(service, APP_CONTROL_DATA_TOTAL_SIZE, &(max_size));
	if (ret != APP_CONTROL_ERROR_NONE) {
		ge_dbg("failed in app_control_get_extra_data");
	}

	if (max_size) {
		ugd->limitsize = _ge_atoi(max_size);
		GE_FREE(max_size);
	}
	if (select_mode) {
		if (!strcasecmp(select_mode, GE_BUNDLE_SELECTION_MODE_SINGLE)) {
			launch_type = strdup(GE_LAUNCH_SELECT_ONE);
		} else if (!strcasecmp(select_mode, GE_BUNDLE_SELECTION_MODE_MULTI)) {
			launch_type = strdup(GE_LAUNCH_SELECT_MULTIPLE);
		}
		GE_FREE(select_mode);
	}
	if (launch_type == NULL)
		app_control_get_extra_data(service, GE_BUNDLE_LAUNCH_TYPE,
		                           &launch_type);
	if (launch_type == NULL) {
		if (operation &&
		        !strcasecmp(operation, APP_CONTROL_OPERATION_PICK)) {
			launch_type = strdup(GE_LAUNCH_SELECT_ONE);
			if (launch_type == NULL) {
				GE_FREEIF(operation);
				ge_dbg("launch-type = NULL, galery ug exit");
				return -1;
			}
			ge_dbg("Set launch-type to default");
		} else {
			GE_FREEIF(operation);
			ge_dbg("launch-type = NULL, galery ug exit");
			return -1;
		}
	}
	ge_dbg("launch-type [%s]", launch_type);

	if (!strcasecmp(launch_type, GE_LAUNCH_SELECT_ALBUM)) {
		ugd->album_select_mode = GE_ALBUM_SELECT_T_ONE;
	} else if (!strcasecmp(launch_type, GE_LAUNCH_SELECT_SETAS)) {
		ugd->file_select_mode = GE_FILE_SELECT_T_SETAS;
		__ge_parse_param_setas(ugd, service);
		/* Checkme: Only image type is offered in SETAS  case */
		ugd->file_type_mode = GE_FILE_T_IMAGE;
	} else {
		if (!strcasecmp(launch_type, GE_LAUNCH_SELECT_ONE)) {
			ugd->file_select_mode = GE_FILE_SELECT_T_ONE;
		} else if (!strcasecmp(launch_type, GE_LAUNCH_SELECT_MULTIPLE)) {
			ugd->file_select_mode = GE_FILE_SELECT_T_MULTIPLE;
			ugd->b_multifile = true;
		} else if (!strcasecmp(launch_type, GE_LAUNCH_SELECT_IMFT)) {
			ugd->file_select_mode = GE_FILE_SELECT_T_IMFT;
			ugd->b_multifile = true;

			char *contact_id = NULL;
			app_control_get_extra_data(service, "contact", &contact_id);
			if (contact_id) {
				ugd->file_select_contact_id = atoi(contact_id);
				GE_FREE(contact_id);
			} else {
				ugd->file_select_mode = GE_FILE_SELECT_T_NONE;
				ugd->file_select_contact_id = -1;
				ge_dbg("Invalid args");
				GE_FREEIF(operation);
				GE_FREE(launch_type);
				return -1;
			}

			ge_sdbg("contact_id: %d", ugd->file_select_contact_id);
		} else if (!strcasecmp(launch_type, GE_LAUNCH_SELECT_SLIDESHOW)) {
			__ge_parse_param_slideshow(ugd, service);
		} else {
			ge_dbgE("Wrong launch type!");
		}

		__ge_parse_param_file(ugd, service);
	}

	if (!ugd->b_multifile) {
		g_strlcpy(ugd->albums_view_title, GE_STR_ID_SELECT_ITEM,
		          sizeof(ugd->albums_view_title));
	} else {
		g_strlcpy(ugd->albums_view_title, GE_STR_ID_SELECT_ITEMS,
		          sizeof(ugd->albums_view_title));
		/* Get maximum number */
		ugd->max_count = -1;
		char *max_cnt = NULL;
		app_control_get_extra_data(service, APP_CONTROL_DATA_TOTAL_COUNT, &max_cnt);
		if (max_cnt) {
			ugd->max_count = _ge_atoi(max_cnt);
			GE_FREE(max_cnt);
		}
		ge_sdbg("max count: %d", ugd->max_count);
	}

	if (!ugd->overlap_mode) {
		char *indicator = NULL;
		app_control_get_extra_data(service, "indicator-state", &indicator);
		if (indicator) {
			ge_dbg("indicator: %s", indicator);
			if (!strcasecmp(indicator, "hide")) {
				ugd->b_hide_indicator = true;
			}
			GE_FREE(indicator);
		}
	}

	GE_FREEIF(operation);
	GE_FREE(launch_type);
	return 0;
}

static int __ge_get_rotate_value(ge_ugdata *ugd)
{
	GE_CHECK_VAL(ugd, -1);
	GE_CHECK_VAL(ugd->win, -1);
	/* changed_ang value is 0 90 180 270 */
	int changed_ang = elm_win_rotation_get(ugd->win);
	/* Send event to UG */
	switch (changed_ang) {
	case APP_DEVICE_ORIENTATION_0:
		ugd->rotate_mode = GE_ROTATE_PORTRAIT;
		break;
	case APP_DEVICE_ORIENTATION_90:
		ugd->rotate_mode = GE_ROTATE_LANDSCAPE_UPSIDEDOWN;
		break;
	case APP_DEVICE_ORIENTATION_180:
		ugd->rotate_mode = GE_ROTATE_PORTRAIT_UPSIDEDOWN;
		break;
	case APP_DEVICE_ORIENTATION_270:
		ugd->rotate_mode = GE_ROTATE_LANDSCAPE;
		break;
	}
	return ugd->rotate_mode;
}

/**
* @brief
*
* @param ug
* @param mode
* @param data
* @param priv
*
* @return
*/
void _ge_create(void *priv)
{
	ge_dbg("Enter...");
	ge_ugdata *ugd = NULL;
	GE_CHECK(priv);
	ge_dbgW("Gallery UG start...");

	ugd = (ge_ugdata *)priv;
		//	ugd->ug = ug;
	ugd->attach_panel_display_mode = ATTACH_PANEL_NONE;
	/* Get window */
	
	ugd->win = elm_win_util_standard_add("ug-gallery-efl", "ug-gallery-efl");

	GE_CHECK(ugd->win);
	elm_win_conformant_set(ugd->win, EINA_TRUE);
	elm_win_autodel_set(ugd->win, EINA_TRUE);

	/* Get conformant */
	Evas_Object *parent = elm_conformant_add(ugd->win);
	if (!parent) {
		return ;
	}
	evas_object_size_hint_weight_set(parent, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(parent);
	evas_object_show(ugd->win);
	ugd->conform = parent;

	/* Get caller layout */
//	ugd->ly_parent = elm_layout_add(ugd->win);
//	GE_CHECK(ugd->ly_parent);
	/* Bind text domain for internalization */
	char locale[1024] = {0};
	char * path = app_get_resource_path();
	ge_dbgW("Gallery UG start...and resource path is : %s", path);
	snprintf(locale, 1024, "%s%s", path, "locale");
	bindtextdomain("ug-gallery-efl" , locale);
	free(path);
	/* Reset inited flag, it would be set as TRUE if albums view created */
	_ge_ui_get_indicator_state(ugd);
#ifdef _USE_HIDE_INDICATOR
	_ge_ui_hide_indicator(ugd);
#endif
	/* Add window rotation callback to rotate view as fast as possible */
	evas_object_smart_callback_add(ugd->win, "wm,rotation,changed",
			__ge_win_rot_changed_cb, (void *)ugd);
	__ge_get_rotate_value(ugd);

		/* Connect DB first */
	if (_ge_data_init(ugd) != 0) {
		ge_dbgE("_ge_data_init failed!");
	}
	/*Register db udpate callback*/
	_ge_db_update_reg_cb(ugd);
	/* Parse parameters passed from parent */

	ugd->is_attach_panel = false;

	if (ugd->overlap_mode) {
		ugd->b_hide_indicator = true;
	} else if (ugd->b_hide_indicator) {
		_ge_ui_hide_indicator(ugd);
	} else {
		ge_dbg("Normal mode");
	}



}

/**
* @brief
*
* @param ug
* @param data
* @param priv
*/
#ifdef _UG_UI_CONVERSION
static void _ge_start(void *priv)
{
}
#endif

/**
* @brief
*
* @param ug
* @param data
* @param priv
*/
void _ge_pause(void *priv)
{
	ge_dbg("");
}

/**
* @brief
*
* @param ug
* @param data
* @param priv
*/
void _ge_resume(void *priv)
{
	ge_dbg("");
	GE_CHECK(priv);
	ge_ugdata *ugd = (ge_ugdata *)priv;
	/*update*/
	if (ugd->file_select_mode != GE_FILE_SELECT_T_SLIDESHOW) {
//		ge_update_view(ugd);
	}
}

/**
* @brief
*
* @param ug
* @param data
* @param priv
*/
void _ge_destroy(void *priv)
{
	ge_dbgW("");
	GE_CHECK(priv);
	_ge_close_view((ge_ugdata *)priv);
	ge_dbgW("Destroy gallery UG done!");
}

/**
* @brief
*
* @param ug
* @param msg
* @param data
* @param priv
*/

#ifdef _UG_UI_CONVERSION
static void _ge_message(ui_gadget_h ug, app_control_h msg, app_control_h service, void *priv)
{
	ge_dbg("");
	GE_CHECK(priv);
	ge_ugdata *ugd = (ge_ugdata *)priv;
	char *display_mode = NULL;
	if (ugd->is_attach_panel) {
		ge_dbg("called by attach panel ");
		app_control_get_extra_data(msg, APP_CONTROL_DATA_SELECTION_MODE, &display_mode);
		if (display_mode) {
			if (!strcmp(display_mode, "single")) {
				//change to single selection
				ge_dbg("single_selection ");
				ugd->attach_panel_display_mode = ATTACH_PANEL_HALF_MODE;
				ugd->file_select_mode = GE_FILE_SELECT_T_ONE;
				ugd->b_multifile = false;
				_ge_grid_change_selection_mode(ugd, false);
			} else if (display_mode && !strcmp(display_mode, "multiple")) {
				//change to multi selection
				ge_dbg("multiple_selection");
				ugd->attach_panel_display_mode = ATTACH_PANEL_FULL_MODE;
				ugd->file_select_mode = GE_FILE_SELECT_T_MULTIPLE;
				ugd->b_multifile = true;
				_ge_grid_change_selection_mode(ugd, true);
			} else {
				ge_dbg("invalid mode: %s", display_mode);
			}
		}
	}
}
#endif

/**
* @brief
*
* @param ug
* @param event
* @param data
* @param priv
*/

#ifdef _UG_UI_CONVERSION
static void _ge_event(ui_gadget_h ug, enum ug_event event, app_control_h service, void *priv)
{
	GE_CHECK(priv);
	ge_ugdata *ugd = (ge_ugdata *)priv;

	switch (event) {
	case UG_EVENT_LOW_MEMORY:
		ge_dbg("UG_EVENT_LOW_MEMORY");
		break;

	case UG_EVENT_LOW_BATTERY:
		ge_dbg("UG_EVENT_LOW_BATTERY");
		break;

	case UG_EVENT_LANG_CHANGE:
		ge_dbg("UG_EVENT_LANG_CHANGE");
		_ge_lang_update(ugd);
		break;

	case UG_EVENT_ROTATE_PORTRAIT:
		ge_dbg("UG_EVENT_ROTATE_PORTRAIT");
		goto UG_ROTATE_EVENT;
		break;

	case UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN:
		ge_dbg("UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN");
		goto UG_ROTATE_EVENT;
		break;

	case UG_EVENT_ROTATE_LANDSCAPE:
		ge_dbg("UG_EVENT_ROTATE_LANDSCAPE");
		goto UG_ROTATE_EVENT;
		break;

	case UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN:
		ge_dbg("UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN");
		goto UG_ROTATE_EVENT;
		break;

	default:
		break;
	}

	return;

UG_ROTATE_EVENT:

	/* Update rotate mode and view */
	ge_dbg("rotate_mode: %d", ugd->rotate_mode);
}


static void _ge_key_event(ui_gadget_h ug, enum ug_key_event event, app_control_h service, void *priv)
{
	ge_dbg("");
	GE_CHECK(ug);
	GE_CHECK(priv);
	ge_ugdata *ugd = (ge_ugdata *)priv;
	switch (event) {
	case UG_KEY_EVENT_END:
		ge_dbg("Receive key end event");
		app_control_destroy(ugd->service);
		break;

	default:
		break;
	}
}
#endif

void __ge_get_app_control_data( app_control_h app_control,void *data)
{
	ge_ugdata *ugd = NULL;
	GE_CHECK(data);
	ugd = (ge_ugdata *)data;
	app_control_clone(&(ugd->service), app_control);

	if (_ge_parse_param(ugd, app_control) != 0) {
			ge_dbgE("Failed to parse parameters!");
			bool reply_requested = false;
			app_control_is_reply_requested(ugd->service, &reply_requested);
			if (reply_requested) {
				ge_sdbg("send reply to caller");
				app_control_h reply = NULL;
				app_control_create(&reply);
				app_control_reply_to_launch_request(reply, ugd->service, APP_CONTROL_RESULT_FAILED);
				app_control_destroy(reply);
			}
			if (ugd->service) {
					app_control_destroy(ugd->service);
					ugd->service = NULL;
				}
			ui_app_exit();
			return;
		}
		if ((ugd->file_select_mode != GE_FILE_SELECT_T_NONE) ||
		        (ugd->album_select_mode != GE_ALBUM_SELECT_T_NONE)) {
			/* create gallery UG */
			if (_ge_create_view(ugd) != 0) {
				ge_dbgE("Failed to create Gallery UG view!");
				return;
			}
		} else {
			ge_dbgE("Wrong file_select_mode[%d] or album_select_mode[%d]",
			        ugd->file_select_mode, ugd->album_select_mode);
		}
		elm_win_resize_object_add(ugd->win, ugd->ly_main);

}

/**
* @brief
*
* @param ops
*
* @return
*/
int main(int argc, char *argv[])
{
	ge_dbgW("UG_MODULE_INIT");
	struct _ge_ugdata ugd;


	ui_app_lifecycle_callback_s ops;
	int ret = APP_ERROR_NONE;
	app_event_handler_h hLanguageChangedHandle;
	app_event_handler_h hRegionFormatChangedHandle;
	memset(&ops, 0x0, sizeof(ui_app_lifecycle_callback_s));
	memset(&ugd, 0x0, sizeof(struct _ge_ugdata));
	ops.create = _ge_create;
	ops.terminate = _ge_destroy;
	ops.pause = _ge_pause;
	ops.resume = _ge_resume;
	ops.app_control =__ge_get_app_control_data;
	ret = ui_app_add_event_handler(&hRegionFormatChangedHandle, APP_EVENT_REGION_FORMAT_CHANGED, _ge_lang_update, (void*)&ugd);
	if (ret != APP_ERROR_NONE) {
		ge_dbgE("APP_EVENT_REGION_FORMAT_CHANGED ui_app_add_event_handler failed : [%d]!!!", ret);
		return -1;
	}

	ret = ui_app_add_event_handler(&hLanguageChangedHandle, APP_EVENT_LANGUAGE_CHANGED, _ge_lang_update, (void*)&ugd);
	if (ret != APP_ERROR_NONE) {
		ge_dbgE("APP_EVENT_LANGUAGE_CHANGED ui_app_add_event_handler failed : [%d]!!!", ret);
		return -1;
	}
	return ui_app_main(argc, argv, &ops, &ugd);

}

/**
* @brief
*
* @param ops
*
* @return
*/

#ifdef _UG_UI_CONVERSION
UG_MODULE_API void UG_MODULE_EXIT(struct ug_module_ops *ops)
{
	GE_CHECK(ops);
	GE_FREEIF(ops->priv);
	ge_dbgW("UG_MODULE_EXIT");
}
#endif
