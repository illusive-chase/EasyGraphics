#include "include/ImGui.hh"
#include "system/SystemIO.hh"


int main() {

	using namespace easy;
	using namespace easy::imgui;

	begin_im;
	with (MakeGrid({100,200,0}, {200,100,0})) {
		BackgroundColor = Colors::Blue;
		with (MakeElement()) {
			BackgroundColor = Colors::Red;
			BorderColor = Colors::Green;
			BorderThickness = { 10,10,10,10 };
			Margin = { 0,2,10,60 };
			GridPosition = { 2, 2 };
			Drag += [=](Element sender, MouseEventArgs args) {
				Parent->BackgroundColor.Red += args.offset.X;
				Parent->BackgroundColor.Green += args.offset.Y;
				Renderer::Invalidated() = true;
			};

			Click += [=](auto sender, auto args) {
				This->BeginAnimation(
					This,
					&_Element::BackgroundColor,
					This->BackgroundColor,
					Parent->BackgroundColor,
					500
				);
				Renderer::Invalidated() = true;
			};
		}
	}
}

