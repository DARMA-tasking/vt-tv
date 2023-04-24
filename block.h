#if !defined INCLUDED_VT_TV_API_BLOCK_H
#define INCLUDED_VT_TV_API_BLOCK_H

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/list.h>
#include <nanobind/stl/map.h>
#include <nanobind/stl/unordered_set.h>
#include <nanobind/operators.h>

#include <string>
#include <iostream>
#include <list>
#include <map>
#include <format>
#include <unordered_set>

namespace nb = nanobind;

namespace vt { namespace tv {

/**
 *  A class representing a memory block with footprint and home.
 */
struct Block
{
private:
  uint64_t index;
  uint64_t home_id;
  double size = 0;
  std::unordered_set<uint64_t> attached_object_ids;
public:
  uint64_t get_id() const { return this->index; };
  uint64_t get_home_id() const { return this->home_id; };
  double get_size() const { return this->size; };
  std::unordered_set<uint64_t> get_attached_object_ids() const { return this->attached_object_ids; };

  uint64_t detach_object_id(uint64_t);
  void attach_object_id(uint64_t);

  std::string to_string() const;
  
  friend std::ostream & operator << (std::ostream &out, const Block &b);

  Block(uint64_t, uint64_t, double, std::unordered_set<uint64_t>);
  ~Block() = default;
};

std::ostream & operator << (std::ostream &out, const Block &b)
{
  out << "Block id: " << b.get_id() << ", home id: " << b.get_home_id()
  << ", size: " << b.get_size() << ", object ids: ";
  for (const auto &elem : b.get_attached_object_ids()) {
    out << elem << " ";
  }
  out << std::endl;
  return out;
}

}} /* end namesapce vt::tv */

#endif /*INCLUDED_VT_TV_API_BLOCK_H*/
