/* spiralimage.pov
 * Persistence of Vision Raytracer scene description file
 *
 * Demo scene using spiralimage.inc: an Archimedean spiral impression of an
 * image map.
 *
 * Copyright © 2022 Richard Callwood III.  Some rights reserved.
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
 */
// +A +R5 +W800 +H600
#version max (3.5, min (3.8, version));

#ifndef (H) #declare H = 75; #end

#include "spiralimage.inc"

global_settings { assumed_gamma 1 }
#default { finish { diffuse 0 ambient #if (version >= 3.7) 0 emission #end 1 } }

#declare Dims = <1, 0.75, 0> * H;
#declare Size = vlength (Dims) + 1;
camera
{ orthographic
  location -2 * z
  right Size * x
  up Size * image_height / image_width * y
}

object
{ SpiralImage
  ( pigment { image_map { jpeg "spiralimage_sample" interpolate 2 } },
    Dims, -135, -1, 0.1, 0.9, 0.1, SPIMG_SRGB, SPIMG_UNION, 5
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
// end of spiralimage.pov
