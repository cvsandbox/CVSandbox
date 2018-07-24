/*
    Imaging library of Computer Vision Sandbox

    Copyright (C) 2011-2018, cvsandbox
    http://www.cvsandbox.com/contacts.html

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "ximaging.h"
#include <math.h>

// -------------------------------------------------------------------
// ========================= HSL color space =========================
// -------------------------------------------------------------------

// Convert RGB color to HSL
XErrorCode Rgb2Hsl( const xargb* rgb, xhsl* hsl )
{
    XErrorCode ret = SuccessCode;

    if ( ( rgb == 0 ) || ( hsl == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        float r = ( rgb->components.r / 255.0f );
        float g = ( rgb->components.g / 255.0f );
        float b = ( rgb->components.b / 255.0f );

        float min   = XMIN3( r, g, b );
        float max   = XMAX3( r, g, b );
        float delta = max - min;

        // get luminance value
        hsl->Luminance = ( max + min ) / 2;

        if ( delta == 0 )
        {
            // gray color
            hsl->Hue = 0;
            hsl->Saturation = 0.0f;
        }
        else
        {
            float hue;

            // get saturation value
            hsl->Saturation = ( hsl->Luminance <= 0.5 ) ? ( delta / ( max + min ) ) : ( delta / ( 2 - max - min ) );

            // get hue value
            if ( r == max )
            {
                hue = ( g - b ) / delta;

                // correct hue if needed
                if ( hue < 0 ) hue += 6;
            }
            else if ( g == max )
            {
                hue = 2.0f + ( b - r ) / delta;
            }
            else
            {
                hue = 4.0f + ( r - g ) / delta;
            }

            hsl->Hue = (uint16_t) ( hue * 60 );
        }
    }

    return ret;
}

// Convert RGB color to SL part only of HSL (hue is not calculated)
XErrorCode Rgb2Sl( const xargb* rgb, xhsl* hsl )
{
    XErrorCode ret = SuccessCode;

    if ( ( rgb == 0 ) || ( hsl == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        float r = ( rgb->components.r / 255.0f );
        float g = ( rgb->components.g / 255.0f );
        float b = ( rgb->components.b / 255.0f );

        float min   = XMIN3( r, g, b );
        float max   = XMAX3( r, g, b );
        float delta = max - min;

        // get luminance value
        hsl->Luminance = ( max + min ) / 2;

        if ( delta == 0 )
        {
            // gray color
            hsl->Saturation = 0.0f;
        }
        else
        {
            // get saturation value
            hsl->Saturation = ( hsl->Luminance <= 0.5 ) ? ( delta / ( max + min ) ) : ( delta / ( 2 - max - min ) );
        }

        hsl->Hue = 0;
    }

    return ret;
}

// Helper function for HSL to RGB conversion
static float Hue_2_RGB( float v1, float v2, float vH )
{
    if ( vH < 0 )
        vH += 1;
    if ( vH > 1 )
        vH -= 1;
    if ( vH < 1.0f / 6 )
        return ( v1 + ( v2 - v1 ) * 6 * vH );
    if ( vH < 0.5f )
        return v2;
    if ( vH < 2.0f / 3 )
        return ( v1 + ( v2 - v1 ) * ( ( 2.0f / 3 ) - vH ) * 6 );
    return v1;
}

// Convert HSL color to RGB
XErrorCode Hsl2Rgb( const xhsl* hsl, xargb* rgb )
{
    XErrorCode ret = SuccessCode;

    // HSL values are not checked - we rely on the correct range

    if ( ( rgb == 0 ) || ( hsl == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        if ( hsl->Saturation == 0 )
        {
            // gray values
            rgb->components.r = rgb->components.g = rgb->components.b = (uint8_t) ( hsl->Luminance * 255 );
        }
        else
        {
            float v1, v2;
            float hue = (float) hsl->Hue / 360;

            v2 = ( hsl->Luminance < 0.5 ) ?
                 ( hsl->Luminance * ( 1 + hsl->Saturation ) ) :
                 ( ( hsl->Luminance + hsl->Saturation ) - ( hsl->Luminance * hsl->Saturation ) );
            v1 = 2 * hsl->Luminance - v2;

            rgb->components.r = (uint8_t) ( 255 * Hue_2_RGB( v1, v2, hue + ( 1.0f / 3 ) ) );
            rgb->components.g = (uint8_t) ( 255 * Hue_2_RGB( v1, v2, hue ) );
            rgb->components.b = (uint8_t) ( 255 * Hue_2_RGB( v1, v2, hue - ( 1.0f / 3 ) ) );
        }
        rgb->components.a = 255;
    }

    return ret;
}

// -------------------------------------------------------------------
// ========================= HSV color space =========================
// -------------------------------------------------------------------

// Convert RGB color to HSV
// (standard implementation)
/*
XErrorCode Rgb2Hsv( const xargb* rgb, xhsv* hsv )
{
    XErrorCode ret = SuccessCode;

    if ( ( rgb == 0 ) || ( hsv == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        float r = ( rgb->components.r / 255.0f );
        float g = ( rgb->components.g / 255.0f );
        float b = ( rgb->components.b / 255.0f );

        float min   = XMIN3( r, g, b );
        float max   = XMAX3( r, g, b );
        float delta = max - min;

        hsv->Value = max;

        if ( max == 0 )
        {
            hsv->Saturation = 0;
            hsv->Hue        = 0;
        }
        else
        {
            hsv->Saturation = delta / max;

            if ( delta == 0 )
            {
                hsv->Hue = 0;
            }
            else
            {
                float hue;

                // get hue value
                if ( r == max )
                {
                    hue = ( g - b ) / delta;

                    // correct hue if needed
                    if ( hue < 0 ) hue += 6;
                }
                else if ( g == max )
                {
                    hue = 2.0f + ( b - r ) / delta;
                }
                else
                {
                    hue = 4.0f + ( r - g ) / delta;
                }

                hsv->Hue = (uint16_t) ( hue * 60 );
            }
        }
    }

    return ret;
}
*/

