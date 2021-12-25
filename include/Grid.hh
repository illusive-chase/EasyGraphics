#ifndef GRID_HH_
#define GRID_HH_

#include "Element.hh"
#include <vector>

namespace easy {

	struct _Grid;

	using Grid = std::shared_ptr<_Grid>;

	struct _Grid : _Element {
	protected:
		std::vector<int> row_size, col_size;
		int row_free, col_free;
		std::vector<std::vector<Element>> children;

	public:

		_Grid(const std::initializer_list<int>& rows, const std::initializer_list<int>& cols)
			:
			row_size(rows.begin(), rows.end()),
			col_size(cols.begin(), cols.end()),
			row_free(0),
			col_free(0),
			children(rows.size(), std::vector<Element>(cols.size()))
		{
			for (int sz : row_size) {
				SpecSize.Height += sz;
				if (sz == 0) ++row_free;
			}
			for (int sz : col_size) {
				SpecSize.Width += sz;
				if (sz == 0) ++col_free;
			}
		}

		Size Capacity() const {
			return { static_cast<int>(col_size.size()), static_cast<int>(row_size.size()) };
		}

		void Set(int row, int col, const Element& elem) {
			children[row][col] = elem;
		}

		void Reset(int row, int col) {
			children[row][col] = nullptr;
		}

		void Clear() {
			for (auto& i : children)
				for (auto& j : i)
					j = nullptr;
		}

		std::pair<Element, int> MouseTarget(Pos pos) {
			if (!Enable) return std::make_pair(nullptr, std::numeric_limits<int>::max());
			std::vector<std::pair<Element, int>> results;
			using itor = std::vector<std::pair<Element, int>>::iterator;
			auto comp = [](const std::pair<Element, int>& a, const std::pair<Element, int>& b) {
				return a.second < b.second;
			};
			for (auto& i : children)
				for (auto& j : i)
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
			Size margin = {
				std::max(0, size.Width - Margin.Right - Margin.Left - SpecSize.Width),
				std::max(0, size.Height - Margin.Top - Margin.Bottom - SpecSize.Height),
			};
			if (row_free > 0) margin.Height = margin.Height / row_free;
			if (col_free > 0) margin.Width = margin.Width / col_free;
			for (int r = 0; r < (int)row_size.size(); ++r) {
				for (int c = 0; c < (int)col_size.size(); ++c) {
					if (!children[r][c]) continue;
					Size arranged = { col_size[c], row_size[r] };
					if (arranged.Height == 0) arranged.Height = margin.Height;
					if (arranged.Width == 0) arranged.Width = margin.Width;
					children[r][c]->Measure(arranged);
				}
			}
			ActualSize = {
				std::max(0, std::min(size.Width - Margin.Right - Margin.Left, SpecSize.Width + col_free * margin.Width)),
				std::max(0, std::min(size.Height - Margin.Top - Margin.Bottom, SpecSize.Height + row_free * margin.Height))
			};
		}

		void Arange(Pos base, Size size) {
			_Element::Arange(base, size);
			Size margin = {
				std::max(0, size.Width - Margin.Right - Margin.Left - SpecSize.Width),
				std::max(0, size.Height - Margin.Top - Margin.Bottom - SpecSize.Height),
			};
			if (row_free > 0) margin.Height = margin.Height / row_free;
			if (col_free > 0) margin.Width = margin.Width / col_free;
			Pos rel = ActualPos;
			Size arranged = {};
			for (int r = 0; r < (int)row_size.size(); ++r) {
				rel.X = ActualPos.X;
				arranged.Height = row_size[r];
				if (arranged.Height == 0) arranged.Height = margin.Height;
				for (int c = 0; c < (int)col_size.size(); ++c) {
					arranged.Width = col_size[c];
					if (arranged.Width == 0) arranged.Width = margin.Width;
					if (children[r][c]) children[r][c]->Arange(rel, arranged);
					rel.X += arranged.Width;
				}
				rel.Y += arranged.Height;
			}
		}
		
		void Render() {
			if (!Visible) return;
			_Element::Render();
			for (auto& rows : children)
				for (auto& item : rows)
					if (item) item->Render();
		}

	};

	Grid MakeGrid(const std::initializer_list<int>& rows, const std::initializer_list<int>& cols) {
		return Grid(new _Grid(rows, cols));
	}


}

#endif