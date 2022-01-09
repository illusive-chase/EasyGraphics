#ifndef ELEMENT_HH_
#define ELEMENT_HH_

#include "Animation.hh"
#include "Event.hh"
#include "Render.hh"

namespace easy {

	constexpr int MouseTargetThreshold = 20;

	enum class VerticalAlignType {
		Top,
		Center,
		Bottom
	};

	enum class HorizontalAlignType {
		Left,
		Center,
		Right
	};


	struct _Element;

	using Element = std::shared_ptr<_Element>;

	struct _Element : IAnimation, IEvent<Element> {
		Rect Margin = {};
		Size SpecSize = {};
		Size ActualSize = {};
		Pos ActualPos = {};
		bool Visible = true;
		Rect BorderThickness = {};
		Color BorderColor = Colors::Trasparent;
		Color BackgroundColor = Colors::Trasparent;
		VerticalAlignType VerticalAlignment = VerticalAlignType::Top;
		HorizontalAlignType HorizontalAlignment = HorizontalAlignType::Left;

		virtual std::pair<Element, int> MouseTarget(Pos pos) {
			if (!Enable) return std::make_pair(nullptr, std::numeric_limits<int>::max());
			int dist = 
				std::abs(std::abs(pos.X - ActualPos.X) + std::abs(pos.X - ActualPos.X - ActualSize.Width) - ActualSize.Width) / 2
				+ std::abs(std::abs(pos.Y - ActualPos.Y) + std::abs(pos.Y - ActualPos.Y - ActualSize.Height) - ActualSize.Height) / 2;
			return std::make_pair(nullptr, dist);
		}

		virtual void Measure(Size size) {
			ActualSize = {
				std::max(0, std::min(size.Width - Margin.Right - Margin.Left, SpecSize.Width)),
				std::max(0, std::min(size.Height - Margin.Top - Margin.Bottom, SpecSize.Height))
			};
			if (SpecSize.Width == 0) ActualSize.Width = std::max(0, size.Width - Margin.Right - Margin.Left);
			if (SpecSize.Height == 0) ActualSize.Height = std::max(0, size.Height - Margin.Top - Margin.Bottom);
		}

		virtual void Arrange(Pos base, Size size) {
			if (HorizontalAlignment == HorizontalAlignType::Left)
				ActualPos.X = base.X + Margin.Left;
			else if (HorizontalAlignment == HorizontalAlignType::Right)
				ActualPos.X = base.X + size.Width - Margin.Right - ActualSize.Width;
			else
				ActualPos.X = base.X + (size.Width - Margin.Left - Margin.Right - ActualSize.Width) / 2 + Margin.Left;
			if (VerticalAlignment == VerticalAlignType::Top)
				ActualPos.Y = base.Y + Margin.Top;
			else if (VerticalAlignment == VerticalAlignType::Bottom)
				ActualPos.Y = base.Y + size.Height - Margin.Bottom - ActualSize.Height;
			else
				ActualPos.Y = base.Y + (size.Height - Margin.Top - Margin.Bottom - ActualSize.Height) / 2 + Margin.Top;
		}
		virtual void Render() {
			if (!Visible) return;
			if (BackgroundColor.Alpha != 0xFF)
				Renderer::DrawFilledRect(Rect::BaseOn(ActualPos, ActualSize), BackgroundColor);
			if (BorderColor.Alpha != 0xFF)
				Renderer::DrawRect(Rect::BaseOn(ActualPos, ActualSize), BorderColor, BorderThickness);
		}
	};

	Element MakeElement() {
		return Element(new _Element);
	}


	template<typename T>
	void Renderer::MainLoop(T root, double FPS) {
		Size size = {};
		RegisterSize(size.Width, size.Height);
		auto OnRender = [root, size]() {
			if (Renderer::Invalidated()) {
				Pos origin = { 0, 0 };
				root->Measure(size);
				root->Arrange(origin, size);
				root->BeforeRender(root, EventArgs{ EventType::BeforeRender } );
				root->Render();
				Renderer::Render();
				Renderer::Invalidated() = false;
			}
		};
		Timer::RecurrentInvoke(std::max(1, static_cast<int>(1000 / FPS)), 0, OnRender);

		Element on_dragging = nullptr;
		Pos last_mouse = { -1,-1 };

		while (true) {
			int* mouse_click_status = Renderer::RegisterMouseClick(nullptr)();
			Pos mouse = { mouse_click_status[1], mouse_click_status[2] };
			if (on_dragging) {
				Pos offset = mouse - last_mouse;
				on_dragging->Drag(on_dragging, MouseEventArgs { EventType::Drag, mouse, offset });
			}
			if (mouse_click_status[0] == 1 || (mouse_click_status[0] == 2 && !on_dragging)) {
				auto t = root->MouseTarget(mouse);
				if (!t.first) t.first = root;
				if (t.second <= easy::MouseTargetThreshold) {
					if (mouse_click_status[0] == 1)
						t.first->Click(t.first, MouseEventArgs { EventType::Click, mouse, {} }),
						on_dragging = nullptr;
					else if (mouse_click_status[0] == 2)
						on_dragging = t.first;
				}
			}
			last_mouse = mouse;
			Timer::Sync();
		}
	}

}

#endif