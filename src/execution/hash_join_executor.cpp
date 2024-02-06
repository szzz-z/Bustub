//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// hash_join_executor.cpp
//
// Identification: src/execution/hash_join_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/hash_join_executor.h"
#include <utility>
#include "type/value.h"
#include "type/value_factory.h"

namespace bustub {

HashJoinExecutor::HashJoinExecutor(ExecutorContext *exec_ctx, const HashJoinPlanNode *plan,
                                   std::unique_ptr<AbstractExecutor> &&left_child,
                                   std::unique_ptr<AbstractExecutor> &&right_child)
    : AbstractExecutor(exec_ctx),
      plan_(plan),
      left_executor_(std::move(left_child)),
      right_executor_(std::move(right_child)) {
  if (plan->GetJoinType() != JoinType::LEFT && plan->GetJoinType() != JoinType::INNER) {
    // Note for 2023 Spring: You ONLY need to implement left join and inner join.
    throw bustub::NotImplementedException(fmt::format("join type {} not supported", plan->GetJoinType()));
  }
}

void HashJoinExecutor::Init() {
  auto l_schema = plan_->GetLeftPlan()->OutputSchema();
  auto r_schema = plan_->GetRightPlan()->OutputSchema();
  auto l_expression = plan_->LeftJoinKeyExpressions();
  auto r_expression = plan_->RightJoinKeyExpressions();
  left_executor_->Init();
  right_executor_->Init();
  RID id;
  Tuple t;
  Tuple left_tp;
  while (right_executor_->Next(&t, &id)) {
    std::vector<Value> join_keys;
    join_keys.reserve(r_expression.size());
    for (const auto &expr : r_expression) {
      join_keys.emplace_back(expr->Evaluate(&t, r_schema));
    }
    ht_[{join_keys}].emplace_back(t);
  }
  while (left_executor_->Next(&left_tp, &id)) {
    bool success{false};
    std::vector<Value> left_keys;
    left_keys.reserve(l_expression.size());
    for (const auto &expr : l_expression) {
      left_keys.emplace_back(expr->Evaluate(&left_tp, l_schema));
    }
    HashJoinKey left_key{left_keys};
    if (ht_.count(left_key) != 0U) {
      for (auto &right_tp : ht_[left_key]) {
        std::vector<Value> right_keys;
        right_keys.reserve(r_expression.size());
        for (const auto &expr : r_expression) {
          right_keys.emplace_back(expr->Evaluate(&right_tp, r_schema));
        }
        HashJoinKey right_key{right_keys};
        if (left_key == right_key) {
          success = true;
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
    }
    if (plan_->GetJoinType() == JoinType::LEFT && !success) {
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
  it_ = result_.begin();
}

auto HashJoinExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  if (it_ != result_.end()) {
    *tuple = *it_++;
    return true;
  }
  return false;
}
}  // namespace bustub
