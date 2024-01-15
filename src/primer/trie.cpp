#include "primer/trie.h"
#include <cstddef>
#include <memory>
#include <shared_mutex>
#include <stack>
#include <string_view>
#include <utility>
#include "common/exception.h"

namespace bustub {


template <class T>
auto Trie::Get(std::string_view key) const -> const T * {
  // throw NotImplementedException("Trie::Get is not implemented.");
  // You should walk through the trie to find the node corresponding to the key. If the node doesn't exist, return
  // nullptr. After you find the node, you should use `dynamic_cast` to cast it to `const TrieNodeWithValue<T> *`. If
  // dynamic_cast returns `nullptr`, it means the type of the value is mismatched, and you should return nullptr.
  // Otherwise, return the value.
  std::shared_ptr<const TrieNode> current_node=root_;
  for(auto ch:key){
    if(current_node==nullptr){
      break;
    }
    auto tmp=current_node->children_.find(ch);
    current_node=tmp==current_node->children_.end()?nullptr:tmp->second;
  }
  if(current_node==nullptr||!current_node->is_value_node_){
    return nullptr;
  }
  auto fuck=dynamic_cast<const TrieNodeWithValue<T>*>(current_node.get());
  if(fuck==nullptr){
    return nullptr;
  }     
  return fuck->value_.get(); 
}


template <class T>
auto Trie::Put(std::string_view key, T value) const -> Trie {
  // Note that `T` might be a non-copyable type. Always use `std::move` when creating `shared_ptr` on that value.
  //throw NotImplementedException("Trie::Put is not implemented.");

  // You should walk through the trie and create new nodes if necessary. If the node corresponding to the key already
  // exists, you should create a new `TrieNodeWithValue`.
  std::shared_ptr<T> value_backup=std::make_shared<T>(std::move(value));
  std::stack<std::shared_ptr<const TrieNode>> existed_node;
  std::shared_ptr<const TrieNode> current_node=root_;
  std::size_t idx=0;
  for(idx=0;idx<key.size()&&current_node!=nullptr;++idx){
    existed_node.push(current_node);
    auto it = current_node->children_.find(key[idx]);
    if (it == current_node->children_.end()) {
      current_node = nullptr; 
    }
    else { 
      current_node = it->second;
    }
  }
  std::shared_ptr<const TrieNodeWithValue<T>> leaf=
    current_node==nullptr
    ?std::make_shared<const TrieNodeWithValue<T>>(value_backup)
    :std::make_shared<const TrieNodeWithValue<T>>(current_node->children_,value_backup);
  std::shared_ptr<const TrieNode> child=leaf;
  for(int i=key.size()-1;i>=static_cast<int>(idx);--i){//防止size_t越界 fuck!!!!!
    std::map<char, std::shared_ptr<const TrieNode>> son{{key[i],child}};
    current_node=std::make_shared<const TrieNode>(son);
    child=current_node;
  }
  if(key.empty()) {//key为空会出问题
    current_node=child;
  }
  while(!existed_node.empty()){
    current_node=std::shared_ptr<const TrieNode>(existed_node.top()->Clone());
    const_cast<TrieNode *>(current_node.get())->children_[key[existed_node.size()-1]] = child;
    child=current_node;
    existed_node.pop();
  }
  return Trie(current_node);
}

auto Trie::Remove(std::string_view key) const -> Trie {
  //throw NotImplementedException("Trie::Remove is not implemented.");

  // You should walk through the trie and remove nodes if necessary. If the node doesn't contain a value any more,
  // you should convert it to `TrieNode`. If a node doesn't have children any more, you should remove it.

  std::stack<std::shared_ptr<const TrieNode>> existed_node;
  std::shared_ptr<const TrieNode> current_node=root_;
  std::size_t idx=0;
  for(idx=0;idx<key.size()&&current_node!=nullptr;++idx){
    existed_node.push(current_node);
    auto it = current_node->children_.find(key[idx]);
    if (it == current_node->children_.end()) {
      current_node = nullptr; 
    }
    else { 
      current_node = it->second;
    }
  }
  if(idx!=key.size()||!current_node||!current_node->is_value_node_){
    // return Trie(root_);
    return *this;
  }
  std::shared_ptr<const TrieNode> leaf=
    current_node->children_.empty()
    ? nullptr
    :std::make_shared<const TrieNode>(current_node->children_);
  std::shared_ptr<const TrieNode> child=leaf;
  if(key.empty()) {
    current_node=child;
  }
  while(!existed_node.empty()){
    current_node=std::shared_ptr<const TrieNode>(existed_node.top()->Clone());
    const_cast<TrieNode *>(current_node.get())->children_[key[existed_node.size()-1]] = child;
    child=current_node;
    existed_node.pop();
  }
  return Trie(current_node);
}

// Below are explicit instantiation of template functions.
//
// Generally people would write the implementation of template classes and functions in the header file. However, we
// separate the implementation into a .cpp file to make things clearer. In order to make the compiler know the
// implementation of the template functions, we need to explicitly instantiate them here, so that they can be picked up
// by the linker.

template auto Trie::Put(std::string_view key, uint32_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint32_t *;

template auto Trie::Put(std::string_view key, uint64_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint64_t *;

template auto Trie::Put(std::string_view key, std::string value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const std::string *;

// If your solution cannot compile for non-copy tests, you can remove the below lines to get partial score.

using Integer = std::unique_ptr<uint32_t>;

template auto Trie::Put(std::string_view key, Integer value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const Integer *;

template auto Trie::Put(std::string_view key, MoveBlocked value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const MoveBlocked *;

}  // namespace bustub
