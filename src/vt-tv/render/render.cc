/*
//@HEADER
// *****************************************************************************
//
//                                 render.cc
//             DARMA/vt-tv => Virtual Transport -- Task Visualizer
//
// Copyright 2019 National Technology & Engineering Solutions of Sandia, LLC
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

#include "vt-tv/render/render.h"

// #include <nanobind/nanobind.h>

// namespace nb = nanobind;

// using namespace nb::literals;

namespace vt { namespace tv {

Render::Render(std::unordered_map<PhaseType, PhaseWork> in_phase_info)
: phase_info_(std::move(in_phase_info))
{ };

void Render::compute_object_load_range() {
  // Initialize space-time object QOI range attributes
  TimeType oq_max = -1 * std::numeric_limits<double>::infinity();
  TimeType ol;

  // Iterate over all phases
  for (auto const& [_, phase_work] : this->phase_info_) {
    // Iterate over all objects in phase
    for (auto const& [elm_id, work] : phase_work.getObjectWork()) {
      // Update maximum object load
      ol = work.getLoad();
      if (ol > oq_max) {
        oq_max = ol;
      }
    }
  }

  // Update extrema attribute
  this->object_load_max_ = oq_max;
}

/*static*/ vtkNew<vtkColorTransferFunction> Render::createColorTransferFunction(
  double range[2], double avg_load, ColorType ct
) {
  vtkNew<vtkColorTransferFunction> ctf;
  ctf->SetNanColorRGBA(1., 1., 1., 0.);
  ctf->UseBelowRangeColorOn();
  ctf->UseAboveRangeColorOn();

  switch (ct) {
  case BlueToRed: {
    ctf->SetColorSpaceToDiverging();
    double const mid_point = (range[0] + range[1]) * .5;
    ctf->AddRGBPoint(range[0], .231, .298, .753);
    ctf->AddRGBPoint(mid_point, .865, .865, .865);
    ctf->AddRGBPoint(range[1], .906, .016, .109);
    ctf->SetBelowRangeColor(0.0, 1.0, 0.0);
    ctf->SetAboveRangeColor(1.0, 0.0, 1.0);
    break;
  }
  case HotSpot: {
    ctf->SetColorSpaceToDiverging();
    double const mid_point = avg_load;
    ctf->AddRGBPoint(range[0], .231, .298, .753);
    ctf->AddRGBPoint(mid_point, .865, .865, .865);
    ctf->AddRGBPoint(range[1], .906, .016, .109);
    ctf->SetBelowRangeColor(0.0, 1.0, 1.0);
    ctf->SetAboveRangeColor(1.0, 1.0, 0.0);
    break;
  }
  case WhiteToBlack: {
    ctf->AddRGBPoint(range[0], 1.0, 1.0, 1.0);
    ctf->AddRGBPoint(range[1], 0.0, 0.0, 0.0);
    ctf->SetBelowRangeColor(0.0, 0.0, 1.0);
    ctf->SetAboveRangeColor(1.0, 0.0, 0.0);
    break;
  }
  case Default: {
    double const mid_point = (range[0] + range[1]) * .5;
    ctf->AddRGBPoint(range[0], .431, .761, .161);
    ctf->AddRGBPoint(mid_point, .98, .992, .059);
    ctf->AddRGBPoint(range[1], 1.0, .647, 0.0);
    ctf->SetBelowRangeColor(0.8, 0.8, .8);
    ctf->SetAboveRangeColor(1.0, 0.0, 1.0);
    break;
  }
  }

  return ctf;
}

/*static*/ vtkNew<vtkScalarBarActor> Render::createScalarBarActor(
  vtkPolyDataMapper* mapper, std::string title, double x, double y
) {
  vtkNew<vtkScalarBarActor> scalar_bar_actor;
  scalar_bar_actor->SetLookupTable(mapper->GetLookupTable());

  scalar_bar_actor->SetOrientationToHorizontal();
  scalar_bar_actor->UnconstrainedFontSizeOn();
  scalar_bar_actor->SetNumberOfLabels(2);
  scalar_bar_actor->SetHeight(0.08);
  scalar_bar_actor->SetWidth(0.4);
  scalar_bar_actor->SetLabelFormat("%.2G");
  scalar_bar_actor->SetBarRatio(0.3);
  scalar_bar_actor->DrawTickLabelsOn();

  std::vector<vtkTextProperty*> props;
  props.push_back(scalar_bar_actor->GetTitleTextProperty());
  props.push_back(scalar_bar_actor->GetLabelTextProperty());
  props.push_back(scalar_bar_actor->GetAnnotationTextProperty());

  for (auto&& p : props) {
    p->SetColor(0.0, 0.0, 0.0);
    p->ItalicOff();
    p->BoldOff();
    p->SetFontFamilyToArial();
    p->SetFontSize(72);
  }

  scalar_bar_actor->SetTitle(title.c_str());
  auto position = scalar_bar_actor->GetPositionCoordinate();
  position->SetCoordinateSystemToNormalizedViewport();
  position->SetValue(x, y, 0.0);

  return scalar_bar_actor;
}

