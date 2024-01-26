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
  int left = 0;
  int right = GetSize();
  while (left + 1 < right) {
    int mid = (left + right) >> 1;
    int comparation = comparator(key, KeyAt(mid));
    if (comparation < 0) {
      right = mid;
    } else if (comparation > 0) {
      left = mid;
    } else {
      left = mid;
      right = mid + 1;
      break;
    }
  }
  return left;
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Insert(KeyType key, ValueType value, int index) {
  auto size = GetSize();
  std::move_backward(array_ + index, array_ + size, array_ + size + 1);
  array_[index] = std::make_pair(key, value);
  IncreaseSize(1);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Remove(int index) {
  std::move(array_ + index + 1, array_ + GetSize(), array_ + index);
  IncreaseSize(-1);
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::Split(BPlusTreeInternalPage *neighbor) -> KeyType {
  int size = GetSize();
  int mid = size >> 1;
  KeyType mid_key = KeyAt(mid);
  std::move(array_ + mid, array_ + size, neighbor->array_);
  SetSize(mid);
  neighbor->SetSize(size - mid);
  return mid_key;
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::Merge(BPlusTreeInternalPage *neighbor_node) -> KeyType {
  auto size = GetSize();
  auto neighbor_node_size = neighbor_node->GetSize();
  auto remove = neighbor_node->KeyAt(0);
  std::move(neighbor_node->array_, neighbor_node->array_ + neighbor_node_size, array_ + size);
  IncreaseSize(neighbor_node_size);
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
  KeyType insert_key{};
  if (is_left) {
    insert_key = neighbor->KeyAt(neighbor->GetSize() - 1);
    this->Insert(insert_key, neighbor->ValueAt(neighbor->GetSize() - 1), 0);
    neighbor->Remove(neighbor->GetSize() - 1);
  } else {
    insert_key = neighbor->KeyAt(0);
    this->Insert(insert_key, neighbor->ValueAt(0), this->GetSize());
    neighbor->Remove(0);
  }
  return insert_key;
}

// valuetype for internalNode should be page id_t
template class BPlusTreeInternalPage<GenericKey<4>, page_id_t, GenericComparator<4>>;
template class BPlusTreeInternalPage<GenericKey<8>, page_id_t, GenericComparator<8>>;
template class BPlusTreeInternalPage<GenericKey<16>, page_id_t, GenericComparator<16>>;
template class BPlusTreeInternalPage<GenericKey<32>, page_id_t, GenericComparator<32>>;
template class BPlusTreeInternalPage<GenericKey<64>, page_id_t, GenericComparator<64>>;
}  // namespace bustub
