// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_PERFORMANCE_MANAGER_EXECUTION_CONTEXT_PRIORITY_INHERIT_PARENT_PRIORITY_VOTER_H_
#define COMPONENTS_PERFORMANCE_MANAGER_EXECUTION_CONTEXT_PRIORITY_INHERIT_PARENT_PRIORITY_VOTER_H_

#include "components/performance_manager/public/execution_context_priority/execution_context_priority.h"
#include "components/performance_manager/public/execution_context_priority/priority_voting_system.h"
#include "components/performance_manager/public/graph/frame_node.h"

namespace performance_manager::execution_context_priority {

// This class is used to ensure the priority of a child frame when its parent's
// priority is higher.
//
// This is needed to correctly support the use case of using a non-visible
// cross-origin frame to sandbox some of the work that a web application wants
// to do (See https://crbug.com/336161235 for example).
//
// Ad frames do not inherit the priority of their parent as it is not necessary.
//
// Note: This FrameNodeObserver can affect the initial priority of a frame and
// thus uses `OnBeforeFrameNodeAdded`.
class InheritParentPriorityVoter : public PriorityVoter,
                                   public FrameNodeObserver {
 public:
  static const char kPriorityInheritedReason[];

  InheritParentPriorityVoter();
  ~InheritParentPriorityVoter() override;

  InheritParentPriorityVoter(const InheritParentPriorityVoter&) = delete;
  InheritParentPriorityVoter& operator=(const InheritParentPriorityVoter&) =
      delete;

  // PriorityVoter:
  void InitializeOnGraph(Graph* graph, VotingChannel voting_channel) override;
  void TearDownOnGraph(Graph* graph) override;

  // FrameNodeObserver:
  void OnBeforeFrameNodeAdded(
      const FrameNode* frame_node,
      const FrameNode* pending_parent_frame_node,
      const PageNode* pending_page_node,
      const ProcessNode* pending_process_node,
      const FrameNode* pending_parent_or_outer_document_or_embedder) override;
  void OnBeforeFrameNodeRemoved(const FrameNode* frame_node) override;
  void OnIsAdFrameChanged(const FrameNode* frame_node) override;
  void OnPriorityAndReasonChanged(
      const FrameNode* frame_node,
      const PriorityAndReason& previous_value) override;

  VoterId voter_id() const { return voting_channel_.voter_id(); }

 private:
  OptionalVotingChannel voting_channel_;
};

}  // namespace performance_manager::execution_context_priority

#endif  // COMPONENTS_PERFORMANCE_MANAGER_EXECUTION_CONTEXT_PRIORITY_INHERIT_PARENT_PRIORITY_VOTER_H_
