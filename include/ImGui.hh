#ifndef IMGUI_HH_
#define IMGUI_HH_
#include "Element.hh"
#include "Label.hh"
#include "Grid.hh"
#include "OverlapPanel.hh"
#include <stack>

namespace easy {

	namespace imgui {

		namespace _impl {


			struct FrameHelper;
			struct Frame {
			public:
				friend struct FrameHelper;
				Frame* parent;
				Element elem;
			protected:
				Frame(const Element& elem) : elem(elem), parent(nullptr) {}
			private:
				static Frame*& CurrentFrame() {
					static Frame* curr = nullptr;
					return curr;
				}
			public:
				static Frame* Top() {
					return CurrentFrame();
				}
				static void Push(Frame* next) {
					next->parent = CurrentFrame();
					CurrentFrame() = next;
				}
				static void Pop() {
					Frame* curr = CurrentFrame();
					Frame* parent = curr->parent;
					CurrentFrame() = parent;
					if (parent) {
						parent->Close(curr);
						delete curr;
					} else {
						Element root = curr->elem;
						delete curr;
						Renderer::MainLoop(root);
					}
				}
				static void Clear() {
					Frame* curr = CurrentFrame();
					while (curr) {
						CurrentFrame() = curr->parent;
						delete curr;
						curr = CurrentFrame();
					}
				}
			protected:
				virtual void Close(Frame* child) {
					throw std::runtime_error("Element cannot have children");
				}
			};

			struct GridFrame : Frame {
			public:
				Pos pos = {};
				GridFrame(const Grid& elem) : Frame(elem) {}
			protected:
				void Close(Frame* child) {
					_Grid* g = dynamic_cast<_Grid*>(elem.get());
					if (pos.Y < 0 || pos.Y >= g->Capacity().Height ||
						pos.X < 0 || pos.X >= g->Capacity().Width) {
						throw std::runtime_error("Grid index out of range");
					}
					g->Set(pos.Y, pos.X, child->elem);
				}
			};

			struct OverlapPanelFrame : Frame {
			public:
				OverlapPanelFrame(const OverlapPanel& elem) : Frame(elem) {}
			protected:
				void Close(Frame* child) {
					_OverlapPanel* o = dynamic_cast<_OverlapPanel*>(elem.get());
					o->Add(child->elem);
				}
			};


			template<typename T, typename O>
			struct Assigner {
				T O::* prop;
				Assigner(T O::* prop): prop(prop) {}
				Assigner(const Assigner&) = delete;

				void operator =(T value) const {
					_Element* e = Frame::Top()->elem.get();
					dynamic_cast<O*>(e)->*prop = value;
				}
			};

			template<typename H, EventType E, typename O>
			struct EventAdder {
				_event_forwarder<EventType, E, H> O::* e;
				EventAdder(_event_forwarder<EventType, E, H> O::* e) : e(e) {}
				EventAdder(const EventAdder&) = delete;
				void operator +=(const H& handler) {
					_Element* elem = Frame::Top()->elem.get();
					dynamic_cast<O*>(elem)->*e += handler;
				}
			};

			struct GridAssigner {
				GridAssigner() {}
				GridAssigner(const GridAssigner&) = delete;

				void operator =(Pos pos) const {
					GridFrame* e = dynamic_cast<GridFrame*>(Frame::Top()->parent);
					if (!e) throw std::runtime_error("Only children of Grid can set position");
					e->pos = pos;
				}
			};

			struct FrameHelper {
				template<typename T> FrameHelper(const T& elem) { Frame::Push(new Frame(elem)); }
				template<> FrameHelper(const Grid& elem) { Frame::Push(new GridFrame(elem)); }
				template<> FrameHelper(const OverlapPanel& elem) { Frame::Push(new OverlapPanelFrame(elem)); }
				FrameHelper(const FrameHelper&) = delete;
				~FrameHelper() { Frame::Pop(); }
				operator bool() const { return true; }
			};
		}

		_impl::Assigner Margin = &_Element::Margin;
		_impl::Assigner SpecSize = &_Element::SpecSize;
		_impl::Assigner ActualSize = &_Element::ActualSize;
		_impl::Assigner ActualPos = &_Element::ActualPos;
		_impl::Assigner Visible = &_Element::Visible;
		_impl::Assigner BorderThickness = &_Element::BorderThickness;
		_impl::Assigner BorderColor = &_Element::BorderColor;
		_impl::Assigner BackgroundColor = &_Element::BackgroundColor;
		_impl::Assigner VerticalAlignment = &_Element::VerticalAlignment;
		_impl::Assigner HorizontalAlignment = &_Element::HorizontalAlignment;
		_impl::EventAdder Drag = &_Element::Drag;
		_impl::EventAdder BeforeRender = &_Element::BeforeRender;
		_impl::EventAdder Click = &_Element::Click;
		_impl::EventAdder StartAnyAnimation = &IAnimation::StartAnyAnimation;
		_impl::EventAdder FinishAllAnimation = &IAnimation::FinishAllAnimation;
		_impl::GridAssigner GridPosition;
		_impl::Assigner Text = &_Label::Text;
		_impl::Assigner FontColor = &_Label::FontColor;
		_impl::Assigner FontSize = &_Label::FontSize;
		_impl::Assigner FontVerticalAlignment = &_Label::FontVerticalAlignment;
		_impl::Assigner FontHorizontalAlignment = &_Label::FontHorizontalAlignment;


#define begin_im Register<Renderer>(); Element This = nullptr
#define with(x) if (Element Parent = This, This = x; easy::imgui::_impl::FrameHelper _ = std::dynamic_pointer_cast<typename decltype(x)::element_type, _Element>(This))
#define with_named(name, x) if (Element Parent = This, This = (name = x); easy::imgui::_impl::FrameHelper _ = std::dynamic_pointer_cast<typename decltype(x)::element_type, _Element>(This))
		


	}

}

#endif // !IMGUI_HH_
