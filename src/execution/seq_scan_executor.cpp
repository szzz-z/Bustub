//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// seq_scan_executor.cpp
//
// Identification: src/execution/seq_scan_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/seq_scan_executor.h"

namespace bustub {

SeqScanExecutor::SeqScanExecutor(ExecutorContext *exec_ctx, const SeqScanPlanNode *plan)
    : AbstractExecutor(exec_ctx),
      plan_(plan),
      it_(exec_ctx_->GetCatalog()->GetTable(plan_->table_name_)->table_->MakeIterator()) {}

void SeqScanExecutor::Init() {}

auto SeqScanExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  while (!it_.IsEnd()) {
    auto [m, t] = it_.GetTuple();
    auto id = it_.GetRID();
    ++it_;
    if (!m.is_deleted_) {
      *tuple = t;
      *rid = id;
      return true;
      ;
    }
  }
  return false;
}

}  // namespace bustub
