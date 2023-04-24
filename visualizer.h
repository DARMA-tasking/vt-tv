#if !defined INCLUDED_VT_TV_VISUALIZER_H
#define INCLUDED_VT_TV_VISUALIZER_H

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

#include <Python.h>

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/list.h>
#include <nanobind/stl/map.h>

#include <string>
#include <iostream>
#include <list>
#include <map>

namespace nb = nanobind;

struct Phase {
    uint64_t n_objects = 0;
};

struct Visualizer {
private:
    std::list<std::string> qoi_request;
    bool continuous_object_qoi;
    std::list<Phase> phases;
    std::list<std::string> grid_size;
    double object_jitter = 0;
    std::string output_dir = ".";
    std::string output_file_stem = "LBAF_out";
    std::map<std::string, std::string> distributions;
    std::map<std::string, std::string> statistics;
    double resolution = 1;

public:
    Visualizer(
        std::list<std::string>,
        bool,
        std::list<Phase>,
        std::list<std::string>,
        double,
        std::string,
        std::string,
        std::map<std::string, std::string>,
        std::map<std::string, std::string>,
        double
    );

};

#endif /*INCLUDED_VT_TV_VISUALIZER_H*/
