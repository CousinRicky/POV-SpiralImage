/* spiralimage.pov version 2.0
 * Persistence of Vision Raytracer scene description file
 * A proposed POV-Ray Object Collection demo
 *
 * Demo scene using spiralimage.inc: an Archimedean spiral impression of an
 * image map.
 *
 * Copyright © 2022 Richard Callwood III.  Some rights reserved.
 * This file is licensed under the terms of the CC-LGPL
 * a.k.a. the GNU Lesser General Public License version 2.1.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  Please
 * visit https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html for
 * the text of the GNU Lesser General Public License version 2.1.
 *
 * Vers  Date         Notes
 * ----  ----         -----
 * 1.0   2022-Feb-03  Created.
 *       2022-Feb-09  The spiral outside the image map is darkened.
 *       2022-Feb-13  Macro SpImg_Size_uv() is demoed.
 * 2.0   2022-Feb-13  An RGB color mode is added.
 */
// +W800 +H600 +A +R5 +FJ
// +W800 +H600 +A0.0 +R5 -J Declare=Color=1 +Ospiralimage_color
// +W160 +H120 +A0.0 +R5 +Ospiralimage_thumbnail Declare=Radius=20 +FJ
//(Omit the +FJ for POV-Ray older than 3.7.)
#version max (3.5, min (3.8, version));

#ifndef (Color) #declare Color = no; #end
#ifndef (Radius) #declare Radius = (Color? 30: 45); #end

#include "spiralimage.inc"

global_settings { assumed_gamma 1 }
#default { finish { diffuse 0 ambient #if (version >= 3.7) 0 emission #end 1 } }

#declare Size = 2 * Radius + 1;
camera
{ orthographic
  location -2 * z
  right Size * x
  up Size * image_height / image_width * y
}

#if (Color)

  #include "colors.inc"

  #local Hue = 0;
  #while (Hue < 359)
    #local C = color (CHSV2RGB (<Hue, 1, 1>));
    object
    { SpiralImage
      ( pigment { image_map { jpeg "spiralimage_sample" interpolate 2 } },
        SpImg_Size_uv (Radius, 4/3), Hue, 1, 1/3, 1/30, 1/6, C,
        SPIMG_UNION, 5
      )
      pigment { C }
    }
    #local Hue = Hue + 120;
  #end

  background { Black }

#else

  object
  { SpiralImage
    ( pigment { image_map { jpeg "spiralimage_sample" interpolate 2 } },
      SpImg_Size_uv (Radius, 4/3), -135, -1, 0.1, 0.9, 0.4, SPIMG_SRGB,
      SPIMG_UNION, 5
    )
    pigment { red 0.2 }
  }

  box
  { 0, 1
    pigment
    { gradient y color_map
      { [0 rgb <0.3, 0.55, 0.8>]
        [1 rgb <0.7, 0.85, 1.0>]
      }
    }
    translate <-0.5, -0.5, 1>
    scale <Size, Size * image_height / image_width, 1>
  }
#end
// end of spiralimage.pov
