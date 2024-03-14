//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// transaction_manager.cpp
//
// Identification: src/concurrency/transaction_manager.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "concurrency/transaction_manager.h"

#include <mutex>  // NOLINT
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

#include "catalog/catalog.h"
#include "common/macros.h"
#include "storage/table/table_heap.h"
namespace bustub {

void TransactionManager::Commit(Transaction *txn) {
  // Release all the locks.
  ReleaseLocks(txn);

  txn->SetState(TransactionState::COMMITTED);
}

void TransactionManager::Abort(Transaction *txn) {
  /* TODO: revert all the changes in write set */
  // revert insertion and deletions
  auto txn_id = txn->GetTransactionId();
  auto table_write_set = txn->GetWriteSet();
  while (!table_write_set->empty()) {
    auto record = table_write_set->back();
    auto table_heap = record.table_heap_;
    auto [m, t] = record.table_heap_->GetTuple(record.rid_);

    // re-insert / delete tuples
    if (m.is_deleted_ && m.delete_txn_id_ == txn_id) {
      m.is_deleted_ = false;
      m.delete_txn_id_ = INVALID_TXN_ID;
    } else if (!m.is_deleted_ && m.insert_txn_id_ == txn_id) {
      m.is_deleted_ = true;
      m.delete_txn_id_ = txn_id;
    }
    // update meta in table heap
    table_heap->UpdateTupleMeta(m, record.rid_);
    txn->GetWriteSet()->pop_back();
  }
  ReleaseLocks(txn);

  txn->SetState(TransactionState::ABORTED);
}

void TransactionManager::BlockAllTransactions() { UNIMPLEMENTED("block is not supported now!"); }

void TransactionManager::ResumeTransactions() { UNIMPLEMENTED("resume is not supported now!"); }

}  // namespace bustub
