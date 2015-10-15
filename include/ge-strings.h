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

#ifndef _GE_STRINGS_H_
#define _GE_STRINGS_H_

#ifdef _cplusplus
extern "C" {
#endif

#define GE_STR_DOMAIN_SYS "sys_string"
#define GE_STR_DOMAIN_LOCAL "ug-gallery-efl"

#define _GE_GETSYSTEMSTR(ID) dgettext(GE_STR_DOMAIN_SYS, (ID))
#define _GE_GETLOCALSTR(ID) dgettext(GE_STR_DOMAIN_LOCAL, (ID))
/* hard code strings */
#define GE_ALBUM_CAMERA_NAME	_("Camera")
#define GE_POPUP_ONLY_SUPPORT_IMAGE	_("Only image is supported!")
#define GE_ALBUM_DOWNLOADS_NAME	_("Downloads")
#define GE_POPUP_ADD_SHORTCUT_FAILED	_("Add shortcut failed!")
#define GE_STR_SR_TITLE _("title")
#define GE_STR_ID_SELECT_ITEM "IDS_MEDIABR_HEADER_SELECT_ITEM"
#define GE_STR_ID_SELECT_ITEMS _GE_GETLOCALSTR("IDS_MEDIABR_BODY_SELECT_ITEMS")//"IDS_MEDIABR_BODY_SELECT_ITEMS"
#define GE_STR_ID_ALBUMS "IDS_COM_BODY_ALBUMS"
#define GL_STR_ID_TIME_LINE "IDS_MEDIABR_HEADER_TIMELINE_ABB"
#define GE_STR_ID_CANCEL "IDS_COM_SK_CANCEL"
#define GE_STR_ID_DONE "IDS_COM_SK_DONE"
#define GE_STR_ID_START "IDS_COM_BODY_START"
#define GE_STR_ID_BACK "IDS_COM_SK_BACK"
#define GE_STR_ID_OK "IDS_COM_SK_OK"
#define GE_STR_ID_CANCEL_CAP _GE_GETLOCALSTR("IDS_TPLATFORM_ACBUTTON_CANCEL_ABB")
#define GE_STR_ID_DONE_CAP _GE_GETLOCALSTR("IDS_TPLATFORM_ACBUTTON_DONE_ABB")

#define GE_ALBUM_NAME _GE_GETLOCALSTR("IDS_MEDIABR_BUTTON_ALBUMS")
#define GE_ALBUM_ALL_NAME		_GE_GETLOCALSTR("IDS_MEDIABR_BODY_ALL_ALBUMS")
#define GE_STR_NO_ALBUMS _GE_GETLOCALSTR("IDS_MEDIABR_BODY_NO_ALBUMS")
#define GE_ALBUM_ROOT_NAME		_GE_GETLOCALSTR("IDS_MEDIABR_POP_NO_NAME")
#define GE_STR_PD_SELECTED	_GE_GETLOCALSTR("IDS_MEDIABR_HEADER_PD_SELECTED")
#define GE_STR_UNSUPPORTED_FILE _GE_GETLOCALSTR("IDS_GALLERY_TPOP_FILE_CORRUPT_OR_IN_UNSUPPORTED_FORMAT")

/* system strings which are included in sys-string-0 po files */
#define GE_STR_CAMERA 		_GE_GETLOCALSTR("IDS_GALLERY_BUTTON_CAMERA")
#define GE_STR_DOWNLOADS 	_GE_GETLOCALSTR("IDS_MF_BODY_DOWNLOADS")
#define GE_STR_NO_VIDEOS _GE_GETSYSTEMSTR("IDS_COM_BODY_NO_VIDEOS")
#define GE_STR_NO_IMAGES _GE_GETSYSTEMSTR("IDS_COM_BODY_NO_IMAGES")
#define GE_STR_NO_ITEMS _GE_GETSYSTEMSTR("IDS_COM_BODY_NO_ITEMS")
#define GE_STR_SELECTED _GE_GETSYSTEMSTR("IDS_COM_POP_SELECTED")
#define GE_STR_UNKNOWN _GE_GETSYSTEMSTR("IDS_COM_BODY_UNKNOWN")
#define GE_STR_MAXIMUM_COUNT _GE_GETLOCALSTR("IDS_IV_POP_YOU_CAN_SELECT_UP_TO_PD_ITEMS")
#define GE_STR_MAXIMUM_SIZE _GE_GETLOCALSTR("IDS_IM_POP_MAXIMUM_SIZE_OF_FILES_EXCEEDED")

#define ATTACH_PANEL_FLICK_MODE_ENABLE "enable"
#define ATTACH_PANEL_FLICK_MODE_DISABLE "disable"
#define ATTACH_PANEL_FLICK_MODE_KEY "__ATTACH_PANEL_FLICK_DOWN__"

#ifdef _cplusplus
}
#endif
#endif //_GE_STRINGS_H_
