/*
* Copyright 2014 Facebook, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

// @author Bo Hu (bhu@fb.com)
// @author Jordan DeLong (delong.j@fb.com)

//edited by Tommaso Checchi (tommo@mojang.com) to remove boost, exceptions, and add locking on full queue

#pragma once

#include "dojo_common_header.h"

#include <atomic>
#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace folly {

	/*
	* ProducerConsumerQueue is a one producer and one consumer queue
	* without locks.
	*/
	template<class T>
	struct Pipe {
		typedef T value_type;

		// size must be >= 2.
		//
		// Also, note that the number of usable slots in the queue at any
		// given time is actually (size-1), so if you start with an empty queue,
		// isFull() will return true after size-1 insertions.
		explicit Pipe(uint32_t size = 10000)
			: size_(size)
			, records_(static_cast<T*>(std::malloc(sizeof(T)* size)))
			, readIndex_(0)
			, writeIndex_(0)
		{
			assert(size >= 2);
			DEBUG_ASSERT(records_, "Can't allocate the required amount of records");
		}

		//tommo hack: replace boost::noncopyable
		explicit Pipe(const Pipe& pipe) = delete;
		Pipe& operator=(const Pipe& pipe) = delete;

		~Pipe() {
			// We need to destruct anything that may still exist in our queue.
			// (No real synchronization needed at destructor time: only one
			// thread can be doing this.)

			//TODO what does this REALLY mean?
			//     if (!boost::has_trivial_destructor<T>::value) {
			//       int read = readIndex_;
			//       int end = writeIndex_;
			//       while (read != end) {
			//         records_[read].~T();
			//         if (++read == size_) {
			//           read = 0;
			//         }
			//       }
			//     }

			std::free(records_);
		}

		template<class ...Args>
		bool queue(Args&&... recordArgs) {

			while (isFull()); //spinlock a little while full, it's fine - the system has already hang if it is!

			auto const currentWrite = writeIndex_.load(std::memory_order_relaxed);
			auto nextRecord = currentWrite + 1;
			if (nextRecord == size_) {
				nextRecord = 0;
			}
			if (nextRecord != readIndex_.load(std::memory_order_acquire)) {
				new (&records_[currentWrite]) T(std::forward<Args>(recordArgs)...);
				writeIndex_.store(nextRecord, std::memory_order_release);
				return true;
			}

			// queue is full
			return false;
		}

		// move (or copy) the value at the front of the queue to given variable
		bool tryPop(T& record) {
			auto const currentRead = readIndex_.load(std::memory_order_relaxed);
			if (currentRead == writeIndex_.load(std::memory_order_acquire)) {
				// queue is empty
				return false;
			}

			auto nextRecord = currentRead + 1;
			if (nextRecord == size_) {
				nextRecord = 0;
			}
			record = std::move(records_[currentRead]);
			records_[currentRead].~T();
			readIndex_.store(nextRecord, std::memory_order_release);
			return true;
		}

		// pointer to the value at the front of the queue (for use in-place) or
		// nullptr if empty.
		T* frontPtr() {
			auto const currentRead = readIndex_.load(std::memory_order_relaxed);
			if (currentRead == writeIndex_.load(std::memory_order_acquire)) {
				// queue is empty
				return nullptr;
			}
			return &records_[currentRead];
		}

		// queue must not be empty
		void popFront() {
			auto const currentRead = readIndex_.load(std::memory_order_relaxed);
			assert(currentRead != writeIndex_.load(std::memory_order_acquire));

			auto nextRecord = currentRead + 1;
			if (nextRecord == size_) {
				nextRecord = 0;
			}
			records_[currentRead].~T();
			readIndex_.store(nextRecord, std::memory_order_release);
		}

		bool isEmpty() const {
			return readIndex_.load(std::memory_order_consume) ==
				writeIndex_.load(std::memory_order_consume);
		}

		bool isFull() const {
			auto nextRecord = writeIndex_.load(std::memory_order_consume) + 1;
			if (nextRecord == size_) {
				nextRecord = 0;
			}
			if (nextRecord != readIndex_.load(std::memory_order_consume)) {
				return false;
			}
			// queue is full
			return true;
		}

		// * If called by consumer, then true size may be more (because producer may
		//   be adding items concurrently).
		// * If called by producer, then true size may be less (because consumer may
		//   be removing items concurrently).
		// * It is undefined to call this from any other thread.
		size_t sizeGuess() const {
			int ret = writeIndex_.load(std::memory_order_consume) -
				readIndex_.load(std::memory_order_consume);
			if (ret < 0) {
				ret += size_;
			}
			return ret;
		}

	private:
		const uint32_t size_;
		T* const records_;

		std::atomic<int> readIndex_;
		std::atomic<int> writeIndex_;
	};

}

