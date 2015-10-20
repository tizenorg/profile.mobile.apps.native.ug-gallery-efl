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

#ifndef _GE_ALBUMS_H_
#define _GE_ALBUMS_H_

#include "gallery-efl.h"

int _ge_albums_create_view(ge_ugdata *ugd);
int _ge_albums_update_view(ge_ugdata *ugd);
Evas_Object* __ge_add_albums_split_view(ge_ugdata *ugd, Evas_Object *parent);
int __ge_split_view_append_albums(ge_ugdata *ugd, Evas_Object *parent, bool is_update);

#endif /* _GE_ALBUMS_H_ */

