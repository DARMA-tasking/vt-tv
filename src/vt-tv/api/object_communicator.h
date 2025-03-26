/*
//@HEADER
// *****************************************************************************
//
//                            object_communicator.h
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
#if !defined INCLUDED_VT_TV_API_OBJECT_COMMUNICATOR_H
#define INCLUDED_VT_TV_API_OBJECT_COMMUNICATOR_H

#include <algorithm>
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
  ObjectCommunicator() = default;

  /**
   * \brief Construct an \c ObjectCommunicator without any edges
   *
   * \param[in] id_in the object id
   */
  explicit ObjectCommunicator(ElementIDType id_in) : object_id_(id_in) { }

  /**
   * \brief Construct an \c ObjectCommunicator with edges
   *
   * \param[in] id_in the object id
   * \param[in] recv_in receive edges
   * \param[in] sent_in send edges
   */
  ObjectCommunicator(
    ElementIDType id_in,
    std::multimap<ElementIDType, double> recv_in,
    std::multimap<ElementIDType, double> sent_in)
    : object_id_(id_in),
      received_(recv_in),
      sent_(sent_in) { }

  /**
   * \brief Get the id of object for this communicator
   *
   * \return id
   */
  ElementIDType getObjectId() const { return object_id_; };

  /**
   * \brief Return all from_object=volume pairs received by object.
   */
  std::multimap<ElementIDType, double> getReceived() const {
    return this->received_;
  };

  /**
   * \brief Return all volumes of messages received from an object if any.
   */
  std::vector<double> getReceivedFromObject(ElementIDType id) const {
    auto range = this->received_.equal_range(id);

    std::vector<double> results;
    for (auto it = range.first; it != range.second; ++it) {
      results.push_back(it->second);
    }

    return results;
  }

  /**
   * \brief Return all to_object=volume pairs sent from object.
   */
  std::multimap<ElementIDType, double> getSent() const { return this->sent_; };

  /**
   * \brief Return all volumes of messages sent to an object if any.
   */
  std::vector<double> getSentToObject(ElementIDType id) const {
    auto range = this->sent_.equal_range(id);

    std::vector<double> results;
    for (auto it = range.first; it != range.second; ++it) {
      results.push_back(it->second);
    }

    return results;
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
        "Object {} receiving communication from myself\n", this->object_id_);
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
        "Object {} sending communication to myself\n", this->object_id_);
    }
  }

  /**
   * \brief maximum bytes received or sent at this communicator
   */
  double getMaxVolume() const {
    // Search for the maximum value in received and sent (0. if sets are empty)
    double max_recv = !this->received_.empty() ?
      std::max_element(
        this->received_.begin(),
        this->received_.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; })
        ->second :
      0.0;

    double max_sent = !this->sent_.empty() ?
      std::max_element(
        this->sent_.begin(),
        this->sent_.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; })
        ->second :
      0.0;

    // Return the max
    return std::max(max_recv, max_sent);
  }

  /**
   * \brief Get the total received communication volume for this communicator
   *
   * \return total received communication volume
   */
  double getTotalReceivedVolume() const {
    double total = 0.0;
    for (const auto& [key, value] : this->received_) {
      total += value;
    }
    return total;
  }

  /**
   * \brief Get the total sent communication volume for this communicator
   *
   * \return total sent communication volume
   */
  double getTotalSentVolume() const {
    double total = 0.0;
    for (const auto& [key, value] : this->sent_) {
      total += value;
    }
    return total;
  }

  /**
   * \brief Serializer for data
   *
   * \param[in] s the serializer
   */
  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | object_id_;
    s | received_;
    s | sent_;
  }

private:
  ElementIDType object_id_;                       /**< The object id */
  std::multimap<ElementIDType, double> received_; /**< The received edges */
  std::multimap<ElementIDType, double> sent_;     /**< The sent edges */
};

} /* end namespace vt::tv */

#endif /*INCLUDED_VT_TV_API_OBJECT_COMMUNICATOR_H*/
