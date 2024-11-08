%module RSPiX
%{
int _argc;
char** _argv;
#include "RSPiX.h"
#include "BLUE/Blue.h"
#include "GREEN/InputEvent/InputEvent.h"
#include "GREEN/Sample/sample.h"
#include "GREEN/Image/PalFile.h"
#include "GREEN/Image/SpecialTyp.h"
#include "GREEN/Image/ImageFile.h"
#include "GREEN/Image/pal.h"
#include "GREEN/Image/Image.h"
#include "GREEN/3D/pipeline.h"
#include "GREEN/3D/user3d.h"
#include "GREEN/3D/render.h"
#include "GREEN/3D/types3d.h"
#include "GREEN/3D/zbuffer.h"
#include "GREEN/SndFx/SndFx.h"
#include "GREEN/Task/task.h"
#include "GREEN/Hot/hot.h"
#include "GREEN/Mix/MixBuf.h"
#include "GREEN/Mix/mix.h"
#include "GREEN/Snd/snd.h"
#include "ORANGE/QuickMath/Fractions.h"
#include "ORANGE/QuickMath/FixedPoint.h"
#include "ORANGE/QuickMath/QuickMath.h"
#include "ORANGE/QuickMath/VectorMath.h"
#include "ORANGE/str/str.h"
#include "ORANGE/Meter/meter.h"
#include "ORANGE/Laymage/laymage.h"
#include "ORANGE/RString/rstring.h"
#include "ORANGE/DirtRect/DirtRect.h"
#include "ORANGE/Parse/SimpleBatch.h"
#include "ORANGE/MultiGrid/MultiGrid.h"
#include "ORANGE/MultiGrid/MultiGridIndirect.h"
#include "ORANGE/Channel/channel.h"
#include "ORANGE/GameLib/Shapes.h"
#include "ORANGE/GameLib/ANIMSPRT.H"
#include "ORANGE/GameLib/PAL.H"
#include "ORANGE/GameLib/SPRITE.H"
#include "ORANGE/GameLib/GRIP.H"
#include "ORANGE/Attribute/attribute.h"
#include "ORANGE/IFF/iff.h"
#include "ORANGE/MsgBox/MsgBox.h"
#include "ORANGE/File/file.h"
#include "ORANGE/Props/Props.h"
#include "ORANGE/Debug/profile.h"
#include "ORANGE/CDT/List.h"
#include "ORANGE/CDT/QUEUE.H"
#include "ORANGE/CDT/PQueue.h"
#include "ORANGE/CDT/pixel.h"
#include "ORANGE/CDT/fqueue.h"
#include "ORANGE/CDT/slist.h"
#include "ORANGE/CDT/stack.h"
#include "ORANGE/CDT/flist.h"
#include "ORANGE/CDT/listbase.h"
#include "ORANGE/ImageTools/lasso.h"
typedef RFList<RSprite*> ListOfSprites;
#include "WishPiX/Spry/spry.h"
%}

%include <stdint.i>
%include <cpointer.i>
%include "BLUE/Blue.h"
%include "GREEN/InputEvent/InputEvent.h"
%include "GREEN/Sample/sample.h"
%include "GREEN/Image/PalFile.h"
%include "GREEN/Image/SpecialTyp.h"
%include "GREEN/Image/ImageFile.h"
%include "GREEN/Image/pal.h"
%include "GREEN/Image/Image.h"
%include "GREEN/3D/pipeline.h"
%include "GREEN/3D/user3d.h"
%include "GREEN/3D/render.h"
%include "GREEN/3D/types3d.h"
%include "GREEN/3D/zbuffer.h"
%include "GREEN/SndFx/SndFx.h"
%include "GREEN/Task/task.h"
%include "GREEN/Hot/hot.h"
%include "GREEN/Mix/MixBuf.h"
%include "GREEN/Mix/mix.h"
%include "GREEN/Snd/snd.h"
%include "ORANGE/QuickMath/Fractions.h"
%include "ORANGE/QuickMath/FixedPoint.h"
%include "ORANGE/QuickMath/QuickMath.h"
%include "ORANGE/QuickMath/VectorMath.h"
%include "ORANGE/str/str.h"
%include "ORANGE/Meter/meter.h"
%include "ORANGE/Laymage/laymage.h"
%include "ORANGE/RString/rstring.h"
%include "ORANGE/DirtRect/DirtRect.h"
%include "ORANGE/Parse/SimpleBatch.h"
%include "ORANGE/MultiGrid/MultiGrid.h"
%include "ORANGE/MultiGrid/MultiGridIndirect.h"
%include "ORANGE/Channel/channel.h"
%include "ORANGE/GameLib/Shapes.h"
%include "ORANGE/GameLib/ANIMSPRT.H"
%include "ORANGE/GameLib/PAL.H"
%include "ORANGE/GameLib/SPRITE.H"
%include "ORANGE/GameLib/GRIP.H"
%include "ORANGE/Attribute/attribute.h"
%include "ORANGE/IFF/iff.h"
%include "ORANGE/MsgBox/MsgBox.h"
%include "ORANGE/File/file.h"
%include "ORANGE/Props/Props.h"
%include "ORANGE/Debug/profile.h"
%include "ORANGE/CDT/List.h"
%include "ORANGE/CDT/QUEUE.H"
%include "ORANGE/CDT/PQueue.h"
%include "ORANGE/CDT/pixel.h"
%include "ORANGE/CDT/fqueue.h"
%include "ORANGE/CDT/slist.h"
%include "ORANGE/CDT/stack.h"
%include "ORANGE/CDT/flist.h"
%include "ORANGE/CDT/listbase.h"
%include "ORANGE/ImageTools/lasso.h"
%template(ListOfSprites) RFList<RSprite*>;
%include "WishPiX/Spry/spry.h"

%inline %{

FILE* fopen (const char* filename, const char* mode);

RSprite* ppRSprDereference(RSprite** myPtr)
{
	return *myPtr;
}

uint8_t* allocateFile(uint32_t bytes)
{
	return (uint8_t *) malloc(bytes);
}

#define getter(name, type) type name ( type * buf, uintptr_t pos ) { return * ( buf + pos ); }

getter(getByte, uint8_t);
getter(getU16, uint16_t);
getter(getU32, uint32_t);

#define colourGetter(name, retvar) uint8_t name ( RPal * palette , int16_t num ) { uint8_t dstRed; uint8_t dstGreen; uint8_t dstBlue; palette->GetEntries( num, 1, &dstRed, &dstGreen, &dstBlue, 1 ); return retvar ; }

colourGetter(getRed, dstRed);
colourGetter(getGreen, dstGreen);
colourGetter(getBlue, dstBlue);

void malphagen(const char* in, const char* out);

%}