/* static */ std::tuple<uint64_t, uint64_t, uint64_t> Render::global_id_to_cartesian(
    uint64_t flat_id, std::tuple<uint64_t, uint64_t, uint64_t> grid_sizes
) {
  // Sanity check
  uint64_t n01 = std::get<0>(grid_sizes) * std::get<1>(grid_sizes);
  if (flat_id < 0 || flat_id >= n01 * std::get<2>(grid_sizes)) {
    return {NULL, NULL, NULL};
  }

  // Compute successive Euclidean divisions
  uint64_t quot1 = flat_id / n01;
  uint64_t rem1 = flat_id % n01;
  uint64_t quot2 = rem1 / std::get<0>(grid_sizes);
  uint64_t rem2 = rem1 % std::get<0>(grid_sizes);
  return {rem2, quot2, quot1};
}

/*static*/ void Render::createPipeline(
  vtkPoints* rank_points,
  vtkCellArray* rank_lines,
  vtkDoubleArray* qois,
  double qoi_range[2],
  vtkPolyData* object_mesh,
  double glyph_factor,
  double load_range[2],
  int phase,
  int iteration,
  double imbalance,
  int win_size
) {
  vtkNew<vtkPolyData> rank_mesh;
  rank_mesh->SetPoints(rank_points);
  rank_mesh->SetLines(rank_lines);
  rank_mesh->GetPointData()->SetScalars(qois);

  vtkNew<vtkRenderer> renderer;
  renderer->SetBackground(1.0, 1.0, 1.0);
  renderer->GetActiveCamera()->ParallelProjectionOn();

  vtkNew<vtkGlyphSource2D> rank_glyph;
  rank_glyph->SetGlyphTypeToSquare();
  rank_glyph->SetScale(.95);
  rank_glyph->FilledOn();
  rank_glyph->CrossOff();
  vtkNew<vtkGlyph2D> rank_glypher;
  rank_glypher->SetSourceConnection(rank_glyph->GetOutputPort());
  rank_glypher->SetInputData(rank_mesh);
  rank_glypher->SetScaleModeToDataScalingOff();

  //Lower glyphs slightly for visibility
  vtkNew<vtkTransform> z_lower;
  z_lower->Translate(0.0, 0.0, -0.01);
  vtkNew<vtkTransformPolyDataFilter> trans;
  trans->SetTransform(z_lower);
  trans->SetInputConnection(rank_glypher->GetOutputPort());

  vtkNew<vtkPolyDataMapper> rank_mapper;
  rank_mapper->SetInputConnection(trans->GetOutputPort());
  rank_mapper->SetLookupTable(createColorTransferFunction(qoi_range));
  rank_mapper->SetScalarRange(qoi_range);

  vtkNew<vtkActor> rank_actor;
  rank_actor->SetMapper(rank_mapper);
  auto qoi_actor = createScalarBarActor(rank_mapper, "Rank XXX", 0.5, 0.9);
  qoi_actor->DrawBelowRangeSwatchOn();
  qoi_actor->SetBelowRangeAnnotation("<");
  qoi_actor->DrawAboveRangeSwatchOn();
  qoi_actor->SetAboveRangeAnnotation(">");
  renderer->AddActor(rank_actor);
  renderer->AddActor2D(qoi_actor);

  // vtkNew<vtkLookupTable> bw_lut;
  // bw_lut->SetTableRange((0.0, self.__max_object_volume));
  // bw_lut->SetSaturationRange(0, 0);
  // bw_lut->SetHueRange(0, 0);
  // bw_lut->SetValueRange(1, 0);
  // bw_lut->SetNanColor(1.0, 1.0, 1.0, 0.0);
  // bw_lut->Build();

  vtkNew<vtkArrayCalculator> sqrtT;
  sqrtT->SetInputData(object_mesh);
  sqrtT->AddScalarArrayName("Load");
  char const* sqrtT_str = "sqrt(Load)";
  sqrtT->SetFunction(sqrtT_str);
  sqrtT->SetResultArrayName(sqrtT_str);
  sqrtT->Update();
  auto sqrtT_out = sqrtT->GetDataSetOutput();
  sqrtT_out->GetPointData()->SetActiveScalars("Migratable");

  std::vector<std::tuple<double, std::string>> items{{0.0, "Square"}, {1.0, "Circle"}};
  vtkPolyDataMapper* glyph_mapper_out = nullptr;
  for (auto&& [k,v] : items) {
    vtkNew<vtkThresholdPoints> thresh;
    thresh->SetInputData(sqrtT_out);
    thresh->ThresholdBetween(k, k);
    thresh->Update();
    auto thresh_out = thresh->GetOutput();
    if (not thresh_out->GetNumberOfPoints())
      continue;
    thresh_out->GetPointData()->SetActiveScalars(sqrtT_str);

    // Glyph by square root of object loads
    vtkNew<vtkGlyphSource2D> glyph;
    if (v == "Square") {
      glyph->SetGlyphTypeToSquare();
    } else {
      glyph->SetGlyphTypeToCircle();
    }
    glyph->SetResolution(32);
    glyph->SetScale(1.0);
    glyph->FilledOn();
    glyph->CrossOff();
    vtkNew<vtkGlyph3D> glypher;
    glypher->SetSourceConnection(glyph->GetOutputPort());
    glypher->SetInputData(thresh_out);
    glypher->SetScaleModeToScaleByScalar();
    glypher->SetScaleFactor(glyph_factor);
    glypher->Update();
    glypher->GetOutput()->GetPointData()->SetActiveScalars("Load");

    // Raise glyphs slightly for visibility
    vtkNew<vtkTransform> z_raise;
    z_raise->Translate(0.0, 0.0, 0.01);
    vtkNew<vtkTransformPolyDataFilter> trans;
    trans->SetTransform(z_raise);
    trans->SetInputData(glypher->GetOutput());

    // Create mapper and actor for glyphs
    vtkNew<vtkPolyDataMapper> glyph_mapper;
    glyph_mapper_out = glyph_mapper;
    glyph_mapper->SetInputConnection(trans->GetOutputPort());
    glyph_mapper->SetLookupTable(createColorTransferFunction(load_range, 0, BlueToRed));
    glyph_mapper->SetScalarRange(load_range);
    vtkNew<vtkActor> glyph_actor;
    glyph_actor->SetMapper(glyph_mapper);
    renderer->AddActor(glyph_actor);
  }

  if (glyph_mapper_out) {
    auto load_actor = createScalarBarActor(glyph_mapper_out, "Object Load", 0.55, 0.55);
    renderer->AddActor2D(load_actor);
  }

  renderer->ResetCamera();
  vtkNew<vtkRenderWindow> render_window;
  render_window->AddRenderer(renderer);
  render_window->SetWindowName("LBAF");
  render_window->SetSize(win_size, win_size);
  render_window->Render();

  vtkNew<vtkWindowToImageFilter> w2i;
  w2i->SetInput(render_window);
  w2i->SetScale(3);

  vtkNew<vtkPNGWriter> writer;
  writer->SetInputConnection(w2i->GetOutputPort());
  writer->SetFileName("test.png");
  writer->SetCompressionLevel(2);
  writer->Write();
}

