//===----------------------------------------------------------------------===//
//
//                         CMU-DB Project (15-445/645)
//                         ***DO NO SHARE PUBLICLY***
//
// Identification: src/page/b_plus_tree_leaf_page.cpp
//
// Copyright (c) 2018, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <sstream>

#include "common/config.h"
#include "common/exception.h"
#include "common/rid.h"
#include "storage/page/b_plus_tree_leaf_page.h"
#include "storage/page/b_plus_tree_page.h"
#include "type/varlen_type.h"

namespace bustub {

/*****************************************************************************
 * HELPER METHODS AND UTILITIES
 *****************************************************************************/

/**
 * Init method after creating a new leaf page
 * Including set page type, set current size to zero, set next page id and set max size
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::Init(int max_size) {
  SetPageType(IndexPageType::LEAF_PAGE);
  SetSize(0);
  SetMaxSize(max_size);
  SetNextPageId(INVALID_PAGE_ID);
}

/**
 * Helper methods to set/get next page id
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::GetNextPageId() const -> page_id_t { return next_page_id_; }

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::SetNextPageId(page_id_t next_page_id) { next_page_id_ = next_page_id; }

/*
 * Helper method to find and return the key associated with input "index"(a.k.a
 * array offset)
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::KeyAt(int index) const -> KeyType {
  // replace with your own code
  KeyType key = array_[index].first;
  return key;
}
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::ValueAt(int index) const -> ValueType { return array_[index].second; }

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::KeyValueAt(int index) const -> const MappingType & { return array_[index]; }

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::Binarysearch(const KeyType &key, KeyComparator comparator) const -> int {
  int left = 0;
  int right = GetSize() - 1;
  int idx = GetSize();
  while (left <= right) {
    int mid = left + (right - left) / 2;
    int comparation = comparator(key, KeyAt(mid));
    if (comparation <= 0) {
      idx = mid;
      right = mid - 1;
    } else {
      left = mid + 1;
    }
  }
  return idx;
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::Insert(KeyType key, ValueType value, int index) {
  IncreaseSize(1);
  for (int i = GetSize() - 1; i > index; --i) {
    array_[i] = array_[i - 1];
  }
  array_[index] = MappingType(key, value);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::Remove(int index) {
  for (int i = index; i < GetSize() - 1; ++i) {
    array_[i] = array_[i + 1];
  }
  IncreaseSize(-1);
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::Split(BPlusTreeLeafPage *leaf, page_id_t leaf_id) -> KeyType {
  int size = GetSize();
  int mid = size >> 1;
  KeyType mid_key = KeyAt(mid);
  for (int i = mid; i < size; ++i) {
    leaf->Insert(KeyAt(i), ValueAt(i), i - mid);
  }
  SetSize(mid);
  leaf->SetNextPageId(next_page_id_);
  next_page_id_ = leaf_id;
  return mid_key;
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::Merge(BPlusTreeLeafPage *neighbor_leaf) -> KeyType {
  auto size = GetSize();
  auto neighbor_leaf_size = neighbor_leaf->GetSize();
  auto remove = neighbor_leaf->KeyAt(0);
  IncreaseSize(neighbor_leaf_size);
  for (int i = size; i < GetSize(); i++) {
    array_[i] = neighbor_leaf->array_[i - size];
  }
  next_page_id_ = neighbor_leaf->GetNextPageId();
  // neighbor_leaf->SetNextPageId(INVALID_PAGE_ID);
  neighbor_leaf->SetSize(0);
  return remove;
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::Borrow(BPlusTreeLeafPage *neighbor_leaf, bool is_left) -> KeyType {
  KeyType new_key{};
  if (is_left) {
    new_key = neighbor_leaf->KeyAt(neighbor_leaf->GetSize() - 1);
    Insert(new_key, neighbor_leaf->ValueAt(neighbor_leaf->GetSize() - 1), 0);
    neighbor_leaf->Remove(neighbor_leaf->GetSize() - 1);
  } else {
    Insert(neighbor_leaf->KeyAt(0), neighbor_leaf->ValueAt(0), GetSize());
    neighbor_leaf->Remove(0);
    new_key = neighbor_leaf->KeyAt(0);
  }
  return new_key;
}

template class BPlusTreeLeafPage<GenericKey<4>, RID, GenericComparator<4>>;
template class BPlusTreeLeafPage<GenericKey<8>, RID, GenericComparator<8>>;
template class BPlusTreeLeafPage<GenericKey<16>, RID, GenericComparator<16>>;
template class BPlusTreeLeafPage<GenericKey<32>, RID, GenericComparator<32>>;
template class BPlusTreeLeafPage<GenericKey<64>, RID, GenericComparator<64>>;
}  // namespace bustub
