//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lock_manager.cpp
//
// Identification: src/concurrency/lock_manager.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "concurrency/lock_manager.h"

#include <memory>
#include <mutex>

#include "common/config.h"
#include "concurrency/transaction.h"
#include "concurrency/transaction_manager.h"

namespace bustub {

auto LockManager::LockTable(Transaction *txn, LockMode lock_mode, const table_oid_t &oid) -> bool {
  if (!CanTxnTakeLock(txn, lock_mode)) {
    return false;
  }
  // find the lock request queue for the table (create one if not exist)
  table_lock_map_latch_.lock();
  auto lock_req_on_table = table_lock_map_.find(oid);
  if (lock_req_on_table == table_lock_map_.end()) {
    lock_req_on_table = table_lock_map_.emplace(oid, std::make_shared<LockRequestQueue>()).first;
  }

  auto req_queue = lock_req_on_table->second;
  std::unique_lock<std::mutex> lock(req_queue->latch_);
  table_lock_map_latch_.unlock();

  auto txn_id = txn->GetTransactionId();
  auto prev_req = std::find_if(req_queue->request_queue_.begin(), req_queue->request_queue_.end(),
                               [txn_id](const std::shared_ptr<bustub::LockManager::LockRequest> &current) {
                                 return current->txn_id_ == txn_id;
                               });

  if (prev_req != req_queue->request_queue_.end()) {
    auto prev_lock_mode = (*prev_req)->lock_mode_;
    if (prev_lock_mode == lock_mode) {
      lock.unlock();
      return true;
    }

    if (req_queue->upgrading_ != INVALID_TXN_ID) {
      lock.unlock();
      txn->SetState(TransactionState::ABORTED);
      throw TransactionAbortException(txn_id, AbortReason::UPGRADE_CONFLICT);
    }

    if (!CanLockUpgrade(prev_lock_mode, lock_mode)) {
      lock.unlock();
      txn->SetState(TransactionState::ABORTED);
      throw TransactionAbortException(txn_id, AbortReason::INCOMPATIBLE_UPGRADE);
    }

    req_queue->upgrading_ = txn_id;

    GetTableLockSet(txn, prev_lock_mode)->erase(oid);
    (*prev_req)->lock_mode_ = lock_mode;

    bool has_incompatible_lock = true;
    while (has_incompatible_lock) {
      if (txn->GetState() == TransactionState::ABORTED) {
        req_queue->upgrading_ = INVALID_TXN_ID;
        req_queue->request_queue_.remove(*prev_req);
        req_queue->cv_.notify_all();
        return false;
      }
      has_incompatible_lock = false;
      for (auto &req : req_queue->request_queue_) {
        if (req->txn_id_ != txn_id && req->granted_ && !AreLocksCompatible(req->lock_mode_, lock_mode)) {
          req_queue->cv_.wait(lock);
          has_incompatible_lock = true;
          break;
        }
      }
    }

    GetTableLockSet(txn, lock_mode)->emplace(oid);
    req_queue->upgrading_ = INVALID_TXN_ID;
    lock.unlock();
    return true;
  }

  auto new_req = std::make_shared<LockRequest>(txn_id, lock_mode, oid);
  req_queue->request_queue_.push_back(new_req);

  bool has_incompatible_lock = true;
  while (has_incompatible_lock) {
    if (txn->GetState() == TransactionState::ABORTED) {
      req_queue->request_queue_.remove(new_req);
      req_queue->cv_.notify_all();
      return false;
    }
    has_incompatible_lock = false;
    for (auto &req : req_queue->request_queue_) {
      if (req->txn_id_ != txn_id && req->granted_ && !AreLocksCompatible(req->lock_mode_, lock_mode)) {
        req_queue->cv_.wait(lock);
        has_incompatible_lock = true;
        break;
      }
    }
  }

  new_req->granted_ = true;
  GetTableLockSet(txn, lock_mode)->emplace(oid);
  lock.unlock();
  return true;
}

auto LockManager::UnlockTable(Transaction *txn, const table_oid_t &oid) -> bool {
  table_lock_map_latch_.lock();
  auto lock_req_on_table = table_lock_map_.find(oid);
  auto txn_id = txn->GetTransactionId();

  if (lock_req_on_table == table_lock_map_.end()) {
    table_lock_map_latch_.unlock();
    txn->SetState(TransactionState::ABORTED);
    throw TransactionAbortException(txn_id, AbortReason::ATTEMPTED_UNLOCK_BUT_NO_LOCK_HELD);
  }

  auto req_queue = lock_req_on_table->second;
  std::unique_lock<std::mutex> lock(req_queue->latch_);
  table_lock_map_latch_.unlock();

  auto req_it = std::find_if(req_queue->request_queue_.begin(), req_queue->request_queue_.end(),
                             [txn_id](const auto &req) { return req->txn_id_ == txn_id && req->granted_; });
  // there is no lock
  if (req_it == req_queue->request_queue_.end()) {
    lock.unlock();
    txn->SetState(TransactionState::ABORTED);
    throw TransactionAbortException(txn_id, AbortReason::ATTEMPTED_UNLOCK_BUT_NO_LOCK_HELD);
  }

  if (HoldsRowOnTable(txn, oid)) {
    lock.unlock();
    txn->SetState(TransactionState::ABORTED);
    throw TransactionAbortException(txn_id, AbortReason::TABLE_UNLOCKED_BEFORE_UNLOCKING_ROWS);
  }

  auto lock_mode = (*req_it)->lock_mode_;
  if (lock_mode == LockMode::SHARED || lock_mode == LockMode::EXCLUSIVE) {
    switch (txn->GetIsolationLevel()) {
      case IsolationLevel::REPEATABLE_READ:
        txn->SetState(TransactionState::SHRINKING);
        break;
      case IsolationLevel::READ_COMMITTED:
        if (lock_mode == LockMode::EXCLUSIVE) {
          txn->SetState(TransactionState::SHRINKING);
        }
        break;
      case IsolationLevel::READ_UNCOMMITTED:
        txn->SetState(TransactionState::SHRINKING);
        break;
    }
  }
  GetTableLockSet(txn, lock_mode)->erase(oid);
  req_queue->request_queue_.erase(req_it);
  req_queue->cv_.notify_all();
  lock.unlock();
  return true;
}

auto LockManager::LockRow(Transaction *txn, LockMode lock_mode, const table_oid_t &oid, const RID &rid) -> bool {
  if (!CanTxnTakeLock(txn, lock_mode)) {
    return false;
  }
  auto txn_id = txn->GetTransactionId();
  switch (lock_mode) {
    case LockMode::SHARED:
      if (!txn->IsTableExclusiveLocked(oid) && !txn->IsTableSharedLocked(oid) &&
          !txn->IsTableIntentionExclusiveLocked(oid) && !txn->IsTableIntentionSharedLocked(oid) &&
          !txn->IsTableSharedIntentionExclusiveLocked(oid)) {
        txn->SetState(TransactionState::ABORTED);
        throw TransactionAbortException(txn_id, AbortReason::TABLE_LOCK_NOT_PRESENT);
      }
      break;
    case LockMode::EXCLUSIVE:
      if (!txn->IsTableExclusiveLocked(oid) && !txn->IsTableIntentionExclusiveLocked(oid) &&
          !txn->IsTableSharedIntentionExclusiveLocked(oid)) {
        txn->SetState(TransactionState::ABORTED);
        throw TransactionAbortException(txn_id, AbortReason::TABLE_LOCK_NOT_PRESENT);
      }
      break;
    case LockMode::INTENTION_SHARED:
    case LockMode::INTENTION_EXCLUSIVE:
    case LockMode::SHARED_INTENTION_EXCLUSIVE:
      txn->SetState(TransactionState::ABORTED);
      throw TransactionAbortException(txn_id, AbortReason::ATTEMPTED_INTENTION_LOCK_ON_ROW);
  }
  row_lock_map_latch_.lock();
  auto lock_req_on_row = row_lock_map_.find(rid);
  if (lock_req_on_row == row_lock_map_.end()) {
    lock_req_on_row = row_lock_map_.emplace(rid, std::make_shared<LockRequestQueue>()).first;
  }
  auto req_queue = lock_req_on_row->second;
  std::unique_lock<std::mutex> lock(req_queue->latch_);
  row_lock_map_latch_.unlock();
  auto prev_req = std::find_if(req_queue->request_queue_.begin(), req_queue->request_queue_.end(),
                               [txn_id](const std::shared_ptr<bustub::LockManager::LockRequest> &current) {
                                 return current->txn_id_ == txn_id;
                               });
  if (prev_req != req_queue->request_queue_.end()) {
    auto prev_lock_mode = (*prev_req)->lock_mode_;
    if (prev_lock_mode == lock_mode) {
      lock.unlock();
      return true;
    }
    if (req_queue->upgrading_ != INVALID_TXN_ID) {
      lock.unlock();
      txn->SetState(TransactionState::ABORTED);
      throw TransactionAbortException(txn_id, AbortReason::UPGRADE_CONFLICT);
    }

    if (!CanLockUpgrade(prev_lock_mode, lock_mode)) {
      lock.unlock();
      txn->SetState(TransactionState::ABORTED);
      throw TransactionAbortException(txn_id, AbortReason::INCOMPATIBLE_UPGRADE);
    }

    req_queue->upgrading_ = txn_id;

    RemoveFromRowLockSet(txn, prev_lock_mode, oid, rid);
    (*prev_req)->lock_mode_ = lock_mode;

    bool has_incompatible_lock = true;
    while (has_incompatible_lock) {
      if (txn->GetState() == TransactionState::ABORTED) {
        req_queue->upgrading_ = INVALID_TXN_ID;
        req_queue->request_queue_.remove(*prev_req);
        req_queue->cv_.notify_all();
        return false;
      }
      has_incompatible_lock = false;
      for (auto &req : req_queue->request_queue_) {
        if (req->txn_id_ != txn_id && req->granted_ && !AreLocksCompatible(req->lock_mode_, lock_mode)) {
          req_queue->cv_.wait(lock);
          has_incompatible_lock = true;
          break;
        }
      }
    }

    AddToRowLockSet(txn, lock_mode, oid, rid);
    req_queue->upgrading_ = INVALID_TXN_ID;
    lock.unlock();
    return true;
  }
  auto new_req = std::make_shared<LockRequest>(txn_id, lock_mode, oid, rid);
  req_queue->request_queue_.push_back(new_req);

  bool has_incompatible_lock = true;
  while (has_incompatible_lock) {
    if (txn->GetState() == TransactionState::ABORTED) {
      req_queue->request_queue_.erase(
          std::remove(req_queue->request_queue_.begin(), req_queue->request_queue_.end(), new_req),
          req_queue->request_queue_.end());
      req_queue->cv_.notify_all();
      return false;
    }
    has_incompatible_lock = false;
    for (auto &req : req_queue->request_queue_) {
      if (req->txn_id_ != txn_id && req->granted_ && !AreLocksCompatible(req->lock_mode_, lock_mode)) {
        req_queue->cv_.wait(lock);
        has_incompatible_lock = true;
        break;
      }
    }
  }

  new_req->granted_ = true;
  AddToRowLockSet(txn, lock_mode, oid, rid);
  lock.unlock();
  return true;
}

auto LockManager::UnlockRow(Transaction *txn, const table_oid_t &oid, const RID &rid, bool force) -> bool {
  row_lock_map_latch_.lock();
  auto lock_req_on_row = row_lock_map_.find(rid);

  auto txn_id = txn->GetTransactionId();
  if (lock_req_on_row == row_lock_map_.end()) {
    row_lock_map_latch_.unlock();
    txn->SetState(TransactionState::ABORTED);
    throw TransactionAbortException(txn_id, AbortReason::ATTEMPTED_UNLOCK_BUT_NO_LOCK_HELD);
    return false;
  }

  auto req_queue = lock_req_on_row->second;
  std::unique_lock<std::mutex> lock(req_queue->latch_);
  row_lock_map_latch_.unlock();

  auto req_it =
      std::find_if(req_queue->request_queue_.begin(), req_queue->request_queue_.end(), [txn_id, oid, rid](auto &req) {
        return req->txn_id_ == txn_id && req->granted_ && req->oid_ == oid && req->rid_ == rid;
      });
  if (req_it == req_queue->request_queue_.end()) {
    lock.unlock();
    txn->SetState(TransactionState::ABORTED);
    throw TransactionAbortException(txn_id, AbortReason::ATTEMPTED_UNLOCK_BUT_NO_LOCK_HELD);
    return false;
  }

  auto lock_mode = (*req_it)->lock_mode_;
  if (!force) {
    if (lock_mode == LockMode::SHARED || lock_mode == LockMode::EXCLUSIVE) {
      switch (txn->GetIsolationLevel()) {
        case IsolationLevel::REPEATABLE_READ:
          txn->SetState(TransactionState::SHRINKING);
          break;
        case IsolationLevel::READ_COMMITTED:
          if (lock_mode == LockMode::EXCLUSIVE) {
            txn->SetState(TransactionState::SHRINKING);
          }
          break;
        case IsolationLevel::READ_UNCOMMITTED:

          txn->SetState(TransactionState::SHRINKING);
          break;
      }
    }
  }

  RemoveFromRowLockSet(txn, lock_mode, oid, rid);
  req_queue->request_queue_.erase(req_it);
  req_queue->cv_.notify_all();

  lock.unlock();
  return true;
}

void LockManager::UnlockAll() {
  // You probably want to unlock all table and txn locks here.
}

void LockManager::AddEdge(txn_id_t t1, txn_id_t t2) {}

void LockManager::RemoveEdge(txn_id_t t1, txn_id_t t2) {}

auto LockManager::HasCycle(txn_id_t *txn_id) -> bool { return false; }

auto LockManager::GetEdgeList() -> std::vector<std::pair<txn_id_t, txn_id_t>> {
  std::vector<std::pair<txn_id_t, txn_id_t>> edges(0);
  return edges;
}

void LockManager::RunCycleDetection() {
  while (enable_cycle_detection_) {
    std::this_thread::sleep_for(cycle_detection_interval);
    {  // TODO(students): detect deadlock
    }
  }
}

}  // namespace bustub
