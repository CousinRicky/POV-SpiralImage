/* spiralimage.inc version 2.0
 * Persistence of Vision Raytracer include file
 * A proposed POV-Ray Object Collection module
 *
 * Archimedean spiral impression of a pigment.
 * Inspired by Claude Mellan’s _The Sudarium of Saint Veronica_ (1649).
 *
 * Prerequisite: SphereSweep from the POV-Ray Object Collection
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
 *       2022-Feb-02  Started.
 * 1.0   2022-Feb-03  Completed.
 *       2022-Feb-11  Macros for calculating spiral dimensions are added.
 *       2022-Feb-13  A macro is added for scaling a pigment.
 *       2022-Feb-13  The Quality argument is changed to Resolution, and has
 *                    the same function across all methods; interpolation is
 *                    eliminated.
 *       2022-Feb-13  The angular step size cap is reduced for the linear
 *                    methods.
 * 2.0   2022-Feb-13  The array size is better estimated from the spiral size
 *                    and resolution; and overflow is dealt with.
 *
 * Implementation note: due to POV-Ray namespace scope leakage, local variables
 * and function parameters (other than x, y, and z) *must* be prefixed.
 */
#ifndef (SpiralImage_Inc_Temp) #declare SpiralImage_Inc_Temp = version;
#version max (3.5, min (3.8, version));

#declare SPIRALIMAGE_VERSION = 2.0;
#ifdef (View_POV_Include_Stack)
  #debug "#include \"spiralimage.inc\" //version 2.0\n"
#end

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

//============================= PRIVATE FEATURE ================================
// This tool is not intended be referenced directly by the user.

// x = radius, y = resolution, z = Boolean: is it cubic?
#declare SpImg__fn_Move = function
{ degrees (min (1 / (x * y), pi / select (z, 6, 18, 6)))
}

//============================== PUBLIC MACROS =================================

#macro SpiralImage
( p_Image, uv_Size, Stop, Direction, Thinnest, Thickest, Frame, c_Gray,
  Method, Resolution
)
 // Argument checks and preparations:
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
  #local spimg_R = SpImg_Radius (uv_Size);
  #if (Direction = 0)
    #error concat
    ( spimg_This,
      "(): Direction must be 1 (clockwise) or -1 (counterclockwise)."
    )
  #end
  #local spimg_Way = select (Direction, -1, 1);
  #local spimg_isCubic = (Method = SPIMG_SPHERE_SWEEP); // cubic spline?
  #if (Resolution <= 0)
    #error concat
    ( spimg_This, "(): Resolution was set to ", str (Resolution, 0, 4),
      ". It must be positive."
    )
  #end
  // This next statement estimates the number of sampled points.  The +100
  // covers the increased resolution as the spiral gets tighter; Resolution 1
  // maxes out at +86 for an impractically small spiral, but an impractically
  // small Resolution can exceed this.
  #local spimg_N = ceil (pow (spimg_R + 0.5, 2) * pi * Resolution) + 100;
  #local spimg_Pts = array [spimg_N]
  // Implementation note: we don't know the number of points ahead of time, so
  // rather than sample the points immediately, which would require maintaining
  // and resizing 2 arrays, I decided to tally and sample in separate stages.
 // Loop 1: tally the sampling points:
  #local spimg_dMove = SpImg__fn_Move (spimg_R, Resolution, spimg_isCubic);
  #local spimg_Norm = vrotate (y, (-spimg_dMove * spimg_Way - Stop) * z);
  #if (spimg_isCubic) // then rotate backwards to add a control point:
    #local spimg_dMove = SpImg__fn_Move (spimg_R, Resolution, spimg_isCubic);
    #local spimg_CtrlPt = yes;
    #local spimg_Norm = vrotate (y, (-spimg_dMove * spimg_Way - Stop) * z);
    #local spimg_R = spimg_R + spimg_dMove / 360;
  #else // start at the stop angle:
    #local spimg_CtrlPt = no;
    #local spimg_Norm = vrotate (y, -Stop * z);
  #end
  #local spimg_Pts[0] = spimg_Norm * spimg_R;
  #local spimg_Count = 1;
  // Terminate the loop at > 0, to avoid the impression of a dark spot.
  // Cubic spline gets a lower termination due to the extra control point.
  #while (spimg_R > (spimg_isCubic? 0.25: 0.33))
    // Is the 1st point a cubic control point?
    #if (spimg_CtrlPt)
      #local spimg_CtrlPt = no;
    #else
      #local spimg_dMove = SpImg__fn_Move (spimg_R, Resolution, spimg_isCubic);
    #end
    // Is the array full?
    #if (spimg_Count >= spimg_N)
      #warning concat
      ( spimg_This, "(, <", vstr (2, spimg_Size, ", ", 0, 4), ">,,,,,,, ",
        vstr (2, <Method, Resolution>, ", ", 0, 4), "): Reallocating ",
        vstr (2, <1, 2> * spimg_N,  " to ", 0, 0), "."
      )
      #local spimg_N = spimg_N * 2;
      Resize_Array (spimg_Pts, spimg_N)
    #end
    // Locate the next sampling point:
    #local spimg_Norm = vrotate (spimg_Norm, spimg_dMove * spimg_Way * z);
    #local spimg_R = max (spimg_R - spimg_dMove / 360, 0);
    #local spimg_Pts [spimg_Count] = spimg_Norm * spimg_R;
    #local spimg_Count = spimg_Count + 1;
  #end
  #if (spimg_Count < spimg_N) Resize_Array (spimg_Pts, spimg_Count) #end
 // Loop 2: sample the pigment:
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
      SphereSweep_Native (SSWP_CUBIC_SPLINE, spimg_Pts, spimg_Wts, 0.0001)
      #break
    #case (SPIMG_APPROX)
      SphereSweep_Approx (SSWP_LINEAR_SPLINE, spimg_Pts, spimg_Wts, 1, 0)
      #break
    #case (SPIMG_UNION)
      SphereSweep_Union (SSWP_LINEAR_SPLINE, spimg_Pts, spimg_Wts, 1)
      #break
    #case (SPIMG_MERGE)
      SphereSweep_Merge (SSWP_LINEAR_SPLINE, spimg_Pts, spimg_Wts, 1)
      #break
    #else
      #error
        concat (spimg_This, "(): unknown Method ", str (Method, 0, 4), ".")
  #end
#end

//--------------------------------------

#macro SpImg_Radius (uv_Size)
  #local spimg_Size = uv_Size + <0, 0>;
  (vlength (<spimg_Size.x, spimg_Size.y, 0>) / 2)
#end

//--------------------------------------

#macro SpImg_Scale_p (Pigment, Scale)
  pigment
  { Pigment
    translate <-0.5, -0.5, 0>
    scale Scale
    translate <0.5, 0.5, 0>
  }
#end

//--------------------------------------

#macro SpImg_Size_uv (Radius, Aspect)
  #local spimg_H = 2 * Radius / sqrt (pow (Aspect, 2) + 1);
  #local spimg_W = Aspect * spimg_H;
  <spimg_W, spimg_H>
#end

//--------------------------------------

#version SpiralImage_Inc_Temp;
#end
// end of spiralimage.inc
