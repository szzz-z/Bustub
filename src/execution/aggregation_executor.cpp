//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// aggregation_executor.cpp
//
// Identification: src/execution/aggregation_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include <memory>
#include <utility>
#include <vector>
#include "common/rid.h"
#include "storage/table/tuple.h"

#include "execution/executors/aggregation_executor.h"

namespace bustub {

AggregationExecutor::AggregationExecutor(ExecutorContext *exec_ctx, const AggregationPlanNode *plan,
                                         std::unique_ptr<AbstractExecutor> &&child)
    : AbstractExecutor(exec_ctx),
      plan_(plan),
      child_(std::move(child)),
      aht_({plan_->aggregates_, plan_->agg_types_}),
      aht_iterator_({aht_.Begin()}) {}

void AggregationExecutor::Init() {
  child_->Init();
  Tuple t;
  RID id;
  while (child_->Next(&t, &id)) {
    aht_.InsertCombine(MakeAggregateKey(&t), MakeAggregateValue(&t));
    aht_iterator_ = aht_.Begin();
    if (aht_iterator_ == aht_.End() && plan_->GetGroupBys().empty()) {
      aht_.InsertEmpty();
      aht_iterator_ = aht_.Begin();
    }
  }
}

auto AggregationExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  if (aht_iterator_ != aht_.End()) {
    std::vector<Value> val;
    val.insert(val.end(), aht_iterator_.Key().group_bys_.begin(), aht_iterator_.Key().group_bys_.end());
    val.insert(val.end(), aht_iterator_.Val().aggregates_.begin(), aht_iterator_.Val().aggregates_.end());
    *tuple = {val, &GetOutputSchema()};
    *rid = tuple->GetRid();
    ++aht_iterator_;
    return true;
  }
  return false;
}

auto AggregationExecutor::GetChildExecutor() const -> const AbstractExecutor * { return child_.get(); }

}  // namespace bustub
