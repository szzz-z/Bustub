//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_k_replacer.cpp
//
// Identification: src/buffer/lru_k_replacer.cpp
//
// Copyright (c) 2015-2022, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_k_replacer.h"
#include <mutex>
#include "common/exception.h"

namespace bustub {

LRUKReplacer::LRUKReplacer(size_t num_frames, size_t k) : replacer_size_(num_frames), k_(k) {}

auto LRUKReplacer::Evict(frame_id_t *frame_id) -> bool {
  std::scoped_lock<std::mutex> lock(latch_);
  for (auto it = history_.rbegin(); it != history_.rend(); ++it) {
    auto evictable_it = evictable_.find(*it);
    if (evictable_it != evictable_.end() && evictable_it->second) {
      *frame_id = *it;
      records_.erase(*frame_id);

      auto history_hash_it = history_hash_.find(*frame_id);
      if (history_hash_it != history_hash_.end()) {
        history_.erase(history_hash_it->second);
        history_hash_.erase(history_hash_it);
      }

      curr_size_ -= 1;
      evictable_.erase(evictable_it);
      return true;
    }
  }

  for (auto it = cache_.rbegin(); it != cache_.rend(); ++it) {
    auto evictable_it = evictable_.find(*it);
    if (evictable_it != evictable_.end() && evictable_it->second) {
      *frame_id = *it;
      records_.erase(*frame_id);

      auto node_store_it = cache_hash_.find(*frame_id);
      if (node_store_it != cache_hash_.end()) {
        cache_.erase(node_store_it->second);
        cache_hash_.erase(node_store_it);
      }

      curr_size_ -= 1;
      evictable_.erase(evictable_it);
      return true;
    }
  }

  return false;
}

void LRUKReplacer::RecordAccess(frame_id_t frame_id, [[maybe_unused]] AccessType access_type) {
  std::scoped_lock<std::mutex> lock(latch_);
  BUSTUB_ASSERT(frame_id < static_cast<frame_id_t>(replacer_size_), "frame id invalid");

  auto record_it = records_.find(frame_id);
  if (record_it == records_.end()) {
    evictable_[frame_id] = true;
    records_[frame_id] = 1;
    curr_size_ += 1;
    history_.push_front(frame_id);
    history_hash_[frame_id] = history_.begin();
  } else if (record_it->second + 1 < k_) {
    record_it->second += 1;
  } else if (record_it->second + 1 == k_) {
    record_it->second += 1;
    auto it = history_hash_.find(frame_id);
    if (it != history_hash_.end()) {
      history_.erase(it->second);
      history_hash_.erase(it);
    }
    cache_.push_front(frame_id);
    cache_hash_[frame_id] = cache_.begin();
  } else {
    auto it = cache_hash_.find(frame_id);
    if (it != cache_hash_.end()) {
      cache_.erase(it->second);
      cache_hash_.erase(it);
    }
    cache_.push_front(frame_id);
    cache_hash_[frame_id] = cache_.begin();
  }
}

void LRUKReplacer::SetEvictable(frame_id_t frame_id, bool set_evictable) {
  std::scoped_lock<std::mutex> lock(latch_);
  BUSTUB_ASSERT(frame_id < static_cast<frame_id_t>(replacer_size_), "frame id invalid");

  auto records_it = records_.find(frame_id);
  if (records_it == records_.end() || records_it->second == 0) {
    return;
  }

  auto evictable_it = evictable_.find(frame_id);
  if (evictable_it != evictable_.end()) {
    if (evictable_it->second && !set_evictable) {
      curr_size_ -= 1;
    } else if (!evictable_it->second && set_evictable) {
      curr_size_ += 1;
    }
    evictable_it->second = set_evictable;
  } else {
    evictable_[frame_id] = set_evictable;
    if (set_evictable) {
      curr_size_ += 1;
    }
  }
}

void LRUKReplacer::Remove(frame_id_t frame_id) {
  std::scoped_lock<std::mutex> lock(latch_);

  auto cache_hash_it = cache_hash_.find(frame_id);
  if (cache_hash_it != cache_hash_.end()) {
    cache_.erase(cache_hash_it->second);
    cache_hash_.erase(cache_hash_it);
  }

  auto history_hash_it = history_hash_.find(frame_id);
  if (history_hash_it != history_hash_.end()) {
    history_.erase(history_hash_it->second);
    history_hash_.erase(history_hash_it);
  }

  auto evictable_it = evictable_.find(frame_id);
  if (evictable_it != evictable_.end()) {
    if (evictable_it->second) {
      curr_size_ -= 1;
    }
    evictable_.erase(evictable_it);
  }
  records_.erase(frame_id);
}

auto LRUKReplacer::Size() -> size_t {
  std::scoped_lock<std::mutex> lock(latch_);
  return curr_size_;
}

}  // namespace bustub
