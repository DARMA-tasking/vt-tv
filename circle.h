#if !defined INCLUDED_VT_TV_RENDER_CIRCLE_H
#define INCLUDED_VT_TV_RENDER_CIRCLE_H

// #include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkProperty.h>
#include <vtkCamera.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkGlyphSource2D.h>
#include <vtkGlyph2D.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkColorTransferFunction.h>
// #include <vtkScalarBarActor.h>
#include <vtkTextProperty.h>
#include <vtkArrayCalculator.h>
#include <vtkThresholdPoints.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>
#include <vtkRegularPolygonSource.h>

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

#include <string>
#include <cstdio>

namespace nb = nanobind;

struct Circle {
    std::string color;

    std::string what_color() const { return "Circle color: " + color; }

    void render();
};

#endif /*INCLUDED_VT_TV_RENDER_CIRCLE_H*/