// Convert RGB color to HSV
// (optimized version, see the link below)
// http://lolengine.net/blog/2013/01/13/fast-rgb-to-hsv
//
XErrorCode Rgb2Hsv( const xargb* rgb, xhsv* hsv )
{
    XErrorCode ret = SuccessCode;

    if ( ( rgb == 0 ) || ( hsv == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        float r = ( rgb->components.r / 255.0f );
        float g = ( rgb->components.g / 255.0f );
        float b = ( rgb->components.b / 255.0f );
        float K = 0.f;

        if ( g < b )
        {
            float temp = g; g = b; b = temp;
            K = -1.f;
        }

        if ( r < g )
        {
            float temp = r; r = g; g = temp;
            K = -2.f / 6.f - K;
        }

        float chroma = r - ( ( g < b ) ? g : b );
        float hue    =  K + ( g - b ) / ( 6.f * chroma + 1e-20f );

        if ( hue < 0 ) hue = -hue;

        hsv->Hue        = (uint16_t) ( 360 * hue );
        hsv->Saturation = chroma / ( r + 1e-20f );
        hsv->Value      = r;
    }

    return ret;
}

// Convert HSV color to RGB
XErrorCode Hsv2Rgb( const xhsv* hsv, xargb* rgb )
{
    XErrorCode ret = SuccessCode;

    if ( ( rgb == 0 ) || ( hsv == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        if ( hsv->Saturation == 0 )
        {
            // gray values
            rgb->components.r = rgb->components.g = rgb->components.b = (uint8_t) ( hsv->Value * 255 );
        }
        else
        {
            float p, q, t, hh, ff;
            float r, g, b;
            int   i;

            hh = hsv->Hue;
            if ( hh >= 360.0 ) hh = 0.0;
            hh /= 60.0;

            i = (long) hh;
            ff = hh - i;

            p = hsv->Value * ( 1.0f -   hsv->Saturation );
            q = hsv->Value * ( 1.0f - ( hsv->Saturation * ff ) );
            t = hsv->Value * ( 1.0f - ( hsv->Saturation * ( 1.0f - ff ) ) );

            switch ( i )
            {
            case 0:
                r = hsv->Value;
                g = t;
                b = p;
                break;
            case 1:
                r = q;
                g = hsv->Value;
                b = p;
                break;
            case 2:
                r = p;
                g = hsv->Value;
                b = t;
                break;
            case 3:
                r = p;
                g = q;
                b = hsv->Value;
                break;
            case 4:
                r = t;
                g = p;
                b = hsv->Value;
                break;
            case 5:
            default:
                r = hsv->Value;
                g = p;
                b = q;
                break;
            }

            rgb->components.r = (uint8_t) ( r * 255 );
            rgb->components.g = (uint8_t) ( g * 255 );
            rgb->components.b = (uint8_t) ( b * 255 );
            rgb->components.a = 255;
        }
    }

    return ret;
}
