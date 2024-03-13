//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// update_executor.cpp
//
// Identification: src/execution/update_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include <memory>

#include "execution/executors/update_executor.h"

namespace bustub {

UpdateExecutor::UpdateExecutor(ExecutorContext *exec_ctx, const UpdatePlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), child_executor_(std::move(child_executor)) {
  // As of Fall 2022, you DON'T need to implement update executor to have perfect score in project 3 / project 4.
}

void UpdateExecutor::Init() { child_executor_->Init(); }

auto UpdateExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) -> bool {
  auto catalog = exec_ctx_->GetCatalog();
  auto table_meta = catalog->GetTable(plan_->TableOid());
  auto idx = catalog->GetTableIndexes(table_meta->name_);
  int cnt = 0;
  RID id;
  Tuple t;
  while (child_executor_->Next(&t, &id)) {
    const TupleMeta old_tm{INVALID_TXN_ID, INVALID_TXN_ID, true};
    table_meta->table_->UpdateTupleMeta(old_tm, id);
    const TupleMeta new_tm{INVALID_TXN_ID, INVALID_TXN_ID, false};
    std::vector<Value> val;
    val.reserve(table_meta->schema_.GetColumns().size());
    for (const auto &expr : plan_->target_expressions_) {
      val.emplace_back(expr->Evaluate(&t, child_executor_->GetOutputSchema()));
    }
    Tuple new_tuple = Tuple(val, &child_executor_->GetOutputSchema());
    auto new_rid = table_meta->table_->InsertTuple(new_tm, new_tuple);

    for (auto index : idx) {
      auto old_key = t.KeyFromTuple(table_meta->schema_, index->key_schema_, index->index_->GetKeyAttrs());
      index->index_->DeleteEntry(old_key, id, nullptr);
      auto key = new_tuple.KeyFromTuple(table_meta->schema_, index->key_schema_, index->index_->GetKeyAttrs());
      index->index_->InsertEntry(key, new_rid.value(), nullptr);
    }
    cnt++;
  }
  std::vector<Value> val{Value(INTEGER, cnt)};
  std::vector<Column> col{Column("count", INTEGER)};
  auto sch = Schema(col);
  *tuple = Tuple(val, &sch);
  if (!is_updated_) {
    is_updated_ = true;
    return true;
  }
  return false;
}

}  // namespace bustub
