/* spiralimage.inc
 * Persistence of Vision Raytracer include file
 *
 * Archimedean spiral impression of a pigment.
 * Inspired by Claude Mellan’s _The Sudarium of Saint Veronica_ (1649).
 *
 * Prerequisite: SphereSweep from the POV-Ray Object Collection
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
 *       2022-Feb-02  Started.
 * 1.0   2022-Feb-03  Completed.
 */
#ifndef (SpiralImage_Inc_Temp) #declare SpiralImage_Inc_Temp = version;
#version max (3.5, min (3.8, version));

#include "arrays.inc"
#include "functions.inc"
#include "spheresweep.inc"

//=========================== PUBLIC MAGIC NUMBERS =============================

#declare SPIMG_SPHERE_SWEEP = 0;
#declare SPIMG_APPROX = 1;
#declare SPIMG_UNION = 2;
#declare SPIMG_MERGE = 3;
#declare SPIMG_SRGB = rgb <0.2126, 0.7152, 0.0722>;
#declare SPIMG_GRAY = rgb <0.297, 0.589, 0.114>;

//============================= PRIVATE FEATURES ===============================

#declare SpImg__fn_Field = function (x) { 2 * x + 1 }
#declare SpImg__fn_Move = function (x) { degrees (min (1 / x, pi / 6)) }

//=============================== PUBLIC MACRO =================================

/* Returns an Archimedean spiral object, with 1 POV unit between spirals.
 * Arguments:
 *  p_Image   - the pigment to be interpreted.  Square <0, 0, 0>...<1, 1, 1>
 *              will be sampled, so an image_map pigment can be used as is.
 *  uv_Size   - the size of final image in POV units
 *  Stop      - the position angle in degrees of the outer end of the spiral.
 *              0 is up, 90 is right.
 *  Direction - the direction of the spiral: 1 spirals outward clockwise;
 *              -1 spirals outward counterclockwise
 *  Thinnest  - the line thickness for the darkest points
 *  Thickest  - the line thickness for the lightest points
 *  Frame     - the line thickness outside the unit square; or a negative number
 *              to use the pigment.
 *  c_Gray    - the gray scale function: SPIMG_SRGB or SPIMG_GRAY
 *  Method    - the type of spiral object:
 *                SPIMG_SPHERE_SWEEP: a cubic sphere_sweep
 *                SPIMG_APPROX: a linear sphere_sweep with many short segments
 *                SPIMG_UNION: a union of spheres and cylinders
 *                SPIMG_MERGE: a merge of spheres and cylinders
 *  Quality   - if Method is SPIMG_SPHERE_SWEEP, the sphere_sweep tolerance;
 *              otherwise, the number of line segments between sampled points
 */
#macro SpiralImage
( p_Image, uv_Size, Stop, Direction, Thinnest, Thickest, Frame, c_Gray,
  Method, Quality
)
 // Parameter checks and preparations:
  #local spimg_This = "SpiralImage"
  #local spimg_Size = uv_Size + <0,0>;
  #if (spimg_Size.x < 1 | spimg_Size.y < 1)
    #error concat
    ( spimg_This, "(): uv_Size was set to <", vstr (2, spimg_Size, ", ", 0, 4),
      ">. It must be <1, 1> or larger."
    )
  #end
  #local spimg_P = pigment
  { p_Image
    translate <-0.5, -0.5, 0>
    scale <spimg_Size.x , spimg_Size.y, 1>
  }
  #local spimg_fn_P = function { pigment { spimg_P } }
  #local spimg_R = sqrt (pow (spimg_Size.x, 2) + pow (spimg_Size.y, 2)) / 2;
  #if (Direction = 0)
    #error concat
    ( spimg_This,
      "(): Direction must be 1 (clockwise) or -1 (counterclockwise)."
    )
  #end
  #local spimg_Way = select (Direction, -1, 1);
 // Tally the sampling points:
  #local spimg_Pts = array [ceil (pow (SpImg__fn_Field (spimg_R), 2)) + 5]
  #local spimg_dMove = SpImg__fn_Move (spimg_R);
  #local spimg_v_Norm = vrotate (y, (-spimg_dMove * spimg_Way - Stop) * z);
  #local spimg_R = spimg_R + spimg_dMove / 360;
  #local spimg_PrePt = true; // Is it the 1st control point?
  #local spimg_Pts[0] = spimg_v_Norm * spimg_R;
  #local spimg_Count = 1;
  #while (spimg_R > 0.25) // if < 0.25, it leaves a dark spot
    #if (spimg_PrePt)
      #local spimg_PrePt = false;
    #else
      #local spimg_dMove = SpImg__fn_Move (spimg_R);
    #end
    #local spimg_v_Norm = vrotate (spimg_v_Norm, spimg_dMove * spimg_Way * z);
    #local spimg_R = max (spimg_R - spimg_dMove / 360, 0);
    #local spimg_Pts [spimg_Count] = spimg_v_Norm * spimg_R;
    #local spimg_Count = spimg_Count + 1;
  #end
 // Sample the pigment:
  Resize_Array (spimg_Pts, spimg_Count)
  #local spimg_Wts = array [spimg_Count]
  #local spimg_I = 0;
  #while (spimg_I < spimg_Count)
    #if
    ( abs (spimg_Pts [spimg_I].x) <= spimg_Size.x / 2 &
      abs (spimg_Pts [spimg_I].y) <= spimg_Size.y / 2 |
      Frame < 0
    )
      #local spimg_C = eval_pigment (spimg_P, spimg_Pts [spimg_I]) * c_Gray;
      #local spimg_Wts [spimg_I] =
      (  (1 - spimg_C.red - spimg_C.green - spimg_C.blue)
       * (Thickest - Thinnest)
       + Thinnest
      ) / 2;
    #else
      #local spimg_Wts [spimg_I] = Frame / 2;
    #end
    #local spimg_I = spimg_I + 1;
  #end
 // Create the spiral:
  #switch (Method)
    #case (SPIMG_SPHERE_SWEEP)
      SphereSweep_Native (SSWP_CUBIC_SPLINE, spimg_Pts, spimg_Wts, Quality)
      #break
    #case (SPIMG_APPROX)
      SphereSweep_Approx (SSWP_CUBIC_SPLINE, spimg_Pts, spimg_Wts, Quality, 0)
      #break
    #case (SPIMG_UNION)
      SphereSweep_Union (SSWP_CUBIC_SPLINE, spimg_Pts, spimg_Wts, Quality)
      #break
    #case (SPIMG_MERGE)
      SphereSweep_Merge (SSWP_CUBIC_SPLINE, spimg_Pts, spimg_Wts, Quality)
      #break
    #else
      #error
        concat (spimg_This, "(): unknown Method ", str (Method, 0, 4), ".")
  #end
#end

#version SpiralImage_Inc_Temp;

#end
// end of spiralimage.inc
