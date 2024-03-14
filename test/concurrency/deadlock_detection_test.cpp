/**
 * deadlock_detection_test.cpp
 */

#include <atomic>
#include <random>
#include <thread>  // NOLINT

#include "common/config.h"
#include "concurrency/lock_manager.h"
#include "concurrency/transaction_manager.h"
#include "gtest/gtest.h"

namespace bustub {
TEST(LockManagerDeadLockDetectionTest, CycleTraverseOrderTest1) {
  LockManager lock_mgr{};
  TransactionManager txn_mgr{&lock_mgr};
  lock_mgr.txn_manager_ = &txn_mgr;
  lock_mgr.StartDeadlockDetection();
  /*
    15: 7 -> 2
    15: 6 -> 7
    15: 5 -> 0
    15: 4 -> 5
    15: 3 -> 4
    15: 2 -> 3 6
    15: 1 -> 2
    15: 0 -> 1
  */
  lock_mgr.AddEdge(7, 2);
  lock_mgr.AddEdge(6, 7);
  lock_mgr.AddEdge(5, 0);
  lock_mgr.AddEdge(4, 5);
  lock_mgr.AddEdge(3, 4);
  lock_mgr.AddEdge(2, 3);
  lock_mgr.AddEdge(2, 6);
  lock_mgr.AddEdge(1, 2);
  lock_mgr.AddEdge(0, 1);

  auto lock_mgr_edges = lock_mgr.GetEdgeList();
}

TEST(LockManagerDeadLockDetectionTest, CycleTraverseOrderTest2) {
  LockManager lock_mgr{};
  TransactionManager txn_mgr{&lock_mgr};
  lock_mgr.txn_manager_ = &txn_mgr;
  lock_mgr.StartDeadlockDetection();

  lock_mgr.AddEdge(0, 1);
  lock_mgr.AddEdge(1, 2);
  lock_mgr.AddEdge(2, 3);
  lock_mgr.AddEdge(3, 0);
  lock_mgr.AddEdge(3, 2);

  auto lock_mgr_edges = lock_mgr.GetEdgeList();
}

TEST(LockManagerDeadlockDetectionTest, CycleDetectionTest) {
  LockManager lock_mgr{};
  TransactionManager txn_mgr{&lock_mgr};
  lock_mgr.txn_manager_ = &txn_mgr;
  lock_mgr.StartDeadlockDetection();

  // create txn ids
  // 15: 7 -> 2
  // 15: 6 -> 7
  // 15: 5 -> 0
  // 15: 4 -> 5
  // 15: 3 -> 4
  // 15: 2 -> 3
  // 15: 1 -> 2
  // 15: 0 -> 1
  lock_mgr.AddEdge(7, 2);
  lock_mgr.AddEdge(6, 7);
  lock_mgr.AddEdge(5, 0);
  lock_mgr.AddEdge(4, 5);
  lock_mgr.AddEdge(3, 4);
  lock_mgr.AddEdge(2, 3);
  lock_mgr.AddEdge(1, 2);
  lock_mgr.AddEdge(0, 1);

  // Create edges by pairing adjacent txn_ids
  auto lock_mgr_edges = lock_mgr.GetEdgeList();
}

TEST(LockManagerDeadlockDetectionTest, EdgeTest) {
  LockManager lock_mgr{};
  TransactionManager txn_mgr{&lock_mgr};
  lock_mgr.txn_manager_ = &txn_mgr;
  lock_mgr.StartDeadlockDetection();

  const int num_nodes = 100;
  const int num_edges = num_nodes / 2;
  const int seed = 15445;
  std::srand(seed);

  // Create txn ids and shuffle
  std::vector<txn_id_t> txn_ids;
  txn_ids.reserve(num_nodes);
  for (int i = 0; i < num_nodes; i++) {
    txn_ids.push_back(i);
  }
  EXPECT_EQ(num_nodes, txn_ids.size());
  auto rng = std::default_random_engine{};
  std::shuffle(txn_ids.begin(), txn_ids.end(), rng);
  EXPECT_EQ(num_nodes, txn_ids.size());

  // Create edges by pairing adjacent txn_ids
  std::vector<std::pair<txn_id_t, txn_id_t>> edges;
  for (int i = 0; i < num_nodes; i += 2) {
    EXPECT_EQ(i / 2, lock_mgr.GetEdgeList().size());
    auto t1 = txn_ids[i];
    auto t2 = txn_ids[i + 1];
    lock_mgr.AddEdge(t1, t2);
    edges.emplace_back(t1, t2);
    EXPECT_EQ((i / 2) + 1, lock_mgr.GetEdgeList().size());
  }

  auto lock_mgr_edges = lock_mgr.GetEdgeList();
  EXPECT_EQ(num_edges, lock_mgr_edges.size());
  EXPECT_EQ(num_edges, edges.size());

  std::sort(lock_mgr_edges.begin(), lock_mgr_edges.end());
  std::sort(edges.begin(), edges.end());

  for (int i = 0; i < num_edges; i++) {
    EXPECT_EQ(edges[i], lock_mgr_edges[i]);
  }
}

TEST(LockManagerDeadlockDetectionTest, BasicDeadlockDetectionTest) {
  LockManager lock_mgr{};
  TransactionManager txn_mgr{&lock_mgr};
  lock_mgr.txn_manager_ = &txn_mgr;
  lock_mgr.StartDeadlockDetection();

  table_oid_t toid{0};
  RID rid0{0, 0};
  RID rid1{1, 1};
  auto *txn0 = txn_mgr.Begin();
  auto *txn1 = txn_mgr.Begin();
  EXPECT_EQ(0, txn0->GetTransactionId());
  EXPECT_EQ(1, txn1->GetTransactionId());

  std::thread t0([&] {
    // Lock and sleep
    bool res = lock_mgr.LockTable(txn0, LockManager::LockMode::INTENTION_EXCLUSIVE, toid);
    EXPECT_EQ(true, res);
    res = lock_mgr.LockRow(txn0, LockManager::LockMode::EXCLUSIVE, toid, rid0);
    EXPECT_EQ(true, res);
    EXPECT_EQ(TransactionState::GROWING, txn1->GetState());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // This will block
    res = lock_mgr.LockRow(txn0, LockManager::LockMode::EXCLUSIVE, toid, rid1);
    EXPECT_EQ(true, res);

    lock_mgr.UnlockRow(txn0, toid, rid1);
    lock_mgr.UnlockRow(txn0, toid, rid0);
    lock_mgr.UnlockTable(txn0, toid);

    txn_mgr.Commit(txn0);
    EXPECT_EQ(TransactionState::COMMITTED, txn0->GetState());
  });

  std::thread t1([&] {
    // Sleep so T0 can take necessary locks
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    bool res = lock_mgr.LockTable(txn1, LockManager::LockMode::INTENTION_EXCLUSIVE, toid);
    EXPECT_EQ(res, true);

    res = lock_mgr.LockRow(txn1, LockManager::LockMode::EXCLUSIVE, toid, rid1);
    EXPECT_EQ(res, true);
    EXPECT_EQ(TransactionState::GROWING, txn1->GetState());

    // This will block
    res = lock_mgr.LockRow(txn1, LockManager::LockMode::EXCLUSIVE, toid, rid0);
    EXPECT_EQ(res, false);

    EXPECT_EQ(TransactionState::ABORTED, txn1->GetState());
    txn_mgr.Abort(txn1);
  });

  // Sleep for enough time to break cycle
  std::this_thread::sleep_for(cycle_detection_interval * 2);

  t0.join();
  t1.join();

  delete txn0;
  delete txn1;
}

TEST(LockManagerDeadlockDetectionTest, DeadlockDetectionTest1) {
  LockManager lock_mgr{};
  TransactionManager txn_mgr{&lock_mgr};
  lock_mgr.txn_manager_ = &txn_mgr;
  lock_mgr.StartDeadlockDetection();

  table_oid_t toid{0};
  RID rid0{0, 0};
  RID rid1{1, 1};
  RID rid2{2, 2};
  auto *txn0 = txn_mgr.Begin();
  auto *txn1 = txn_mgr.Begin();
  auto *txn2 = txn_mgr.Begin();

  EXPECT_EQ(0, txn0->GetTransactionId());
  EXPECT_EQ(1, txn1->GetTransactionId());
  EXPECT_EQ(2, txn2->GetTransactionId());

  std::thread t0([&] {
    // T0 first takes intention lock on table
    bool res = lock_mgr.LockTable(txn0, LockManager::LockMode::INTENTION_EXCLUSIVE, toid);
    EXPECT_EQ(true, res);

    // Xlocks on rid0
    res = lock_mgr.LockRow(txn0, LockManager::LockMode::EXCLUSIVE, toid, rid0);
    EXPECT_EQ(true, res);
    EXPECT_EQ(TransactionState::GROWING, txn1->GetState());

    // sleeps to wait for T1 and T2 take their locks
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    // lock on r1, which is currently locked by t2
    res = lock_mgr.LockRow(txn0, LockManager::LockMode::EXCLUSIVE, toid, rid1);
    EXPECT_EQ(true, res);

    lock_mgr.UnlockRow(txn0, toid, rid1);
    lock_mgr.UnlockRow(txn0, toid, rid0);
    lock_mgr.UnlockTable(txn0, toid);

    txn_mgr.Commit(txn0);
    EXPECT_EQ(TransactionState::COMMITTED, txn0->GetState());
  });

  std::thread t1([&] {
    // sleep so T0 can take necessary locks
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // takes intention lock on table -> this should succeed
    bool res = lock_mgr.LockTable(txn1, LockManager::LockMode::INTENTION_EXCLUSIVE, toid);
    EXPECT_EQ(res, true);

    // locks rid1, a different row on table
    res = lock_mgr.LockRow(txn1, LockManager::LockMode::EXCLUSIVE, toid, rid1);
    EXPECT_EQ(res, true);  // this too shall work
    EXPECT_EQ(TransactionState::GROWING, txn1->GetState());

    // sleep before it abors, so T2 can take necessary locks
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    // lock on rid2, which is currently locked by T2
    res = lock_mgr.LockRow(txn1, LockManager::LockMode::EXCLUSIVE, toid, rid2);
    EXPECT_EQ(true, res);

    txn_mgr.Abort(txn1);
  });

  std::thread t2([&] {
    // Sleep so T0 and T1 can take necessary locks
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // also intention locks on table
    bool res = lock_mgr.LockTable(txn2, LockManager::LockMode::INTENTION_EXCLUSIVE, toid);
    EXPECT_EQ(res, true);

    // locks on rid2 -> this should succeed
    res = lock_mgr.LockRow(txn2, LockManager::LockMode::EXCLUSIVE, toid, rid2);
    EXPECT_EQ(res, true);
    EXPECT_EQ(TransactionState::GROWING, txn2->GetState());

    // sleep until T1 and T2 send their second request
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    // locks on rid0 -> this will block
    res = lock_mgr.LockRow(txn2, LockManager::LockMode::EXCLUSIVE, toid, rid0);
    EXPECT_EQ(res, false);

    EXPECT_EQ(TransactionState::ABORTED, txn2->GetState());
    txn_mgr.Abort(txn2);
  });

  // Sleep for enough time to break cycle
  std::this_thread::sleep_for(cycle_detection_interval * 2);

  t0.join();
  t1.join();
  t2.join();

  delete txn0;
  delete txn1;
  delete txn2;
}
}  // namespace bustub
