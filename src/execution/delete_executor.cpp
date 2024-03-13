//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// delete_executor.cpp
//
// Identification: src/execution/delete_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <memory>
#include <utility>

#include "execution/executors/delete_executor.h"

namespace bustub {

DeleteExecutor::DeleteExecutor(ExecutorContext *exec_ctx, const DeletePlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), child_executor_(std::move(child_executor)) {}

void DeleteExecutor::Init() { return child_executor_->Init(); }

auto DeleteExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) -> bool {
  auto catalog = exec_ctx_->GetCatalog();
  auto table_meta = catalog->GetTable(plan_->TableOid());
  auto idx = catalog->GetTableIndexes(table_meta->name_);
  int cnt = 0;
  RID id;
  Tuple t;
  while (child_executor_->Next(&t, &id)) {
    const TupleMeta tm{INVALID_TXN_ID, INVALID_TXN_ID, true};
    table_meta->table_->UpdateTupleMeta(tm, id);
    for (auto index : idx) {
      auto key = t.KeyFromTuple(table_meta->schema_, index->key_schema_, index->index_->GetKeyAttrs());
      index->index_->DeleteEntry(key, id, nullptr);
    }
    cnt++;
  }
  std::vector<Value> val{Value(INTEGER, cnt)};
  std::vector<Column> col{Column("count", INTEGER)};
  auto sch = Schema(col);
  *tuple = Tuple(val, &sch);
  if (!is_deleted_) {
    is_deleted_ = true;
    return true;
  }
  return false;
}

}  // namespace bustub
