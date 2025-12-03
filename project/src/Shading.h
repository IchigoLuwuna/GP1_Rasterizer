#ifndef SHADING_H
#define SHADING_H
#include "ColorRGB.h"
#include "DataTypes.h"

namespace dae
{
ColorRGB GetPixelColor( const Triangle_Out& triangle, const Vector3 baryCentricPosition );
}

#endif
