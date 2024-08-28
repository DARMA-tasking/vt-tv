/*
//@HEADER
// *****************************************************************************
//
//                                 test_vtk.cc
//             DARMA/vt-tv => Virtual Transport -- Task Visualizer
//
// Copyright 2019-2024 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact darma@sandia.gov
//
// *****************************************************************************
//@HEADER
*/

#include <vtkActor.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPNGWriter.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkVersion.h>
#include <vtkWindowToImageFilter.h>

#include "../util.h"

namespace vt::tv::tests::unit::deps {

/**
 * Provides unit tests for the Vtk functions called from vt-tv
 */
class VtkTest : public ::testing::Test {
  void SetUp() override {
    // This test is not testing vt-tv src.
    // That's why it is skipped. But it might be useful locally.
    GTEST_SKIP() << "Skipping VTK basic tests";
    return;

    // Make the output directory for these tests
    std::filesystem::create_directory(fmt::format("{}/output", SRC_DIR));
    std::filesystem::create_directory(fmt::format("{}/output/tests", SRC_DIR));
  }
};

/**
 * Test a VTK example (adapted for tests)
 */
TEST_F(VtkTest, test_vtk_screenshot_example) {
  vtkNew<vtkNamedColors> colors;

  vtkNew<vtkSphereSource> sphere_source;
  sphere_source->SetCenter(0.0, 0.0, 0.0);
  sphere_source->SetRadius(5.0);
  sphere_source->SetPhiResolution(30);
  sphere_source->SetThetaResolution(30);
  sphere_source->Update();

  // Visualize
  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(sphere_source->GetOutputPort());

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  actor->GetProperty()->SetColor(colors->GetColor3d("IndianRed").GetData());

  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> render_window;
  render_window->AddRenderer(renderer);
  render_window->SetAlphaBitPlanes(1); // enable usage of alpha channel
  render_window->SetWindowName("Screenshot");

  actor->GetProperty()->LightingOff();
  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("MistyRose").GetData());

  render_window->Render();

  // Screenshot
  vtkNew<vtkWindowToImageFilter> w2i;
  w2i->SetInput(render_window);
#if VTK_MAJOR_VERSION >= 8 || VTK_MAJOR_VERSION == 8 && VTK_MINOR_VERSION >= 90
  w2i->SetScale(2); // image quality
#else
  w2i->SetMagnification(2); // image quality
#endif
  w2i->SetInputBufferTypeToRGBA(); // also record the alpha
                                                   // (transparency) channel
  w2i->ReadFrontBufferOff();       // read from the back buffer
  w2i->Update();

  vtkNew<vtkPNGWriter> writer;
  writer->SetFileName(
    fmt::format("{}/output/tests/vtk_example_screenshot.png", SRC_DIR).c_str());
  writer->SetInputConnection(windowToImageFilter->GetOutputPort());
}

} // namespace vt::tv::tests::unit::deps