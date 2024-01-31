//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// nested_loop_join_executor.cpp
//
// Identification: src/execution/nested_loop_join_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/nested_loop_join_executor.h"
#include <cstdint>
#include <utility>
#include "binder/table_ref/bound_join_ref.h"
#include "common/exception.h"
#include "type/value_factory.h"

namespace bustub {

NestedLoopJoinExecutor::NestedLoopJoinExecutor(ExecutorContext *exec_ctx, const NestedLoopJoinPlanNode *plan,
                                               std::unique_ptr<AbstractExecutor> &&left_executor,
                                               std::unique_ptr<AbstractExecutor> &&right_executor)
    : AbstractExecutor(exec_ctx),
      plan_(plan),
      left_executor_(std::move(left_executor)),
      right_executor_(std::move(right_executor)) {
  if (plan->GetJoinType() != JoinType::LEFT && plan->GetJoinType() != JoinType::INNER) {
    // Note for 2023 Spring: You ONLY need to implement left join and inner join.
    throw bustub::NotImplementedException(fmt::format("join type {} not supported", plan->GetJoinType()));
  }
}

void NestedLoopJoinExecutor::Init() {
  auto l_schema = plan_->GetLeftPlan()->OutputSchema();
  auto r_schema = plan_->GetRightPlan()->OutputSchema();
  left_executor_->Init();
  right_executor_->Init();
  Tuple left_tp;
  RID id;
  std::vector<Tuple> right_tuples;
  Tuple t;
  while (right_executor_->Next(&t, &id)) {
    right_tuples.emplace_back(t);
  }
  while (left_executor_->Next(&left_tp, &id)) {
    bool matched = false;
    right_executor_->Init();
    for (auto &right_tp : right_tuples) {
      if (plan_->Predicate()->EvaluateJoin(&left_tp, l_schema, &right_tp, r_schema).GetAs<bool>()) {
        matched = true;
        std::vector<Value> val;
        val.reserve((l_schema.GetColumnCount() + r_schema.GetColumnCount()));

        for (uint32_t i = 0; i < l_schema.GetColumnCount(); i++) {
          val.emplace_back(left_tp.GetValue(&l_schema, i));
        }
        for (uint32_t i = 0; i < r_schema.GetColumnCount(); i++) {
          val.emplace_back(right_tp.GetValue(&r_schema, i));
        }
        result_.emplace_back(val, &plan_->OutputSchema());
      }
    }
    if (plan_->GetJoinType() == JoinType::LEFT && !matched) {
      std::vector<Value> val;
      val.reserve((l_schema.GetColumnCount() + r_schema.GetColumnCount()));

      for (uint32_t i = 0; i < l_schema.GetColumnCount(); i++) {
        val.emplace_back(left_tp.GetValue(&l_schema, i));
      }
      for (uint32_t i = 0; i < r_schema.GetColumnCount(); i++) {
        val.emplace_back(ValueFactory::GetNullValueByType(r_schema.GetColumn(i).GetType()));
      }
      result_.emplace_back(val, &plan_->OutputSchema());
    }
  }
  result_it_ = result_.begin();
}

auto NestedLoopJoinExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  if (result_it_ != result_.end()) {
    *tuple = *result_it_;
    ++result_it_;
    return true;
  }
  return false;
}

}  // namespace bustub
