/**
 * index_iterator.cpp
 */
#include <cassert>

#include "storage/index/index_iterator.h"

namespace bustub {

/*
 * NOTE: you can change the destructor/constructor method here
 * set your own input parameters
 */
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE::IndexIterator(BufferPoolManager *bpm, page_id_t current_pid, int index)
    : bpm_(bpm), cur_page_id_(current_pid), index_(index) {}

INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE::~IndexIterator(){};  // NOLINT

INDEX_TEMPLATE_ARGUMENTS
auto INDEXITERATOR_TYPE::IsEnd() -> bool {
  if (cur_page_id_ == INVALID_PAGE_ID) {
    return true;
  }
  ReadPageGuard read_guard = bpm_->FetchPageRead(cur_page_id_);
  auto page = read_guard.As<BPlusTreePage>();
  auto leaf_page = reinterpret_cast<const BPlusTreeLeafPage<KeyType, ValueType, KeyComparator> *>(page);
  return (leaf_page->GetNextPageId() == INVALID_PAGE_ID) && (index_ == leaf_page->GetSize());
}

INDEX_TEMPLATE_ARGUMENTS
auto INDEXITERATOR_TYPE::operator*() -> const MappingType & {
  ReadPageGuard read_guard = bpm_->FetchPageRead(cur_page_id_);
  auto page = read_guard.As<BPlusTreePage>();
  auto leaf_page = reinterpret_cast<const BPlusTreeLeafPage<KeyType, ValueType, KeyComparator> *>(page);
  return leaf_page->KeyValueAt(index_);
}

INDEX_TEMPLATE_ARGUMENTS
auto INDEXITERATOR_TYPE::operator++() -> INDEXITERATOR_TYPE & {
  ReadPageGuard read_guard = bpm_->FetchPageRead(cur_page_id_);
  auto page = read_guard.As<BPlusTreePage>();
  auto leaf_page = reinterpret_cast<const BPlusTreeLeafPage<KeyType, ValueType, KeyComparator> *>(page);
  if (index_ == leaf_page->GetSize() - 1 && leaf_page->GetNextPageId() != INVALID_PAGE_ID) {
    cur_page_id_ = leaf_page->GetNextPageId();
    index_ = 0;
  } else {
    index_++;
  }
  return *this;
}

template class IndexIterator<GenericKey<4>, RID, GenericComparator<4>>;

template class IndexIterator<GenericKey<8>, RID, GenericComparator<8>>;

template class IndexIterator<GenericKey<16>, RID, GenericComparator<16>>;

template class IndexIterator<GenericKey<32>, RID, GenericComparator<32>>;

template class IndexIterator<GenericKey<64>, RID, GenericComparator<64>>;

}  // namespace bustub
