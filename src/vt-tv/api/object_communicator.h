#if !defined INCLUDED_VT_TV_API_OBJECT_COMMUNICATOR_H
#define INCLUDED_VT_TV_API_OBJECT_COMMUNICATOR_H

#include <string>
#include <iostream>
#include <list>
#include <map>
#include <unordered_set>
#include <vector>
#include <utility>

#include "vt-tv/api/types.h"
#include <fmt-vt/format.h>

namespace vt::tv {

struct Object;

/**
 *  A class holding received and sent messages for an object.
 */
struct ObjectCommunicator
{
private:
  ElementIDType object_id_;
  std::map<ElementIDType, double> received_;
  std::map<ElementIDType, double> sent_;

  /**
   * Summarize one-way communicator properties and check for errors.
   */
  std::vector<double> summarize_unidirectional(std::string direction) const {
    // Initialize list of volumes
    std::vector<double> volumes;

    // Iterate over one-way communications
    std::map<ElementIDType, double> communications;
    if(direction == "to") {
      communications = this->sent_;
    } else {
      communications = this->received_;
    }

    // for (const auto& [key, value] : communications) {
    //   sanity check
    //   if(key->get_id() == this->object_id_) {
    //     throw nb::index_error(
    //       "object " + this->object_id_ +
    //       " cannot send communication to itself.");
    //   }
    //   volumes.push_back(value);
    // }

    return volumes;
  }
public:
  /**
   * \brief get id of object for this communicator
   *
   * \return id
   */
  ElementIDType get_object_id() { return object_id_; };

  /**
   * Return all from_object=volume pairs received by object.
   */
  std::map<ElementIDType, double>& get_received() { return this->received_; };

  /**
   * Return the volume of a message received from an object if any.
   */
  double get_received_from_object(ElementIDType id) const {
    return this->received_.at(id);
  }

  /**
   * Return all to_object=volume pairs sent from object.
   */
  std::map<ElementIDType, double>& get_sent() { return this->sent_; };

  /**
   * Return the volume of a message sent to an object if any.
   */
  double get_sent_to_object(ElementIDType id) const {
    return this->sent_.at(id);
  }

  /**
   * Summarize communicator properties and check for errors.
   */
  std::pair<std::vector<double>, std::vector<double>> summarize() const {
    // Summarize sent communications
    std::vector<double> w_sent = this->summarize_unidirectional("to");

    // Summarize received communications
    std::vector<double> w_recv = this->summarize_unidirectional("from");

    return std::make_pair(w_sent, w_recv);
  }

  void addReceived(ElementIDType from_id, double bytes) {
    this->received_.insert(std::make_pair(from_id, bytes));
    if (from_id == this->object_id_) fmt::print("Object {} receiving communication from myself\n",this->object_id_);
  }

  void addSent(ElementIDType to_id, double bytes) {
    this->sent_.insert(std::make_pair(to_id, bytes));
    if (to_id == this->object_id_) fmt::print("Object {} sending communication to myself\n",this->object_id_);
  }

  ObjectCommunicator(ElementIDType id_in)
  : object_id_(id_in)
  , received_()
  , sent_()
  {}
  ObjectCommunicator(ElementIDType id_in,
                      std::map<ElementIDType, double> recv_in,
                      std::map<ElementIDType, double> sent_in)
  : object_id_(id_in)
  , received_(recv_in)
  , sent_(sent_in)
  {}
  ~ObjectCommunicator() = default;
};

} /* end namesapce vt::tv */

#endif /*INCLUDED_VT_TV_API_OBJECT_COMMUNICATOR_H*/
