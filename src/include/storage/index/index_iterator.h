//===----------------------------------------------------------------------===//
//
//                         CMU-DB Project (15-445/645)
//                         ***DO NO SHARE PUBLICLY***
//
// Identification: src/include/index/index_iterator.h
//
// Copyright (c) 2018, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
/**
 * index_iterator.h
 * For range scan of b+ tree
 */
#pragma once
#include "storage/page/b_plus_tree_leaf_page.h"

namespace bustub {

#define INDEXITERATOR_TYPE IndexIterator<KeyType, ValueType, KeyComparator>

INDEX_TEMPLATE_ARGUMENTS
class IndexIterator {
 public:
  // you may define your own constructor based on your member variables
  explicit IndexIterator(BufferPoolManager *bpm, page_id_t current_pid, int index);
  ~IndexIterator();  // NOLINT

  auto IsEnd() -> bool;

  auto operator*() -> const MappingType &;

  auto operator++() -> IndexIterator &;

  auto operator==(const IndexIterator &itr) const -> bool {
    return (itr.bpm_ == bpm_ && itr.cur_page_id_ == cur_page_id_ && itr.index_ == index_);
  }

  auto operator!=(const IndexIterator &itr) const -> bool { return !((*this) == itr); }

 private:
  // add your own private member variables here
  BufferPoolManager *bpm_;
  page_id_t cur_page_id_{INVALID_PAGE_ID};
  int index_{0};
};

}  // namespace bustub
