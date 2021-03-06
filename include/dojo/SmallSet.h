#pragma once

#include <vector>
#include <memory>
#include <algorithm>

namespace Dojo {
	template <class T>
	class SmallSet {

	public:

		typedef std::vector<T> Container;
		typedef typename Container::iterator iterator;
		typedef typename Container::const_iterator const_iterator;

		template <class E>
		static auto find(const SmallSet<E>& c, const E& elem) {
			return std::find(c.c.begin(), c.c.end(), elem);
		}

		template <class PTR, class E>
		static auto find(const SmallSet<PTR>& c, const E& elem) {
			auto itr = c.begin();

			for (; itr != c.end() && itr->get() != &elem; ++itr);

			return itr;
		}

		template <class E>
		static auto find(SmallSet<E>& c, const E& elem) {
			return std::find(c.c.begin(), c.c.end(), elem);
		}

		template <class E>
		static auto find(SmallSet<std::unique_ptr<E>>& c, const E& elem) {
			auto itr = c.begin();

			for (; itr != c.end() && itr->get() != &elem; ++itr);

			return itr;
		}

		SmallSet(int initialSize = 0) :
			c(initialSize) {

		}

		const_iterator find(const T& elem) const {
			return std::find(c.begin(), c.end(), elem);
		}

		iterator find(const T& elem) {
			return std::find(c.begin(), c.end(), elem);
		}

		bool contains(const T& elem) const {
			return find(elem) != c.end();
		}

		template <class... Args>
		auto emplace(Args&& ... args) {
			c.emplace_back(std::forward<Args>(args)...);
			return c.end() - 1;
		}

		void erase(const const_iterator& where) {
			((T&)*where) = std::move(c.back());
			c.pop_back();
		}

		void erase(const T& elem) {
			auto itr = find(elem);

			if (itr != c.end()) {
				erase(itr);
			}
		}

		T& operator[](int idx) {
			return c[idx];
		}

		iterator begin() {
			return c.begin();
		}

		const_iterator begin() const {
			return c.begin();
		}

		iterator end() {
			return c.end();
		}

		const_iterator end() const {
			return c.end();
		}

		size_t size() const {
			return c.size();
		}

		bool empty() const {
			return c.empty();
		}

		void clear() {
			c.clear();
		}

	private:

		std::vector<T> c;
	private:
	};
}
