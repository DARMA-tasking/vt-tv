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
 * \struct ObjectCommunicator
 *
 * \brief A class holding received and sent messages for an object.
 */
struct ObjectCommunicator {

  /**
   * \brief Construct an \c ObjectCommunicator without any edges
   *
   * \param[in] id_in the object id
   */
  explicit ObjectCommunicator(ElementIDType id_in)
    : object_id_(id_in)
  {}

  /**
   * \brief Construct an \c ObjectCommunicator with edges
   *
   * \param[in] id_in the object id
   * \param[in] recv_in receive edges
   * \param[in] sent_in send edges
   */
  ObjectCommunicator(
    ElementIDType id_in,
    std::map<ElementIDType, double> recv_in,
    std::map<ElementIDType, double> sent_in
  ) : object_id_(id_in),
      received_(recv_in),
      sent_(sent_in)
  { }

  /**
   * \brief Summarize one-way communicator properties and check for errors.
   *
   * \param[in] direction the direction to summarize edges
   */
  std::vector<double> summarizeUnidirectional(std::string direction) const {
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

  /**
   * \brief Get the id of object for this communicator
   *
   * \return id
   */
  ElementIDType getObjectId() const { return object_id_; };

  /**
   * \brief Return all from_object=volume pairs received by object.
   */
  std::map<ElementIDType, double> getReceived() const {
    return this->received_;
  };

  /**
   * \brief Return the volume of a message received from an object if any.
   */
  double getReceivedFromObject(ElementIDType id) const {
    return this->received_.at(id);
  }

  /**
   * \brief Return all to_object=volume pairs sent from object.
   */
  std::map<ElementIDType, double> getSent() const { return this->sent_; };

  /**
   * \brief Return the volume of a message sent to an object if any.
   */
  double getSentToObject(ElementIDType id) const {
    return this->sent_.at(id);
  }

  /**
   * \brief Summarize communicator properties and check for errors.
   */
  std::pair<std::vector<double>, std::vector<double>> summarize() const {
    // Summarize sent communications
    std::vector<double> w_sent = this->summarizeUnidirectional("to");

    // Summarize received communications
    std::vector<double> w_recv = this->summarizeUnidirectional("from");

    return std::make_pair(w_sent, w_recv);
  }

  /**
   * \brief Add received edge
   *
   * \param[in] from_id the from object id
   * \param[in] bytes the number of bytes
   */
  void addReceived(ElementIDType from_id, double bytes) {
    this->received_.insert(std::make_pair(from_id, bytes));
    if (from_id == this->object_id_) {
      fmt::print(
        "Object {} receiving communication from myself\n", this->object_id_
      );
    }
  }

  /**
   * \brief Add sent edge
   *
   * \param[in] to_id the to object id
   * \param[in] bytes the number of bytes
   */
  void addSent(ElementIDType to_id, double bytes) {
    this->sent_.insert(std::make_pair(to_id, bytes));
    if (to_id == this->object_id_) {
      fmt::print(
        "Object {} sending communication to myself\n", this->object_id_
      );
    }
  }

private:
  ElementIDType object_id_;                  /**< The object id */
  std::map<ElementIDType, double> received_; /**< The received edges */
  std::map<ElementIDType, double> sent_;     /**< The sent edges */
};

} /* end namesapce vt::tv */

#endif /*INCLUDED_VT_TV_API_OBJECT_COMMUNICATOR_H*/
