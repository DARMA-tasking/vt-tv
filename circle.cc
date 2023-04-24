#include "circle.h"



int add(int a, int b) { return a + b; }

void Circle::render() {
  vtkNew<vtkNamedColors> colors;

  // Create a circle
  vtkNew<vtkRegularPolygonSource> polygonSource;
  // Comment this line to generate a disk instead of a circle.
  //polygonSource->GeneratePolygonOff();
  polygonSource->SetNumberOfSides(50);
  polygonSource->SetRadius(5);
  polygonSource->SetCenter(0, 0, 0);

  // Visualize
  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(polygonSource->GetOutputPort());

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  actor->GetProperty()->SetColor(colors->GetColor3d(this->color).GetData());

  vtkNew<vtkRenderer> renderer;
  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("Blue").GetData());

  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);

  renderWindow->SetWindowName("Circle");
  renderWindow->Render();

  vtkNew<vtkWindowToImageFilter> w2i;
  w2i->SetInput(renderWindow.Get());
  w2i->SetScale(3);

  vtkNew<vtkPNGWriter> writer;
  writer->SetFileName("test.png");
  writer->SetInputConnection(w2i->GetOutputPort());
  writer->Write();
}

NB_MODULE(my_ext, m) {
    nb::class_<Circle>(m, "Circle")
        .def(nb::init<const std::string &>())
        .def("what_color", &Circle::what_color)
        .def_rw("color", &Circle::color)
        .def("render", &Circle::render);
    m.def("add", &add);
    m.attr("the_answer") = 42;
}
