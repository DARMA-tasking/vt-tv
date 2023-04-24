#include "visualizer.h"

Visualizer::Visualizer(
    std::list<std::string> qoi_request,
    bool continuous_object_qoi,
    std::list<Phase> phases,
    std::list<std::string> grid_size,
    double object_jitter,
    std::string output_dir,
    std::string output_file_stem,
    std::map<std::string, std::string> distributions,
    std::map<std::string, std::string> statistics,
    double resolution
)
: qoi_request(qoi_request)
, continuous_object_qoi(continuous_object_qoi)
, phases(phases)
, grid_size(grid_size)
, object_jitter(object_jitter)
, output_dir(output_dir)
, output_file_stem(output_file_stem)
, distributions(distributions)
, statistics(statistics)
, resolution(resolution)
{}


NB_MODULE(visualizer, m) {
     nb::class_<Phase>(m, "phase")
        .def(nb::init<>());
    nb::class_<Visualizer>(m, "Visualizer")
        .def(nb::init<std::list<std::string>,
        bool,
        std::list<Phase>,
        std::list<std::string>,
        double,
        std::string,
        std::string,
        std::map<std::string, std::string>,
        std::map<std::string, std::string>,
        double>());
}
