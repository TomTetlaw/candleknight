#ifndef INCLUDES_H
#define INCLUDES_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <direct.h>
#include <sys/stat.h>
#ifdef _WINDOWS
#include <windows.h>
#endif
#include "include/glew.h"
#include "include/stb_image.h"
#include "include/stb_image_write.h"
#include "include/stb_truetype.h"
#include "include/sdl2/sdl.h"
#include "include/sdl2/sdl_syswm.h"
#include "include/tinydir.h"

#undef assert

#include "mathlib.h"
#include "utils.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"

#include "array.h"
#include "render.h"
#include "texture.h"
#include "font.h"
#include "hotload.h"
#include "property.h"
#include "particle.h"
#include "physics.h"
#include "entity.h"
#include "entity_types.h"
#include "textedit.h"
//#include "ui.h"
#include "editor.h"

#endif
