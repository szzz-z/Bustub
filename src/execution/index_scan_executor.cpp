//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// index_scan_executor.cpp
//
// Identification: src/execution/index_scan_executor.cpp
//
// Copyright (c) 2015-19, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include "execution/executors/index_scan_executor.h"

namespace bustub {
IndexScanExecutor::IndexScanExecutor(ExecutorContext *exec_ctx, const IndexScanPlanNode *plan)
    : AbstractExecutor(exec_ctx),
      plan_(plan),
      index_info_(exec_ctx_->GetCatalog()->GetIndex(plan_->GetIndexOid())),
      it_(dynamic_cast<BPlusTreeIndexForTwoIntegerColumn *>(index_info_->index_.get())->GetBeginIterator()) {}

void IndexScanExecutor::Init() {}

auto IndexScanExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  while (!it_.IsEnd()) {
    auto id = (*it_).second;
    auto [m, t] = exec_ctx_->GetCatalog()->GetTable(index_info_->table_name_)->table_->GetTuple(id);
    ++it_;
    if (!m.is_deleted_) {
      *tuple = t;
      *rid = id;
      return true;
    }
  }
  return false;
}

}  // namespace bustub
