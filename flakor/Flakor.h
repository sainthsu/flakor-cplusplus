#ifndef _FK_FLAKOR_H_
#define _FK_FLAKOR_H_

#define FLAKOR_VERSION 0x00000100

//macro
#include "macros.h"

//core opengl
#include "core/opengl/GL.h"

//lang include
#include "base/lang/Object.h"
#include "base/lang/Array.h"
#include "base/lang/AutoreleasePool.h"
#include "base/lang/Types.h"
#include "base/lang/DataVisitor.h"
#include "base/lang/Dictionary.h"
#include "base/lang/Set.h"
#include "base/lang/FKString.h"
#include "base/lang/Zone.h"

//event
#include "base/event/Touch.h"

//element
#include "base/element/Element.h"
#include "base/element/Helper.h"
#include "base/element/Color.h"
#include "base/element/Type.h"

//entity
#include "base/entity/Entity.h"
#include "base/entity/Scene.h"
#include "base/entity/Sprite.h"

#include "core/texture/Texture2D.h"

const char* getFlakorVersion()
{
       return "flakor engine 0.1.0beta";
}

#endif
