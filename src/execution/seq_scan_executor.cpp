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
      it_(exec_ctx_->GetCatalog()->GetTable(plan_->table_name_)->table_->MakeEagerIterator()) {}

void SeqScanExecutor::Init() {
  // get lock info
  auto txn = exec_ctx_->GetTransaction();
  auto lock_mgr = exec_ctx_->GetLockManager();
  auto isolation = txn->GetIsolationLevel();
  auto oid = plan_->GetTableOid();

  // lock table accordingly
  if (exec_ctx_->IsDelete()) {
    // before locking the table, check if a higher-level lock is held (X, SIX)
    if (txn->IsTableExclusiveLocked(oid) || txn->IsTableSharedIntentionExclusiveLocked(oid)) {
      return;
    }
    // delete operation -> IX lock the entire table
    if (!lock_mgr->LockTable(txn, LockManager::LockMode::INTENTION_EXCLUSIVE, oid)) {
      throw ExecutionException("seqscan <delete>: failed acquiring IX lock on table");
    }
  } else if (isolation != IsolationLevel::READ_UNCOMMITTED) {
    // before locking the table, check if a higher-level lock is held (S, X, IX, SIX)
    if (txn->IsTableSharedLocked(oid) || txn->IsTableExclusiveLocked(oid) ||
        txn->IsTableIntentionExclusiveLocked(oid) || txn->IsTableSharedIntentionExclusiveLocked(oid)) {
      return;
    }
    // IS lock the entire table
    if (!lock_mgr->LockTable(txn, LockManager::LockMode::INTENTION_SHARED, oid)) {
      throw ExecutionException("seqscan: failed acquiring IS lock on table");
    }
  }
}

auto SeqScanExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  auto txn = exec_ctx_->GetTransaction();
  auto lock_mgr = exec_ctx_->GetLockManager();
  auto isolation = txn->GetIsolationLevel();
  auto oid = plan_->GetTableOid();

  // fetch the next tuple
  while (!it_.IsEnd()) {
    auto r = it_.GetRID();

    // lock based on context
    if (exec_ctx_->IsDelete()) {  // take X lock if current op is delete
      if (!lock_mgr->LockRow(txn, LockManager::LockMode::EXCLUSIVE, oid, r)) {
        throw ExecutionException("seqscan <delete>: failed acquiring X lock");
      }
    } else if (isolation != IsolationLevel::READ_UNCOMMITTED) {  // Slock the tuple except for READ_UNCOMMIT
      if (!txn->IsRowExclusiveLocked(oid, r) && !lock_mgr->LockRow(txn, LockManager::LockMode::SHARED, oid, r)) {
        throw ExecutionException("seqscan: failed acquiring S lock");
      }
    }

    // store tuple data
    auto [m, t] = it_.GetTuple();
    ++it_;

    // tuple deleted -> force unlock
    if (m.is_deleted_) {
      // if not delete op, READ_UNCOMMITED does not need to be unlocked
      if (!exec_ctx_->IsDelete() && isolation != IsolationLevel::READ_UNCOMMITTED) {
        lock_mgr->UnlockRow(txn, oid, r, true);
      }

      // if is delete op, all isolation level are locked
      if (exec_ctx_->IsDelete()) {
        lock_mgr->UnlockRow(txn, oid, r, true);
      }
    } else {
      // release S lock immediately for READ_COMMITED
      if (!exec_ctx_->IsDelete() && isolation == IsolationLevel::READ_COMMITTED) {
        lock_mgr->UnlockRow(txn, oid, r);
      }

      // write tuple to output
      *tuple = t;
      *rid = r;
      return true;
    }
  }
  return false;
}

}  // namespace bustub
