#include <optional>
#include <sstream>
#include <string>

#include "common/config.h"
#include "common/logger.h"
#include "common/rid.h"
#include "storage/index/b_plus_tree.h"
#include "storage/page/b_plus_tree_header_page.h"
#include "storage/page/b_plus_tree_page.h"
#include "storage/page/page_guard.h"

namespace bustub {

INDEX_TEMPLATE_ARGUMENTS
BPLUSTREE_TYPE::BPlusTree(std::string name, page_id_t header_page_id, BufferPoolManager *buffer_pool_manager,
                          const KeyComparator &comparator, int leaf_max_size, int internal_max_size)
    : index_name_(std::move(name)),
      bpm_(buffer_pool_manager),
      comparator_(std::move(comparator)),
      leaf_max_size_(leaf_max_size),
      internal_max_size_(internal_max_size),
      header_page_id_(header_page_id) {
  WritePageGuard guard = bpm_->FetchPageWrite(header_page_id_);
  auto root_page = guard.AsMut<BPlusTreeHeaderPage>();
  root_page->root_page_id_ = INVALID_PAGE_ID;
}

/*
 * Helper function to decide whether current b+tree is empty
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::IsEmpty() const -> bool { return GetRootPageId() == INVALID_PAGE_ID; }
/*****************************************************************************
 * SEARCH
 *****************************************************************************/
/*
 * Return the only value that associated with input key
 * This method is used for point query
 * @return : true means key exists
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::GetValue(const KeyType &key, std::vector<ValueType> *result, Transaction *txn) -> bool {
  if (IsEmpty()) {
    return false;
  }
  Context ctx;
  ctx.read_set_.emplace_back(bpm_->FetchPageRead(header_page_id_));
  ctx.root_page_id_ = ctx.read_set_.back().As<BPlusTreeHeaderPage>()->root_page_id_;

  ctx.read_set_.emplace_back(bpm_->FetchPageRead(ctx.root_page_id_));
  ctx.read_set_.pop_front();
  auto cur_page = ctx.read_set_.back().As<BPlusTreePage>();
  while (!cur_page->IsLeafPage()) {
    auto internal_page = reinterpret_cast<const InternalPage *>(cur_page);

    ctx.read_set_.emplace_back(
        bpm_->FetchPageRead(internal_page->ValueAt(internal_page->Binarysearch(key, comparator_))));
    ctx.read_set_.pop_front();
    cur_page = ctx.read_set_.back().As<BPlusTreePage>();
  }
  // FindLeaf(ctx, key, cur_page, 0);
  auto leaf = reinterpret_cast<const LeafPage *>(cur_page);
  ctx.read_set_.clear();
  int idx = leaf->Binarysearch(key, comparator_);
  if (idx < leaf->GetSize() && !comparator_(key, leaf->KeyAt(idx))) {
    if (result != nullptr) {
      result->emplace_back(leaf->ValueAt(idx));
    }
    return true;
  }
  return false;
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/*
 * Insert constant key & value pair into b+ tree
 * if current tree is empty, start new tree, update root page id and insert
 * entry, otherwise insert into leaf page.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::Insert(const KeyType &key, const ValueType &value, Transaction *txn) -> bool {
  // Declaration of context instance.
  Context ctx;
  ctx.write_set_.emplace_back(bpm_->FetchPageWrite(header_page_id_));
  auto root = ctx.write_set_.back().AsMut<BPlusTreeHeaderPage>();
  ctx.root_page_id_ = root->root_page_id_;
  if (ctx.root_page_id_ == INVALID_PAGE_ID) {
    page_id_t leaf_pid{};
    auto leaf_page = bpm_->NewPageGuarded(&leaf_pid);
    WritePageGuard guard = bpm_->FetchPageWrite(leaf_pid);
    auto leaf = reinterpret_cast<LeafPage *>(guard.AsMut<BPlusTreePage>());
    leaf->Init(leaf_max_size_);
    leaf->Insert(key, value, 0);
    root->root_page_id_ = leaf_pid;
    return true;
  }
  ctx.write_set_.emplace_back(bpm_->FetchPageWrite(ctx.root_page_id_));
  auto cur_page = ctx.write_set_.back().As<BPlusTreePage>();
  if (cur_page->GetSize() < cur_page->GetMaxSize()) {
    ctx.write_set_.pop_front();
  }
  FindLeaf(ctx, key, cur_page);
  auto leaf = reinterpret_cast<LeafPage *>(ctx.write_set_.back().AsMut<BPlusTreePage>());
  int pos_insert = leaf->Binarysearch(key, comparator_);
  if (pos_insert < leaf->GetSize() && !comparator_(key, leaf->KeyAt(pos_insert))) {
    return false;
  }
  std::optional<std::pair<KeyType, page_id_t>> tmp_pair = std::nullopt;
  if (leaf->GetSize() >= leaf_max_size_) {
    tmp_pair = SplitLeaf(leaf, key, value);
    ctx.write_set_.pop_back();
    while (!ctx.write_set_.empty()) {
      auto parent = reinterpret_cast<InternalPage *>(ctx.write_set_.back().AsMut<BPlusTreePage>());
      if (parent->GetPageType() != IndexPageType::INTERNAL_PAGE) {
        break;
      }
      InsertToParent(parent, tmp_pair->first, tmp_pair->second);
      if (parent->GetSize() > internal_max_size_) {
        tmp_pair = SplitInternal(parent);
      } else {
        tmp_pair = std::nullopt;
        break;
      }
      ctx.write_set_.pop_back();
    }
  } else {
    leaf->Insert(key, value, pos_insert);
    return true;
  }
  if (tmp_pair) {  // new root
    CreateNewRoot(ctx, root, *tmp_pair);
  }
  return true;
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::FindLeaf(Context &ctx, const KeyType &key, const BPlusTreePage *cur_page) {
  while (!cur_page->IsLeafPage()) {
    auto internal_page = reinterpret_cast<const InternalPage *>(cur_page);
    ctx.write_set_.emplace_back(
        bpm_->FetchPageWrite(internal_page->ValueAt(internal_page->Binarysearch(key, comparator_))));
    cur_page = ctx.write_set_.back().As<BPlusTreePage>();
    if (cur_page->GetSize() < cur_page->GetMaxSize()) {
      while (ctx.write_set_.size() > 1) {
        ctx.write_set_.pop_front();
      }
    }
  }
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::InsertToLeaf(LeafPage *leaf, KeyType key, ValueType value) {
  int pos_insert = leaf->Binarysearch(key, comparator_);
  leaf->Insert(key, value, pos_insert);
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::InsertToParent(InternalPage *parent, KeyType key, page_id_t value) {
  int left = 1;
  int right = parent->GetSize() - 1;
  while (left <= right) {
    int mid = (left + right) >> 1;
    int comparation = comparator_(key, parent->KeyAt(mid));
    if (comparation < 0) {
      right = mid - 1;
    } else {
      left = mid + 1;
    }
  }
  parent->Insert(key, value, right + 1);
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::CreateNewRoot(Context &ctx, BPlusTreeHeaderPage *root, std::pair<KeyType, page_id_t> KV) {
  page_id_t new_root_id;
  bpm_->NewPageGuarded(&new_root_id);
  WritePageGuard root_guard = bpm_->FetchPageWrite(new_root_id);
  auto new_root = reinterpret_cast<InternalPage *>(root_guard.AsMut<BPlusTreePage>());
  new_root->Init(internal_max_size_);
  new_root->Insert(KV.first, ctx.root_page_id_, 0);
  new_root->Insert(KV.first, KV.second, 1);
  root->root_page_id_ = new_root_id;
  ctx.write_set_.pop_back();
}

INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::SplitLeaf(LeafPage *leaf1, KeyType key, ValueType value) -> std::pair<KeyType, page_id_t> {
  page_id_t leaf2_pid;
  auto leaf2_page = bpm_->NewPageGuarded(&leaf2_pid);
  WritePageGuard leaf2_guard = bpm_->FetchPageWrite(leaf2_pid);
  auto leaf2 = reinterpret_cast<LeafPage *>(leaf2_guard.AsMut<BPlusTreePage>());
  leaf2->Init(leaf_max_size_);

  auto mid_key = leaf1->Split(leaf2, leaf2_pid);
  if (comparator_(key, mid_key) < 0) {
    InsertToLeaf(leaf1, key, value);
  } else {
    InsertToLeaf(leaf2, key, value);
  }
  return std::pair<KeyType, page_id_t>(mid_key, leaf2_pid);
}

INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::SplitInternal(InternalPage *node) -> std::pair<KeyType, page_id_t> {
  page_id_t new_node_pid;
  auto new_node_page = bpm_->NewPageGuarded(&new_node_pid);
  WritePageGuard new_node_guard = bpm_->FetchPageWrite(new_node_pid);
  auto new_node = reinterpret_cast<InternalPage *>(new_node_guard.AsMut<BPlusTreePage>());
  new_node->Init(internal_max_size_);

  auto mid_key = node->Split(new_node);
  return std::pair<KeyType, page_id_t>(mid_key, new_node_pid);
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * Delete key & value pair associated with input key
 * If current tree is empty, return immediately.
 * If not, User needs to first find the right leaf page as deletion target, then
 * delete entry from leaf page. Remember to deal with redistribute or merge if
 * necessary.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Remove(const KeyType &key, Transaction *txn) {
  // Declaration of context instance.
  Context ctx;
  (void)ctx;
}

/*****************************************************************************
 * INDEX ITERATOR
 *****************************************************************************/
/*
 * Input parameter is void, find the leftmost leaf page first, then construct
 * index iterator
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::Begin() -> INDEXITERATOR_TYPE { return INDEXITERATOR_TYPE(); }

/*
 * Input parameter is low key, find the leaf page that contains the input key
 * first, then construct index iterator
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::Begin(const KeyType &key) -> INDEXITERATOR_TYPE { return INDEXITERATOR_TYPE(); }

/*
 * Input parameter is void, construct an index iterator representing the end
 * of the key/value pair in the leaf node
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::End() -> INDEXITERATOR_TYPE { return INDEXITERATOR_TYPE(); }

/**
 * @return Page id of the root of this tree
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::GetRootPageId() const -> page_id_t {
  ReadPageGuard root_page_guard = bpm_->FetchPageRead(header_page_id_);
  auto root_page = root_page_guard.As<BPlusTreeHeaderPage>();
  auto root_pid = root_page->root_page_id_;
  return root_pid;
}

/*****************************************************************************
 * UTILITIES AND DEBUG
 *****************************************************************************/

/*
 * This method is used for test only
 * Read data from file and insert one by one
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::InsertFromFile(const std::string &file_name, Transaction *txn) {
  int64_t key;
  std::ifstream input(file_name);
  while (input) {
    input >> key;

    KeyType index_key;
    index_key.SetFromInteger(key);
    RID rid(key);
    Insert(index_key, rid, txn);
  }
}
/*
 * This method is used for test only
 * Read data from file and remove one by one
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::RemoveFromFile(const std::string &file_name, Transaction *txn) {
  int64_t key;
  std::ifstream input(file_name);
  while (input) {
    input >> key;
    KeyType index_key;
    index_key.SetFromInteger(key);
    Remove(index_key, txn);
  }
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Print(BufferPoolManager *bpm) {
  auto root_page_id = GetRootPageId();
  auto guard = bpm->FetchPageBasic(root_page_id);
  PrintTree(guard.PageId(), guard.template As<BPlusTreePage>());
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::PrintTree(page_id_t page_id, const BPlusTreePage *page) {
  if (page->IsLeafPage()) {
    auto *leaf = reinterpret_cast<const LeafPage *>(page);
    std::cout << "Leaf Page: " << page_id << "\tNext: " << leaf->GetNextPageId() << std::endl;

    // Print the contents of the leaf page.
    std::cout << "Contents: ";
    for (int i = 0; i < leaf->GetSize(); i++) {
      std::cout << leaf->KeyAt(i);
      if ((i + 1) < leaf->GetSize()) {
        std::cout << ", ";
      }
    }
    std::cout << '\n';
    std::cout << '\n';

  } else {
    auto *internal = reinterpret_cast<const InternalPage *>(page);
    std::cout << "Internal Page: " << page_id << '\n';

    // Print the contents of the internal page.
    std::cout << "Contents: ";
    for (int i = 0; i < internal->GetSize(); i++) {
      std::cout << internal->KeyAt(i) << ": " << internal->ValueAt(i);
      if ((i + 1) < internal->GetSize()) {
        std::cout << ", ";
      }
    }
    std::cout << '\n';
    std::cout << '\n';
    for (int i = 0; i < internal->GetSize(); i++) {
      auto guard = bpm_->FetchPageBasic(internal->ValueAt(i));
      PrintTree(guard.PageId(), guard.template As<BPlusTreePage>());
    }
  }
}

/**
 * This method is used for debug only, You don't need to modify
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Draw(BufferPoolManager *bpm, const std::string &outf) {
  if (IsEmpty()) {
    LOG_WARN("Drawing an empty tree");
    return;
  }

  std::ofstream out(outf);
  out << "digraph G {" << '\n';
  auto root_page_id = GetRootPageId();
  auto guard = bpm->FetchPageBasic(root_page_id);
  ToGraph(guard.PageId(), guard.template As<BPlusTreePage>(), out);
  out << "}" << '\n';
  out.close();
}

/**
 * This method is used for debug only, You don't need to modify
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::ToGraph(page_id_t page_id, const BPlusTreePage *page, std::ofstream &out) {
  std::string leaf_prefix("LEAF_");
  std::string internal_prefix("INT_");
  if (page->IsLeafPage()) {
    auto *leaf = reinterpret_cast<const LeafPage *>(page);
    // Print node name
    out << leaf_prefix << page_id;
    // Print node properties
    out << "[shape=plain color=green ";
    // Print data of the node
    out << "label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n";
    // Print data
    out << "<TR><TD COLSPAN=\"" << leaf->GetSize() << "\">P=" << page_id << "</TD></TR>\n";
    out << "<TR><TD COLSPAN=\"" << leaf->GetSize() << "\">"
        << "max_size=" << leaf->GetMaxSize() << ",min_size=" << leaf->GetMinSize() << ",size=" << leaf->GetSize()
        << "</TD></TR>\n";
    out << "<TR>";
    for (int i = 0; i < leaf->GetSize(); i++) {
      out << "<TD>" << leaf->KeyAt(i) << "</TD>\n";
    }
    out << "</TR>";
    // Print table end
    out << "</TABLE>>];\n";
    // Print Leaf node link if there is a next page
    if (leaf->GetNextPageId() != INVALID_PAGE_ID) {
      out << leaf_prefix << page_id << " -> " << leaf_prefix << leaf->GetNextPageId() << ";\n";
      out << "{rank=same " << leaf_prefix << page_id << " " << leaf_prefix << leaf->GetNextPageId() << "};\n";
    }
  } else {
    auto *inner = reinterpret_cast<const InternalPage *>(page);
    // Print node name
    out << internal_prefix << page_id;
    // Print node properties
    out << "[shape=plain color=pink ";  // why not?
    // Print data of the node
    out << "label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n";
    // Print data
    out << "<TR><TD COLSPAN=\"" << inner->GetSize() << "\">P=" << page_id << "</TD></TR>\n";
    out << "<TR><TD COLSPAN=\"" << inner->GetSize() << "\">"
        << "max_size=" << inner->GetMaxSize() << ",min_size=" << inner->GetMinSize() << ",size=" << inner->GetSize()
        << "</TD></TR>\n";
    out << "<TR>";
    for (int i = 0; i < inner->GetSize(); i++) {
      out << "<TD PORT=\"p" << inner->ValueAt(i) << "\">";
      if (i > 0) {
        out << inner->KeyAt(i);
      } else {
        out << " ";
      }
      out << "</TD>\n";
    }
    out << "</TR>";
    // Print table end
    out << "</TABLE>>];\n";
    // Print leaves
    for (int i = 0; i < inner->GetSize(); i++) {
      auto child_guard = bpm_->FetchPageBasic(inner->ValueAt(i));
      auto child_page = child_guard.template As<BPlusTreePage>();
      ToGraph(child_guard.PageId(), child_page, out);
      if (i > 0) {
        auto sibling_guard = bpm_->FetchPageBasic(inner->ValueAt(i - 1));
        auto sibling_page = sibling_guard.template As<BPlusTreePage>();
        if (!sibling_page->IsLeafPage() && !child_page->IsLeafPage()) {
          out << "{rank=same " << internal_prefix << sibling_guard.PageId() << " " << internal_prefix
              << child_guard.PageId() << "};\n";
        }
      }
      out << internal_prefix << page_id << ":p" << child_guard.PageId() << " -> ";
      if (child_page->IsLeafPage()) {
        out << leaf_prefix << child_guard.PageId() << ";\n";
      } else {
        out << internal_prefix << child_guard.PageId() << ";\n";
      }
    }
  }
}

INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::DrawBPlusTree() -> std::string {
  if (IsEmpty()) {
    return "()";
  }

  PrintableBPlusTree p_root = ToPrintableBPlusTree(GetRootPageId());
  std::ostringstream out_buf;
  p_root.Print(out_buf);

  return out_buf.str();
}

INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::ToPrintableBPlusTree(page_id_t root_id) -> PrintableBPlusTree {
  auto root_page_guard = bpm_->FetchPageBasic(root_id);
  auto root_page = root_page_guard.template As<BPlusTreePage>();
  PrintableBPlusTree proot;

  if (root_page->IsLeafPage()) {
    auto leaf_page = root_page_guard.template As<LeafPage>();
    proot.keys_ = leaf_page->ToString();
    proot.size_ = proot.keys_.size() + 4;  // 4 more spaces for indent

    return proot;
  }

  // draw internal page
  auto internal_page = root_page_guard.template As<InternalPage>();
  proot.keys_ = internal_page->ToString();
  proot.size_ = 0;
  for (int i = 0; i < internal_page->GetSize(); i++) {
    page_id_t child_id = internal_page->ValueAt(i);
    PrintableBPlusTree child_node = ToPrintableBPlusTree(child_id);
    proot.size_ += child_node.size_;
    proot.children_.push_back(child_node);
  }

  return proot;
}

template class BPlusTree<GenericKey<4>, RID, GenericComparator<4>>;

template class BPlusTree<GenericKey<8>, RID, GenericComparator<8>>;

template class BPlusTree<GenericKey<16>, RID, GenericComparator<16>>;

template class BPlusTree<GenericKey<32>, RID, GenericComparator<32>>;

template class BPlusTree<GenericKey<64>, RID, GenericComparator<64>>;

}  // namespace bustub
