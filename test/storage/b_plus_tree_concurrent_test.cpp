//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// b_plus_tree_concurrent_test.cpp
//
// Identification: test/storage/b_plus_tree_concurrent_test.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <chrono>  // NOLINT
#include <cstdio>
#include <functional>
#include <random>
#include <thread>  // NOLINT

#include "buffer/buffer_pool_manager.h"
#include "gtest/gtest.h"
#include "storage/disk/disk_manager_memory.h"
#include "storage/index/b_plus_tree.h"
#include "test_util.h"  // NOLINT

namespace bustub {

using bustub::DiskManagerUnlimitedMemory;

// helper function to launch multiple threads
template <typename... Args>
void LaunchParallelTest(uint64_t num_threads, Args &&...args) {
  std::vector<std::thread> thread_group;

  // Launch a group of threads
  for (uint64_t thread_itr = 0; thread_itr < num_threads; ++thread_itr) {
    thread_group.push_back(std::thread(args..., thread_itr));
  }

  // Join the threads with the main thread
  for (uint64_t thread_itr = 0; thread_itr < num_threads; ++thread_itr) {
    thread_group[thread_itr].join();
  }
}

// helper function to insert
void InsertHelper(BPlusTree<GenericKey<8>, RID, GenericComparator<8>> *tree, const std::vector<int64_t> &keys,
                  __attribute__((unused)) uint64_t thread_itr = 0) {
  GenericKey<8> index_key;
  RID rid;
  // create transaction
  auto *transaction = new Transaction(0);
  for (auto key : keys) {
    int64_t value = key & 0xFFFFFFFF;
    rid.Set(static_cast<int32_t>(key >> 32), value);
    index_key.SetFromInteger(key);
    tree->Insert(index_key, rid, transaction);
  }
  delete transaction;
}

// helper function to seperate insert
void InsertHelperSplit(BPlusTree<GenericKey<8>, RID, GenericComparator<8>> *tree, const std::vector<int64_t> &keys,
                       int total_threads, __attribute__((unused)) uint64_t thread_itr) {
  GenericKey<8> index_key;
  RID rid;
  // create transaction
  auto *transaction = new Transaction(0);
  for (auto key : keys) {
    if (static_cast<uint64_t>(key) % total_threads == thread_itr) {
      int64_t value = key & 0xFFFFFFFF;
      rid.Set(static_cast<int32_t>(key >> 32), value);
      index_key.SetFromInteger(key);
      tree->Insert(index_key, rid, transaction);
    }
  }
  delete transaction;
}

// helper function to delete
void DeleteHelper(BPlusTree<GenericKey<8>, RID, GenericComparator<8>> *tree, const std::vector<int64_t> &remove_keys,
                  __attribute__((unused)) uint64_t thread_itr = 0) {
  GenericKey<8> index_key;
  // create transaction
  auto *transaction = new Transaction(0);
  for (auto key : remove_keys) {
    index_key.SetFromInteger(key);
    tree->Remove(index_key, transaction);
  }
  delete transaction;
}

// helper function to seperate delete
void DeleteHelperSplit(BPlusTree<GenericKey<8>, RID, GenericComparator<8>> *tree,
                       const std::vector<int64_t> &remove_keys, int total_threads,
                       __attribute__((unused)) uint64_t thread_itr) {
  GenericKey<8> index_key;
  // create transaction
  auto *transaction = new Transaction(0);
  for (auto key : remove_keys) {
    if (static_cast<uint64_t>(key) % total_threads == thread_itr) {
      index_key.SetFromInteger(key);
      tree->Remove(index_key, transaction);
    }
  }
  delete transaction;
}

void LookupHelper(BPlusTree<GenericKey<8>, RID, GenericComparator<8>> *tree, const std::vector<int64_t> &keys,
                  uint64_t tid, __attribute__((unused)) uint64_t thread_itr = 0) {
  auto *transaction = new Transaction(static_cast<txn_id_t>(tid));
  GenericKey<8> index_key;
  RID rid;
  for (auto key : keys) {
    int64_t value = key & 0xFFFFFFFF;
    rid.Set(static_cast<int32_t>(key >> 32), value);
    index_key.SetFromInteger(key);
    std::vector<RID> result;
    bool res = tree->GetValue(index_key, &result, transaction);
    ASSERT_EQ(res, true);
    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(result[0], rid);
  }
  delete transaction;
}

TEST(BPlusTreeConcurrentTest, InsertTest1) {
  // create KeyComparator and index schema
  auto key_schema = ParseCreateStatement("a bigint");
  GenericComparator<8> comparator(key_schema.get());

  auto disk_manager = std::make_unique<DiskManagerUnlimitedMemory>();
  auto *bpm = new BufferPoolManager(50, disk_manager.get());
  // create and fetch header_page
  page_id_t page_id;
  auto header_page = bpm->NewPage(&page_id);
  // create b+ tree
  BPlusTree<GenericKey<8>, RID, GenericComparator<8>> tree("foo_pk", header_page->GetPageId(), bpm, comparator);
  // keys to Insert
  std::vector<int64_t> keys;
  int64_t scale_factor = 100;
  for (int64_t key = 1; key < scale_factor; key++) {
    keys.push_back(key);
  }
  LaunchParallelTest(2, InsertHelper, &tree, keys);

  std::vector<RID> rids;
  GenericKey<8> index_key;
  for (auto key : keys) {
    rids.clear();
    index_key.SetFromInteger(key);
    tree.GetValue(index_key, &rids);
    EXPECT_EQ(rids.size(), 1);

    int64_t value = key & 0xFFFFFFFF;
    EXPECT_EQ(rids[0].GetSlotNum(), value);
  }

  int64_t start_key = 1;
  int64_t current_key = start_key;
  index_key.SetFromInteger(start_key);
  for (auto iterator = tree.Begin(index_key); iterator != tree.End(); ++iterator) {
    auto location = (*iterator).second;
    EXPECT_EQ(location.GetPageId(), 0);
    EXPECT_EQ(location.GetSlotNum(), current_key);
    current_key = current_key + 1;
  }

  EXPECT_EQ(current_key, keys.size() + 1);

  bpm->UnpinPage(HEADER_PAGE_ID, true);
  delete bpm;
}

TEST(BPlusTreeConcurrentTest, InsertTest2) {
  // create KeyComparator and index schema
  auto key_schema = ParseCreateStatement("a bigint");
  GenericComparator<8> comparator(key_schema.get());
  auto disk_manager = std::make_unique<DiskManagerUnlimitedMemory>();
  auto *bpm = new BufferPoolManager(50, disk_manager.get());
  // create and fetch header_page
  page_id_t page_id;
  auto header_page = bpm->NewPage(&page_id);
  // create b+ tree
  BPlusTree<GenericKey<8>, RID, GenericComparator<8>> tree("foo_pk", header_page->GetPageId(), bpm, comparator);
  // keys to Insert
  std::vector<int64_t> keys;
  int64_t scale_factor = 100;
  for (int64_t key = 1; key < scale_factor; key++) {
    keys.push_back(key);
  }
  LaunchParallelTest(2, InsertHelperSplit, &tree, keys, 2);

  std::vector<RID> rids;
  GenericKey<8> index_key;
  for (auto key : keys) {
    rids.clear();
    index_key.SetFromInteger(key);
    tree.GetValue(index_key, &rids);
    EXPECT_EQ(rids.size(), 1);

    int64_t value = key & 0xFFFFFFFF;
    EXPECT_EQ(rids[0].GetSlotNum(), value);
  }

  int64_t start_key = 1;
  int64_t current_key = start_key;
  index_key.SetFromInteger(start_key);
  for (auto iterator = tree.Begin(index_key); iterator != tree.End(); ++iterator) {
    auto location = (*iterator).second;
    EXPECT_EQ(location.GetPageId(), 0);
    EXPECT_EQ(location.GetSlotNum(), current_key);
    current_key = current_key + 1;
  }

  EXPECT_EQ(current_key, keys.size() + 1);

  bpm->UnpinPage(HEADER_PAGE_ID, true);
  delete bpm;
}

TEST(BPlusTreeConcurrentTest, DeleteTest1) {
  // create KeyComparator and index schema
  auto key_schema = ParseCreateStatement("a bigint");
  GenericComparator<8> comparator(key_schema.get());

  auto disk_manager = std::make_unique<DiskManagerUnlimitedMemory>();
  auto *bpm = new BufferPoolManager(50, disk_manager.get());

  GenericKey<8> index_key;
  // create and fetch header_page
  page_id_t page_id;
  auto header_page = bpm->NewPage(&page_id);
  // create b+ tree
  BPlusTree<GenericKey<8>, RID, GenericComparator<8>> tree("foo_pk", header_page->GetPageId(), bpm, comparator);
  // sequential insert
  std::vector<int64_t> keys = {1, 2, 3, 4, 5};
  InsertHelper(&tree, keys);

  std::vector<int64_t> remove_keys = {1, 5, 3, 4};
  LaunchParallelTest(2, DeleteHelper, &tree, remove_keys);

  int64_t start_key = 2;
  int64_t current_key = start_key;
  int64_t size = 0;
  index_key.SetFromInteger(start_key);
  for (auto iterator = tree.Begin(index_key); iterator != tree.End(); ++iterator) {
    auto location = (*iterator).second;
    EXPECT_EQ(location.GetPageId(), 0);
    EXPECT_EQ(location.GetSlotNum(), current_key);
    current_key = current_key + 1;
    size = size + 1;
  }

  EXPECT_EQ(size, 1);

  bpm->UnpinPage(HEADER_PAGE_ID, true);
  delete bpm;
}

TEST(BPlusTreeConcurrentTest, DeleteTest2) {
  // create KeyComparator and index schema
  auto key_schema = ParseCreateStatement("a bigint");
  GenericComparator<8> comparator(key_schema.get());

  auto disk_manager = std::make_unique<DiskManagerUnlimitedMemory>();
  auto *bpm = new BufferPoolManager(50, disk_manager.get());
  GenericKey<8> index_key;
  // create and fetch header_page
  page_id_t page_id;
  auto header_page = bpm->NewPage(&page_id);
  // create b+ tree
  BPlusTree<GenericKey<8>, RID, GenericComparator<8>> tree("foo_pk", header_page->GetPageId(), bpm, comparator);

  // sequential insert
  std::vector<int64_t> keys = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  InsertHelper(&tree, keys);

  std::vector<int64_t> remove_keys = {1, 4, 3, 2, 5, 6};
  LaunchParallelTest(2, DeleteHelperSplit, &tree, remove_keys, 2);

  int64_t start_key = 7;
  int64_t current_key = start_key;
  int64_t size = 0;
  index_key.SetFromInteger(start_key);
  for (auto iterator = tree.Begin(index_key); iterator != tree.End(); ++iterator) {
    auto location = (*iterator).second;
    EXPECT_EQ(location.GetPageId(), 0);
    EXPECT_EQ(location.GetSlotNum(), current_key);
    current_key = current_key + 1;
    size = size + 1;
  }

  EXPECT_EQ(size, 4);

  bpm->UnpinPage(HEADER_PAGE_ID, true);
  delete bpm;
}

TEST(BPlusTreeConcurrentTest, MixTest1) {
  // create KeyComparator and index schema
  auto key_schema = ParseCreateStatement("a bigint");
  GenericComparator<8> comparator(key_schema.get());

  auto disk_manager = std::make_unique<DiskManagerUnlimitedMemory>();
  auto *bpm = new BufferPoolManager(50, disk_manager.get());

  // create and fetch header_page
  page_id_t page_id;
  auto header_page = bpm->NewPage(&page_id);
  // create b+ tree
  BPlusTree<GenericKey<8>, RID, GenericComparator<8>> tree("foo_pk", header_page->GetPageId(), bpm, comparator);
  GenericKey<8> index_key;
  // first, populate index
  std::vector<int64_t> keys = {1, 2, 3, 4, 5};
  InsertHelper(&tree, keys);

  // concurrent insert
  keys.clear();
  for (int i = 6; i <= 10; i++) {
    keys.push_back(i);
  }
  LaunchParallelTest(1, InsertHelper, &tree, keys);
  // concurrent delete
  std::vector<int64_t> remove_keys = {1, 4, 3, 5, 6};
  LaunchParallelTest(1, DeleteHelper, &tree, remove_keys);

  int64_t start_key = 2;
  int64_t size = 0;
  index_key.SetFromInteger(start_key);
  for (auto iterator = tree.Begin(index_key); iterator != tree.End(); ++iterator) {
    size = size + 1;
  }

  EXPECT_EQ(size, 5);

  bpm->UnpinPage(HEADER_PAGE_ID, true);
  delete bpm;
}

TEST(BPlusTreeConcurrentTest, MixTest2) {
  // create KeyComparator and index schema
  auto key_schema = ParseCreateStatement("a bigint");
  GenericComparator<8> comparator(key_schema.get());

  auto disk_manager = std::make_unique<DiskManagerUnlimitedMemory>();
  auto *bpm = new BufferPoolManager(50, disk_manager.get());

  // create and fetch header_page
  page_id_t page_id;
  auto *header_page = bpm->NewPage(&page_id);
  (void)header_page;

  // create b+ tree
  BPlusTree<GenericKey<8>, RID, GenericComparator<8>> tree("foo_pk", page_id, bpm, comparator);

  // Add perserved_keys
  std::vector<int64_t> perserved_keys;
  std::vector<int64_t> dynamic_keys;
  int64_t total_keys = 1000;
  int64_t sieve = 5;
  for (int64_t i = 1; i <= total_keys; i++) {
    if (i % sieve == 0) {
      perserved_keys.push_back(i);
    } else {
      dynamic_keys.push_back(i);
    }
  }
  InsertHelper(&tree, perserved_keys, 1);
  // Check there are 1000 keys in there
  size_t size;

  auto insert_task = [&](int tid) { InsertHelper(&tree, dynamic_keys, tid); };
  auto delete_task = [&](int tid) { DeleteHelper(&tree, dynamic_keys, tid); };
  auto lookup_task = [&](int tid) { LookupHelper(&tree, perserved_keys, tid); };

  std::vector<std::thread> threads;
  std::vector<std::function<void(int)>> tasks;
  tasks.emplace_back(insert_task);
  tasks.emplace_back(delete_task);
  tasks.emplace_back(lookup_task);

  size_t num_threads = 9;
  for (size_t i = 0; i < num_threads; i++) {
    threads.emplace_back(tasks[i % tasks.size()], i);
  }
  for (size_t i = 0; i < num_threads; i++) {
    threads[i].join();
  }

  // Check all reserved keys exist
  size = 0;

  for (auto iter = tree.Begin(); iter != tree.End(); ++iter) {
    const auto &pair = *iter;
    if ((pair.first).ToString() % sieve == 0) {
      size++;
    }
  }

  ASSERT_EQ(size, perserved_keys.size());

  bpm->UnpinPage(HEADER_PAGE_ID, true);
  delete bpm;
}

TEST(BPlusTreeConcurrentTest, MixTest3) {
  // test from github
  // create KeyComparator and index schema
  auto key_schema = ParseCreateStatement("a bigint");
  GenericComparator<8> comparator(key_schema.get());

  auto disk_manager = new DiskManager("test.db");
  auto *bpm = new BufferPoolManager(50, disk_manager);

  // create and fetch header_page
  page_id_t page_id;
  auto header_page = bpm->NewPage(&page_id);
  (void)header_page;

  // create b+ tree
  BPlusTree<GenericKey<8>, RID, GenericComparator<8>> tree("foo_pk", page_id, bpm, comparator);
  GenericKey<8> index_key;
  // RID rid;

  // first, populate index
  std::vector<int64_t> keys;
  std::vector<int64_t> all_deleted;
  for (int i = 1; i <= 1000; ++i) {
    all_deleted.push_back(i);
    keys.push_back(i + 1000);
  }
  // keys1: 0,2,4...
  // keys2: 1,3,5...
  // keys: 100 ~ 200

  // concurrent insert
  LaunchParallelTest(4, InsertHelperSplit, &tree, all_deleted, 4);

  // concurrent insert and delete
  std::thread t0([&]() { InsertHelper(&tree, keys); });
  LaunchParallelTest(4, DeleteHelperSplit, &tree, all_deleted, 4);

  t0.join();

  std::vector<RID> rids;
  for (auto key : all_deleted) {
    rids.clear();
    index_key.SetFromInteger(key);
    auto res = tree.GetValue(index_key, &rids);
    EXPECT_EQ(false, res);
  }

  int64_t current_key = 1001;
  int64_t size = 0;
  index_key.SetFromInteger(current_key);
  for (auto iterator = tree.Begin(index_key); iterator != tree.End(); ++iterator) {
    auto location = (*iterator).second;
    EXPECT_EQ(location.GetPageId(), 0);
    EXPECT_EQ(location.GetSlotNum(), current_key);
    current_key = current_key + 1;
    size = size + 1;
  }

  EXPECT_EQ(size, 1000);
  bpm->UnpinPage(HEADER_PAGE_ID, true);
  delete disk_manager;
  delete bpm;
  remove("test.db");
  remove("test.log");
}

// helper function to insert and get
void InsertAndGetHelper(BPlusTree<GenericKey<8>, RID, GenericComparator<8>> &tree, const std::vector<int64_t> &keys,
                        __attribute__((unused)) uint64_t thread_itr = 0) {
  GenericKey<8> index_key;
  RID rid;
  // create transaction
  auto *transaction = new Transaction(0);
  for (auto key : keys) {
    int64_t value = key & 0xFFFFFFFF;
    rid.Set(static_cast<int32_t>(key >> 32), value);
    index_key.SetFromInteger(key);
    tree.Insert(index_key, rid, transaction);
    std::vector<RID> rids;
    bool get_suc = tree.GetValue(index_key, &rids, transaction);
    EXPECT_EQ(get_suc, true);
    EXPECT_EQ(rids[0].GetSlotNum(), value);
  }

  delete transaction;
}

void DeleteAndGetHelper(BPlusTree<GenericKey<8>, RID, GenericComparator<8>> &tree,
                        const std::vector<int64_t> &remove_keys, __attribute__((unused)) uint64_t thread_itr = 0) {
  GenericKey<8> index_key;
  // create transaction
  auto *transaction = new Transaction(0);
  for (auto key : remove_keys) {
    index_key.SetFromInteger(key);
    tree.Remove(index_key, transaction);
    std::vector<RID> rids;
    bool get_suc = tree.GetValue(index_key, &rids, transaction);
    EXPECT_EQ(get_suc, false);
  }
  delete transaction;
}

TEST(BPlusTreeConcurrentTest, MixTest4) {
  // create KeyComparator and index schema
  auto key_schema = ParseCreateStatement("a bigint");
  GenericComparator<8> comparator(key_schema.get());

  auto disk_manager = new DiskManager("test.db");
  auto *bpm = new BufferPoolManager(50, disk_manager);

  // create and fetch header_page
  page_id_t page_id;
  auto header_page = bpm->NewPage(&page_id);
  (void)header_page;

  // create b+ tree
  BPlusTree<GenericKey<8>, RID, GenericComparator<8>> tree("foo_pk", page_id, bpm, comparator);
  GenericKey<8> index_key;
  RID rid;

  // first, populate index
  std::vector<int64_t> keys;
  std::vector<int64_t> keys2;
  std::vector<int64_t> deleted1;
  std::vector<int64_t> deleted2;
  int scale = 10000;
  for (int i = 1; i <= scale; ++i) {
    keys.push_back(i);
  }
  std::shuffle(keys.begin(), keys.end(), std::mt19937(std::random_device()()));
  for (int i = 1; i <= scale / 4; ++i) {
    deleted1.push_back(keys.back());
    keys.pop_back();
  }
  for (int i = 1; i <= scale / 4; ++i) {
    deleted2.push_back(keys.back());
    keys.pop_back();
  }
  for (int i = 1; i <= scale / 4; ++i) {
    keys2.push_back(keys.back());
    keys.pop_back();
  }
  //  for (const auto i: keys)
  //    cout << i << ' ';
  //  cout<<endl;
  //  for (const auto i: keys2)
  //    cout << i << ' ';
  //  cout<<endl;
  //  for (const auto i: all_deleted)
  //    cout << i << ' ';
  //  cout<<endl;

  // concurrent insert
  LaunchParallelTest(4, InsertHelperSplit, (&tree), std::ref(deleted1), 4);
  LaunchParallelTest(4, InsertHelperSplit, (&tree), std::ref(deleted2), 4);
  // concurrent insert and delete
  std::thread t0(InsertAndGetHelper, std::ref(tree), keys, 0);
  std::thread t1(InsertAndGetHelper, std::ref(tree), keys2, 0);
  std::thread t2(DeleteAndGetHelper, std::ref(tree), deleted1, 0);
  std::thread t3(DeleteAndGetHelper, std::ref(tree), deleted2, 0);

  // std::thread t2(IterateHelper, std::ref(tree));
  // LaunchParallelTest(4, DeleteHelperSplit, std::ref(tree), std::ref(all_deleted), 4);

  t0.join();
  t1.join();
  t2.join();
  t3.join();
  std::vector<RID> rids;
  for (auto key : deleted1) {
    rids.clear();
    index_key.SetFromInteger(key);
    auto res = tree.GetValue(index_key, &rids);
    EXPECT_EQ(false, res);
  }
  for (auto key : deleted2) {
    rids.clear();
    index_key.SetFromInteger(key);
    auto res = tree.GetValue(index_key, &rids);
    EXPECT_EQ(false, res);
  }

  int64_t current_key = 0;
  int64_t size = -1;
  index_key.SetFromInteger(current_key);
  int64_t value = current_key & 0xFFFFFFFF;
  rid.Set(static_cast<int32_t>(current_key), value);
  tree.Insert(index_key, rid);
  for (auto iterator = tree.Begin(index_key); iterator != tree.End(); ++iterator) {
    size = size + 1;
  }

  EXPECT_EQ(size, scale / 2);
  bpm->UnpinPage(HEADER_PAGE_ID, true);
  delete disk_manager;
  delete bpm;
  remove("test.db");
  remove("test.log");
}

}  // namespace bustub
