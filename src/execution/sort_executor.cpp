#include "execution/executors/sort_executor.h"
#include "common/rid.h"
#include "storage/table/tuple.h"

namespace bustub {

SortExecutor::SortExecutor(ExecutorContext *exec_ctx, const SortPlanNode *plan,
                           std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), child_executor_(std::move(child_executor)) {}

void SortExecutor::Init() {
  Tuple t;
  RID id;
  child_executor_->Init();
  while (child_executor_->Next(&t, &id)) {
    sorted_.emplace_back(t);
  }
  auto schema = plan_->OutputSchema();
  auto order_by = plan_->GetOrderBy();
  auto cmp = [=](const Tuple &t1, const Tuple &t2) -> bool {
    for (const auto &order : order_by) {
      auto order_by_type = order.first;
      auto expr = order.second;
      if (static_cast<bool>(expr->Evaluate(&t1, schema).CompareEquals(expr->Evaluate(&t2, schema)))) {
        continue;
      }
      switch (order_by_type) {
        case OrderByType::INVALID:
        case OrderByType::DEFAULT:
        case OrderByType::ASC:
          return static_cast<bool>(expr->Evaluate(&t1, schema).CompareLessThan(expr->Evaluate(&t2, schema)));
        case OrderByType::DESC:
          return static_cast<bool>(expr->Evaluate(&t1, schema).CompareGreaterThan(expr->Evaluate(&t2, schema)));
      }
    }
    return false;
  };
  std::sort(sorted_.begin(), sorted_.end(), cmp);
  it_ = sorted_.begin();
}

auto SortExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  if (it_ != sorted_.end()) {
    *tuple = *it_;
    *rid = tuple->GetRid();
    ++it_;
    return true;
  }
  return false;
}

}  // namespace bustub