void Render::generate() {
  // Create vector of number of objects per phase
  std::vector<uint64_t> n_objects_list;
  for (auto const& [phase, phase_work] : this->phase_info_) {
    n_objects_list.push_back(phase_work.getObjectWork().size());
  }

  // Create vector of vectors of object loads per phase
  std::vector<std::vector<TimeType>> phases_object_loads;
  for (auto const& [phase, phase_work] : this->phase_info_) {
    std::vector<TimeType> object_loads;
    for (auto const& [elm_id, work] : phase_work.getObjectWork()) {
      object_loads.push_back(work.getLoad());
    }
    phases_object_loads.push_back(object_loads);
  }

  // Find min max values of loads for phase 0 for visualization scaling
  auto max_load = *max_element(std::begin(phases_object_loads[0]), std::end(phases_object_loads[0]));
  auto min_load = *min_element(std::begin(phases_object_loads[0]), std::end(phases_object_loads[0]));

  // Assign number of objects as number of actors in the visualization
  uint64_t n_actors = n_objects_list[0];

  // Add objects in visualization as spheres and scale them appropriately
  vtkNew<vtkNamedColors> colors;
  vtkNew<vtkRenderer> renderer;
  std::vector<std::string> color_vector = {"Red", "Green", "Cornsilk"};
  for (uint64_t actor_i = 0; actor_i < n_actors; actor_i++)
  {
    // Create a sphere
    vtkNew<vtkSphereSource> sphereSource;
    sphereSource->SetRadius(pow(phases_object_loads[0][actor_i] / (max_load-min_load), 2) * 0.5);
    const auto [i, j, k] = this->global_id_to_cartesian(actor_i, {5, 5, 1});
    sphereSource->SetCenter(i, j, k);

    // Make the surface smooth.
    sphereSource->SetPhiResolution(100);
    sphereSource->SetThetaResolution(100);

    // Add sphere to mapper
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(sphereSource->GetOutputPort());

    // Add actor to render
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(colors->GetColor3d(color_vector[2]).GetData());
    renderer->AddActor(actor);
  }

  // Set background color
  renderer->SetBackground(colors->GetColor3d("steelblue").GetData());

  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);

  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);

  renderWindow->SetWindowName("Render");
  renderWindow->Render();
  renderWindowInteractor->Start();
}

}} /* end namesapce vt::tv */
