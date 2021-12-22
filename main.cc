#include "include/Grid.hh"
#include "linux/LinuxRender.hh"

using namespace easy;

int main() {
	Register<Renderer>();
	Size screen = {};
	Renderer::RegisterSize(screen.Width, screen.Height);
	Element elem = MakeElement();
	elem->SpecSize = { 50, 50 };
	elem->BackgroundColor = Colors::Purple;
	Grid grid = MakeGrid({ 100, 0, 0 }, { 0, 200, 0 });
	grid->BackgroundColor = Colors::Red;
	grid->Margin = { 5, 5, 5, 5 };
	grid->Set(0, 0, elem);


	elem->BeginAnimation(
		elem,
		&_Element::BackgroundColor,
		elem->BackgroundColor,
		Colors::Blue,
		10000
	);
	
	elem->Drag += [](Element sender, MouseEventArgs args) {
		sender->Margin.Top += args.offset.Y;
		sender->Margin.Left += args.offset.X;
		Renderer::Invalidated() = true;
	};

	Renderer::MainLoop(grid);
}

