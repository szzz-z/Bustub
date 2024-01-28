//===----------------------------------------------------------------------===//
//
//                         CMU-DB Project (15-445/645)
//                         ***DO NO SHARE PUBLICLY***
//
// Identification: src/page/b_plus_tree_internal_page.cpp
//
// Copyright (c) 2018, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <utility>

#include "common/config.h"
#include "storage/page/b_plus_tree_internal_page.h"
#include "storage/page/b_plus_tree_page.h"

namespace bustub {
/*****************************************************************************
 * HELPER METHODS AND UTILITIES
 *****************************************************************************/
/*
 * Init method after creating a new internal page
 * Including set page type, set current size, and set max page size
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Init(int max_size) {
  SetPageType(IndexPageType::INTERNAL_PAGE);
  SetSize(0);
  SetMaxSize(max_size);
}
/*
 * Helper method to get/set the key associated with input "index"(a.k.a
 * array offset)
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::KeyAt(int index) const -> KeyType {
  // replace with your own code
  KeyType key = array_[index].first;
  return key;
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::SetKeyAt(int index, const KeyType &key) { array_[index].first = key; }

/*
 * Helper method to get the value associated with input "index"(a.k.a array
 * offset)
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::ValueAt(int index) const -> ValueType { return array_[index].second; }

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::SetValueAt(int index, const ValueType &value) { array_[index].second = value; }

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::Binarysearch(const KeyType &key, KeyComparator comparator) const -> int {
  int idx = 0;
  if (GetSize() > 1) {
    int l = 1;
    int r = GetSize() - 1;
    while (l <= r) {
      int mid = l + (r - l) / 2;
      int comparation = mid == 0 ? 1 : comparator(key, KeyAt(mid));
      if (comparation < 0) {
        r = mid - 1;
      } else {
        idx = mid;
        l = mid + 1;
      }
    }
  }
  return idx;
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Insert(KeyType key, ValueType value, int index) {
  for (int i = GetSize(); i > index; --i) {
    array_[i] = array_[i - 1];
  }
  array_[index] = MappingType(key, value);
  IncreaseSize(1);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Remove(int index) {
  for (int i = index; i < GetSize() - 1; ++i) {
    array_[i] = array_[i + 1];
  }
  IncreaseSize(-1);
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::Split(BPlusTreeInternalPage *neighbor) -> KeyType {
  int size = GetSize();
  int mid = size >> 1;
  auto mid_key = KeyAt(mid);
  for (int i = mid; i < size; ++i) {
    neighbor->Insert(KeyAt(i), ValueAt(i), i - mid);
  }
  SetSize(mid);
  return mid_key;
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::Merge(BPlusTreeInternalPage *neighbor_node) -> KeyType {
  auto size = GetSize();
  auto neighbor_node_size = neighbor_node->GetSize();
  auto remove = neighbor_node->KeyAt(0);
  IncreaseSize(neighbor_node_size);
  for (int i = size; i < GetSize(); i++) {
    this->array_[i] = neighbor_node->array_[i - size];
  }
  neighbor_node->SetSize(0);
  return remove;
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::GetInternalNeighbors(int index) const -> std::pair<page_id_t, page_id_t> {
  auto left = index > 0 ? ValueAt(index - 1) : INVALID_PAGE_ID;
  auto righe = index < GetSize() - 1 ? ValueAt(index + 1) : INVALID_PAGE_ID;
  return {left, righe};
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::Borrow(BPlusTreeInternalPage *neighbor, bool is_left) -> KeyType {
  KeyType new_key;
  if (is_left) {
    new_key = neighbor->KeyAt(neighbor->GetSize() - 1);
    Insert(new_key, neighbor->ValueAt(neighbor->GetSize() - 1), 0);
    neighbor->Remove(neighbor->GetSize() - 1);
  } else {
    Insert(neighbor->KeyAt(0), neighbor->ValueAt(0), GetSize());
    neighbor->Remove(0);
    new_key = neighbor->KeyAt(0);
  }
  return new_key;
}

// valuetype for internalNode should be page id_t
template class BPlusTreeInternalPage<GenericKey<4>, page_id_t, GenericComparator<4>>;
template class BPlusTreeInternalPage<GenericKey<8>, page_id_t, GenericComparator<8>>;
template class BPlusTreeInternalPage<GenericKey<16>, page_id_t, GenericComparator<16>>;
template class BPlusTreeInternalPage<GenericKey<32>, page_id_t, GenericComparator<32>>;
template class BPlusTreeInternalPage<GenericKey<64>, page_id_t, GenericComparator<64>>;
}  // namespace bustub
