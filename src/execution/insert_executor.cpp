//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// insert_executor.cpp
//
// Identification: src/execution/insert_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <memory>
#include <vector>
#include "catalog/column.h"
#include "common/rid.h"
#include "storage/table/tuple.h"
#include "type/type_id.h"
#include "type/value.h"

#include "execution/executors/insert_executor.h"

namespace bustub {

InsertExecutor::InsertExecutor(ExecutorContext *exec_ctx, const InsertPlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), child_executor_(std::move(child_executor)) {}

void InsertExecutor::Init() { child_executor_->Init(); }

auto InsertExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) -> bool {
  auto catalog = exec_ctx_->GetCatalog();
  auto table_meta = catalog->GetTable(plan_->TableOid());
  auto idx = catalog->GetTableIndexes(table_meta->name_);
  int cnt = 0;
  RID id;
  Tuple t;
  while (child_executor_->Next(&t, &id)) {
    const TupleMeta tm{INVALID_TXN_ID, INVALID_TXN_ID, false};
    const auto new_rid = table_meta->table_->InsertTuple(tm, t);
    for (auto index : idx) {
      auto key = t.KeyFromTuple(table_meta->schema_, index->key_schema_, index->index_->GetKeyAttrs());
      index->index_->InsertEntry(key, new_rid.value(), nullptr);
    }
    cnt++;
  }
  std::vector<Value> val{Value(INTEGER, cnt)};
  std::vector<Column> col{Column("count", INTEGER)};
  auto sch = Schema(col);
  *tuple = Tuple(val, &sch);
  if (!is_inserted_) {
    is_inserted_ = true;
    return true;
  }
  return false;
}

}  // namespace bustub
