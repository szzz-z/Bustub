#include "execution/executors/topn_executor.h"
#include "binder/bound_order_by.h"
#include "common/rid.h"
#include "storage/table/tuple.h"

namespace bustub {

TopNExecutor::TopNExecutor(ExecutorContext *exec_ctx, const TopNPlanNode *plan,
                           std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx),
      plan_(plan),
      child_executor_(std::move(child_executor)),
      cmp_([this](const Tuple &t1, const Tuple &t2) -> bool {
        auto schema = GetOutputSchema();
        for (const auto &order_by : plan_->GetOrderBy()) {
          auto type = order_by.first;
          auto expr = order_by.second;
          if (static_cast<bool>(expr->Evaluate(&t1, schema).CompareEquals(expr->Evaluate(&t2, schema)))) {
            continue;
          }
          switch (type) {
            case bustub::OrderByType::INVALID:
            case bustub::OrderByType::DEFAULT:
            case bustub::OrderByType::ASC:
              return static_cast<bool>(expr->Evaluate(&t1, schema).CompareLessThan(expr->Evaluate(&t2, schema)));
            case bustub::OrderByType::DESC:
              return static_cast<bool>(expr->Evaluate(&t1, schema).CompareGreaterThan(expr->Evaluate(&t2, schema)));
          }
        }
        return false;
      }),
      heap_(cmp_) {}

void TopNExecutor::Init() {
  child_executor_->Init();
  Tuple t;
  RID id;
  while (child_executor_->Next(&t, &id)) {
    heap_.push(std::move(t));
    if (GetNumInHeap() > plan_->GetN()) {
      heap_.pop();
    }
  }
  output_tuple_.reserve(heap_.size());
  while (!heap_.empty()) {
    output_tuple_.emplace_back(heap_.top());
    heap_.pop();
  }
  std::reverse(output_tuple_.begin(), output_tuple_.end());
  it_ = output_tuple_.begin();
}

auto TopNExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  if (it_ != output_tuple_.end()) {
    *tuple = *it_++;
    *rid = tuple->GetRid();
    return true;
  }
  return false;
}

auto TopNExecutor::GetNumInHeap() -> size_t { return heap_.size(); }

}  // namespace bustub
