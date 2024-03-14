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
#include <vector>

#include "execution/executors/update_executor.h"

namespace bustub {

/**
 * Construct a new UpdateExecutor instance.
 * @param exec_ctx The executor context
 * @param plan The update plan to be executed
 * @param child_executor The child executor that feeds the update
 */
UpdateExecutor::UpdateExecutor(ExecutorContext *exec_ctx, const UpdatePlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), child_executor_(std::move(child_executor)) {
  // As of Fall 2022, you DON'T need to implement update executor to have perfect score in project 3 / project 4.
}

void UpdateExecutor::Init() { child_executor_->Init(); }

/**
 * Yield the next tuple from the udpate. To implement an update, first delete the affected tuple and then insert a new
 * tuple.
 * @param[out] tuple The next tuple produced by the update
 * @return `true` if a tuple was produced, `false` if there are no more tuples
 */
auto UpdateExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) -> bool {
  if (!updated_) {
    updated_ = true;
    auto txn_id = exec_ctx_->GetTransaction()->GetTransactionId();
    auto catalog = exec_ctx_->GetCatalog();
    auto table_meta = catalog->GetTable(plan_->TableOid());
    auto indexes = catalog->GetTableIndexes(table_meta->name_);

    // Update tuples
    int count = 0;
    Tuple t;
    RID r;
    const TupleMeta new_meta{txn_id, INVALID_TXN_ID, false};
    while (child_executor_->Next(&t, &r)) {
      // First remove the tuple from the table
      auto old_meta = table_meta->table_->GetTupleMeta(r);
      old_meta.delete_txn_id_ = txn_id;
      old_meta.is_deleted_ = true;
      table_meta->table_->UpdateTupleMeta(old_meta, r);

      // Then insert the new tuple into the table
      std::vector<Value> vec;
      vec.reserve(table_meta->schema_.GetColumns().size());
      for (const auto &expr : plan_->target_expressions_) {
        vec.emplace_back(expr->Evaluate(&t, child_executor_->GetOutputSchema()));
      }
      Tuple new_tuple = Tuple(vec, &child_executor_->GetOutputSchema());
      auto new_rid = table_meta->table_->InsertTuple(new_meta, new_tuple);
      BUSTUB_ASSERT(new_rid, "Insertion failed");

      // Update indexes (if any)
      for (auto index_meta : indexes) {
        auto old_key = t.KeyFromTuple(table_meta->schema_, index_meta->key_schema_, index_meta->index_->GetKeyAttrs());
        index_meta->index_->DeleteEntry(old_key, r, nullptr);
        auto key =
            new_tuple.KeyFromTuple(table_meta->schema_, index_meta->key_schema_, index_meta->index_->GetKeyAttrs());
        index_meta->index_->InsertEntry(key, *new_rid, nullptr);
      }
      count++;
    }

    // Emit number of updated rows
    std::vector<Value> vec(1, Value(INTEGER, count));
    const std::vector<Column> cols(1, Column("count", INTEGER));
    const auto s = Schema(cols);
    *tuple = Tuple(vec, &s);
    return true;
  }
  return false;
}

}  // namespace bustub
