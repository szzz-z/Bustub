//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// hash_join_executor.h
//
// Identification: src/include/execution/executors/hash_join_executor.h
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <utility>
#include "common/util/hash_util.h"
#include "execution/executor_context.h"
#include "execution/executors/abstract_executor.h"
#include "execution/plans/hash_join_plan.h"
#include "storage/table/tuple.h"

namespace bustub {
struct HashJoinKey {
  std::vector<Value> join_keys_;
  auto operator==(const HashJoinKey &other) const -> bool {
    auto size = other.join_keys_.size();
    if (join_keys_.size() != size) {
      return false;
    }
    for (size_t i = 0; i < size; ++i) {
      if (join_keys_[i].CompareEquals(other.join_keys_[i]) != CmpBool::CmpTrue) {
        return false;
      }
    }
    return true;
  }
};

struct HashJoinKeyHash {
  auto operator()(const HashJoinKey &join_key) const -> std::size_t {
    size_t curr_hash = 0;
    for (const auto &key : join_key.join_keys_) {
      if (!key.IsNull()) {
        curr_hash = HashUtil::CombineHashes(curr_hash, HashUtil::HashValue(&key));
      }
    }
    return curr_hash;
  }
};
struct HashJoinKeyEqual {
  auto operator()(const HashJoinKey &lhs, const HashJoinKey &rhs) const -> bool { return lhs == rhs; }
};

/**
 * HashJoinExecutor executes a nested-loop JOIN on two tables.
 */
class HashJoinExecutor : public AbstractExecutor {
 public:
  /**
   * Construct a new HashJoinExecutor instance.
   * @param exec_ctx The executor context
   * @param plan The HashJoin join plan to be executed
   * @param left_child The child executor that produces tuples for the left side of join
   * @param right_child The child executor that produces tuples for the right side of join
   */
  HashJoinExecutor(ExecutorContext *exec_ctx, const HashJoinPlanNode *plan,
                   std::unique_ptr<AbstractExecutor> &&left_child, std::unique_ptr<AbstractExecutor> &&right_child);

  /** Initialize the join */
  void Init() override;

  /**
   * Yield the next tuple from the join.
   * @param[out] tuple The next tuple produced by the join.
   * @param[out] rid The next tuple RID, not used by hash join.
   * @return `true` if a tuple was produced, `false` if there are no more tuples.
   */
  auto Next(Tuple *tuple, RID *rid) -> bool override;

  /** @return The output schema for the join */
  auto GetOutputSchema() const -> const Schema & override { return plan_->OutputSchema(); };

 private:
  /** The NestedLoopJoin plan node to be executed. */
  const HashJoinPlanNode *plan_;
  std::unique_ptr<AbstractExecutor> left_executor_;
  std::unique_ptr<AbstractExecutor> right_executor_;
  std::unordered_map<HashJoinKey, std::vector<Tuple>, HashJoinKeyHash, HashJoinKeyEqual> ht_;
  std::vector<Tuple> result_;
  std::vector<Tuple>::iterator iter_;
};
}  // namespace bustub
