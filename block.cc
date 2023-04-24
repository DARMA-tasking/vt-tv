#include "block.h"

namespace vt { namespace tv {

Block::Block(uint64_t b_id,
  uint64_t h_id,
  double size,
  std::unordered_set<uint64_t> o_ids)
: index(b_id)
, home_id(h_id)
, size(size)
, attached_object_ids(o_ids)
{}

uint64_t Block::detach_object_id(uint64_t o_id) {
  try {
    this->attached_object_ids.erase(o_id);
  }
  catch(const nb::type_error e) {
    std::cerr << e.what() << '\n';
  }
  return this->attached_object_ids.size();
}

void Block::attach_object_id(uint64_t o_id) {
  this->attached_object_ids.insert(o_id);
}


std::string Block::to_string() const {
  std::string out;
  out += "Block id: " + std::to_string(this->get_id())
      + ", home id: " + std::to_string(this->get_home_id())
      + ", size: " + std::to_string(this->get_size())
      + ", object ids: {";
  for (const auto &elem : this->get_attached_object_ids()) {
    out += std::to_string(elem) + " ";
  }
  out += "}";
  return out;
}

NB_MODULE(block, m) {
     nb::class_<Block>(m, "Block")
        .def(nb::init<uint64_t, uint64_t, double, std::unordered_set<uint64_t>>())
        .def("get_id", &Block::get_id)
        .def("get_home_id", &Block::get_home_id)
        .def("get_size", &Block::get_size)
        .def("detach_object_id", &Block::detach_object_id)
        .def("attach_object_id", &Block::attach_object_id)
        .def("to_string", &Block::to_string);
}

}} /* end namesapce vt::tv */
