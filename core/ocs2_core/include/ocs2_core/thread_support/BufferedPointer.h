// Copyright (c) 2025, Koło Naukowe Robotyków
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/*
 * Authors: Bartłomiej Krajewski (https://github.com/BartlomiejK2)
 */

#pragma once

#include <ocs2_core/thread_support/Synchronized.h>

namespace ocs2 {

/**
 * Wraps a pointer with a thread-safe buffer. A pointer remains constant and its value can be accessed through the get function until
 * updateFromBuffer is called.
 *
 * In the meantime, multiple threads can set new pointers to the buffer. The active value is not protected by a mutex, so
 * only one thread should access/modify the active pointer (i.e. not simultaneously calling get() and updateFromBuffer()).
 *
 * @tparam T : wrapped type
 */
template <typename T>
class BufferedPointer {
 public:
  /**
   * Constructor initializes with a given value and an empty buffer.
   * @param value
   */
  explicit BufferedPointer(std::unique_ptr<T> value) : activeValue_(std::move(value)), buffer_(nullptr){};

  /** Read the currently active pointer value. */
  const T& get() const { return *activeValue_.get(); }

  /** Read/write the currently active pointer value. */
  T& get() { return *activeValue_.get(); }

  /** Move a new pointer value into the buffer. */
  void setBuffer(std::unique_ptr<T> value) { buffer_.reset(std::move(value)); }

  /**
   * Replaces the active pointer with the pointer in the buffer.
   * The active pointer is not mutex protected so this method is NOT thread-safe w.r.t. get()
   * The buffer is mutex protected, so this method is thread-safe w.r.t. setBuffer()
   * @return True: the active pointer was updated, False: the active pointer was not updated.
   */
  bool updateFromBuffer() {
    // Read buffer with a pointer swap to minimize time under the lock. The swapped pointer will be null if there was no new value set.
    std::unique_ptr<T> updatedValuePtr(nullptr);
    buffer_.swap(updatedValuePtr);
    if (updatedValuePtr != nullptr) {
      activeValue_ = std::move(updatedValuePtr);
      return true;
    } else {
      return false;
    }
  }

 private:
  std::unique_ptr<T> activeValue_;
  Synchronized<T> buffer_;
};

}  // namespace ocs2