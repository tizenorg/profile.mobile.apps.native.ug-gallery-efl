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

#ifndef _GE_EXT_UG_LOAD_H_
#define _GE_EXT_UG_LOAD_H_

#include <ui-gadget.h>
#include "gallery-efl.h"

typedef enum {
	GE_UG_FAIL,
	GE_UG_SUCCESS
} ge_ug_load_status;

typedef enum {
	GE_UG_IMAGEVIEWER,
	GE_UG_GALLERY_SETTING_SLIDESHOW,
	GE_UG_GALLERY_SETTING,
	GE_UG_CNT,
} ge_ext_ug_type;

typedef enum {
	GE_UG_IV,
	/*Invoke imageviewer to slideshow on local album */
	GE_UG_IV_SLIDESHOW,
	/*Invoke imageviewer to slideshow All items */
	GE_UG_IV_SLIDESHOW_ALL,
#ifdef _USE_APP_SLIDESHOW
	/*Invoke imageviewer to slideshow selected files */
	GE_UG_IV_SLIDESHOW_SELECTED,
#endif
	GE_UG_IV_CNT,
} ge_ext_iv_type;

int _ge_ext_load_iv_ug(ge_ugdata *ugd, char *filt_url, char *album_index, int image_index);
int _ge_ext_load_iv_ug_for_help(ge_ugdata *ugd, const char *uri);
int _ge_ext_load_iv_ug_select_mode(void *data, ge_media_s *item, ge_ext_iv_type type);

#endif //_GE_EXT_UG_LOAD_H_

