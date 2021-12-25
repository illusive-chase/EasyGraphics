#ifndef OverlapPanel_HH_
#define OverlapPanel_HH_

#include "Element.hh"
#include <list>

namespace easy {

	struct _OverlapPanel;

	using OverlapPanel = std::shared_ptr<_OverlapPanel>;

	struct _OverlapPanel : _Element {
	protected:
		std::list<Element> children;

	public:

		_OverlapPanel() {}

		void Add(const Element& elem) {
			children.push_back(elem);
		}

		void AddAt(const Element& elem, int index) {
			if (index == 0) return children.push_front(elem);
			auto p = children.begin();
			for (int i = 0; i < index; ++i) ++p;
			children.insert(p, elem);
		}

		int ElementSize() const {
			return static_cast<int>(children.size());
		}

		auto GetRange() const {
			return std::make_pair(children.begin(), children.end());
		}

		void Remove(const Element& elem) {
			children.remove(elem);
		}

		void RemoveAt(int index) {
			auto p = children.begin();
			for (int i = 0; i < index; ++i) ++p;
			children.erase(p);
		}

		void Clear() {
			children.clear();
		}

		std::pair<Element, int> MouseTarget(Pos pos) {
			if (!Enable) return std::make_pair(nullptr, std::numeric_limits<int>::max());
			std::vector<std::pair<Element, int>> results;
			using itor = std::vector<std::pair<Element, int>>::iterator;
			auto comp = [](const std::pair<Element, int>& a, const std::pair<Element, int>& b) {
				return a.second < b.second;
			};
			for (auto& j : children)
				if (j) {
					auto result = j->MouseTarget(pos);
					if (!result.first) result.first = j;
					results.push_back(result);
				}
			if (results.empty()) return _Element::MouseTarget(pos);
			itor p = results.begin();
			if (results.size() > 1) p = std::min_element(results.begin(), results.end(), comp);
			if (p->second > MouseTargetThreshold) return _Element::MouseTarget(pos);
			return *p;
		}

		void Measure(Size size) {
			ActualSize = {
				std::max(0, size.Width - Margin.Right - Margin.Left),
				std::max(0, size.Height - Margin.Top - Margin.Bottom),
			};
			for (auto& child : children) {
				if (child) child->Measure(ActualSize);
			}
		}

		void Arange(Pos base, Size size) {
			_Element::Arange(base, size);
			for (auto& child : children)
				if (child) child->Arange(ActualPos, ActualSize);
		}

		void Render() {
			if (!Visible) return;
			_Element::Render();
			for (auto& child : children)
				if (child) child->Render();
		}

	};

	OverlapPanel MakeOverlapPanel() {
		return OverlapPanel(new _OverlapPanel());
	}


}

#endif