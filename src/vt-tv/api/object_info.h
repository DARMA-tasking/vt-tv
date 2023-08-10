/*
//@HEADER
// *****************************************************************************
//
//                               object_info.h
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

#if !defined INCLUDED_VT_TV_API_OBJECT_INFO_H
#define INCLUDED_VT_TV_API_OBJECT_INFO_H

#include "vt-tv/api/types.h"

#include <vector>

namespace vt::tv {

/**
 * \struct ObjectInfo
 *
 * \brief Holds basic information about an object that does not change across
 * ranks or phases.
 */
struct ObjectInfo {

  ObjectInfo() = default;

  /**
   * \brief Construct information about an object
   *
   * \param[in] in_id the unique global identifier for an object
   * \param[in] in_home the home rank for the object
   * \param[in] in_migratable whether it's migratable
   * \param[in] in_index the index for the object
   */
  ObjectInfo(
    ElementIDType in_id,
    NodeType in_home,
    bool in_migratable,
    std::vector<UniqueIndexBitType> const& in_index
  ) : id_(in_id),
      home_(in_home),
      migratable_(in_migratable),
      index_(in_index)
  { }

  /**
   * \brief Get the object's ID
   *
   * \return object ID
   */
  ElementIDType getID() const { return id_; }

  /**
   * \brief Get the home rank for this object (the rank responsible for it)
   *
   * \return the home rank
   */
  NodeType getHome() const { return home_; }

  /**
   * \brief Get whether the object is migratable
   *
   * \return whether it is migratable
   */
  bool isMigratable() const { return migratable_; }

  /**
   * \brief Get the logical index for the task (if one exists)
   *
   * \note For singleton objects this may be empty.
   *
   * \return the index array
   */
  auto const& getIndexArray() const { return index_; }

  /**
   * \brief Set whether this object is part of a collection
   *
   * \param[in] is_collection if it is part of a collection
   */
  void setIsCollection(bool is_collection) { is_collection_ = is_collection; }

  /**
   * \brief Set whether this object is part of an object group
   *
   * \param[in] is_objgroup if it is part of an object group
   */
  void setIsObjGroup(bool is_objgroup) { is_objgroup_ = is_objgroup; }

  /**
   * \brief Get whether it's part of a collection
   *
   * \return whether it is
   */
  bool getIsCollection() const { return is_collection_; }

  /**
   * \brief Get whether it's part of a object group
   *
   * \return whether it is
   */
  bool getIsObjGroup() const { return is_objgroup_; }

  /**
   * \brief Set the meta ID. Will only be set if it is part of a collection of
   * object group. If it is part of a collection it will be the \c
   * VirtualProxyType bits from VT. If it is part of a object group, it will be
   * the \c ObjGroupProxyType bits from VT. Both of these IDs uniquely represent
   * the collection or object group for a run
   *
   * \param[in] in_meta_id the id to set
   */
  void setMetaID(CollectionObjGroupIDType in_meta_id) { meta_id_ = in_meta_id; }

  /**
   * \brief Get the meta ID.
   *
   * \return the meta ID
   */
  CollectionObjGroupIDType getMetaID() const { return meta_id_; }

  /**
   * \brief Serializer for data
   *
   * \param[in] s the serializer
   */
  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | id_;
    s | home_;
    s | migratable_;
    s | index_;
    s | meta_id_;
    s | is_objgroup_;
    s | is_collection_;
  }

private:
  /// Unique identifier across all ranks for the object
  ElementIDType id_ = 0;
  /// The rank the element started on
  NodeType home_ = 0;
  /// Whether it is migratable (if not, `home_` == `curr_`)
  bool migratable_ = false;
  /// Optional: index for the element
  std::vector<UniqueIndexBitType> index_;
  /// Optional: the objgroup or collection ID
  CollectionObjGroupIDType meta_id_ = 0;
  /// Whether it's an objgroup
  bool is_objgroup_ = false;
  /// Whether it's an collection
  bool is_collection_ = false;
};

} /* end namesapce vt::tv */

#endif /*INCLUDED_VT_TV_API_OBJECT_INFO_H*/
