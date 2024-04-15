//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// nested_index_join_executor.cpp
//
// Identification: src/execution/nested_index_join_executor.cpp
//
// Copyright (c) 2015-19, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/nested_index_join_executor.h"
#include "type/value_factory.h"

namespace bustub {

NestIndexJoinExecutor::NestIndexJoinExecutor(ExecutorContext *exec_ctx, const NestedIndexJoinPlanNode *plan,
                                             std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx),
      plan_(plan),
      child_executor_(std::move(child_executor)),
      index_info_(exec_ctx_->GetCatalog()->GetIndex(plan_->GetIndexOid())),
      table_info_(exec_ctx_->GetCatalog()->GetTable(index_info_->table_name_)),
      it_(dynamic_cast<BPlusTreeIndexForTwoIntegerColumn *>(index_info_->index_.get())) {
  if (!(plan->GetJoinType() == JoinType::LEFT || plan->GetJoinType() == JoinType::INNER)) {
    // Note for 2023 Spring: You ONLY need to implement left join and inner join.
    throw bustub::NotImplementedException(fmt::format("join type {} not supported", plan->GetJoinType()));
  }
}

void NestIndexJoinExecutor::Init() { child_executor_->Init(); }

auto NestIndexJoinExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  Tuple left_tuple;
  RID id;
  auto outer_schema = child_executor_->GetOutputSchema();
  auto inner_schema = plan_->InnerTableSchema();
  std::vector<Value> val;
  while (child_executor_->Next(&left_tuple, &id)) {
    Value value = plan_->KeyPredicate()->Evaluate(&left_tuple, outer_schema);
    std::vector<RID> righe_result;
    it_->ScanKey(Tuple{{value}, index_info_->index_->GetKeySchema()}, &righe_result, exec_ctx_->GetTransaction());

    Tuple right_tuple;
    if (!righe_result.empty()) {
      auto [m, t] = table_info_->table_->GetTuple(*righe_result.begin());
      right_tuple = t;
      val.reserve((outer_schema.GetColumnCount() + inner_schema.GetColumnCount()));

      for (uint32_t i = 0; i < outer_schema.GetColumnCount(); ++i) {
        val.emplace_back(left_tuple.GetValue(&outer_schema, i));
      }
      for (uint32_t i = 0; i < inner_schema.GetColumnCount(); ++i) {
        val.emplace_back(right_tuple.GetValue(&inner_schema, i));
      }
      *tuple = Tuple(val, &GetOutputSchema());
      return true;
    }
    if (plan_->GetJoinType() == JoinType::LEFT) {
      val.reserve((outer_schema.GetColumnCount() + inner_schema.GetColumnCount()));

      for (uint32_t i = 0; i < outer_schema.GetColumnCount(); ++i) {
        val.emplace_back(left_tuple.GetValue(&outer_schema, i));
      }
      for (uint32_t i = 0; i < inner_schema.GetColumnCount(); ++i) {
        val.emplace_back(ValueFactory::GetNullValueByType(inner_schema.GetColumn(i).GetType()));
      }
      *tuple = Tuple(val, &GetOutputSchema());
      return true;
    }
  }
  return false;
}

}  // namespace bustub
