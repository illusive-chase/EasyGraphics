#include "include/ImGui.hh"
#include "system/SystemIO.hh"
#include <iostream>
#include <string>
using namespace easy;

int as_int(std::string s) {
	const char* str = s.c_str() + 2;
	int x = 0;
	bool neg = false;
	if (str[0] == '-') str++, neg = true;
	while (*str) {
		x = 10 * x + *str - '0';
		str++;
	}
	return neg ? -x : x;
}

int main() {

	int result = 0;
	bool wait_new = true;

	Label input, hint;
	OverlapPanel history;

	auto MakeBtn = []() {
		Label btn = MakeLabel();
		btn->SpecSize = { 76, 76 };
		btn->VerticalAlignment = VerticalAlignType::Center;
		btn->HorizontalAlignment = HorizontalAlignType::Center;
		btn->BackgroundColor = Color::FromARGB(0xF0F0F0);
		return btn;
	};

	auto add_history = [&](std::string info) {
		Label item = MakeLabel();
		item->Text = info;
		item->FontHorizontalAlignment = HorizontalAlignType::Left;
		item->SpecSize = { 340, 40 };
		item->Margin.Top = 10;
		item->BeginAnimation(
			Element(item),
			&_Element::BackgroundColor,
			Color::FromARGB(0xE6E6E6),
			Color::FromARGB(0xF0F0F0),
			500,
			EaseInOutCubic
		);
		item->BeginAnimation(
			item,
			&_Label::FontColor,
			Color::FromARGB(0xE6E6E6),
			Colors::Black,
			500,
			EaseInOutCubic
		);
		auto [begin, end] = history->GetRange();
		int total = history->Capacity();
		for (int i = 0; begin != end; ++i, ++begin) {
			if (i > total - 6)
				(*begin)->BeginAnimation(
					*begin,
					&_Element::Margin,
					(*begin)->Margin,
					Rect {
						0, 60 + 50 * (total - 1 - i)
					},
					500,
					EaseInOutCubic
					);
			else if (i == total - 6) {
				(*begin)->BeginAnimation(
					std::dynamic_pointer_cast<_Label>(*begin),
					&_Label::FontColor,
					Colors::Black,
					Color::FromARGB(0xE6E6E6),
					500,
					EaseInOutCubic
				);
				(*begin)->BeginAnimation(
					*begin,
					&_Element::BackgroundColor,
					(*begin)->BackgroundColor,
					Color::FromARGB(0xE6E6E6),
					500,
					EaseInOutCubic
				);
			}
		}
		history->Add(item);
	};

	using namespace imgui;

	begin_im;
	with (MakeGrid({ 130, 0 }, { 0 })) {
		BackgroundColor = Color::FromARGB(0xE6E6E6);
		with (MakeOverlapPanel()) {
			GridPosition = { 0, 0 };
			with_named (input, MakeLabel()) {
				Margin = { 20 };
				SpecSize = { 700, 70 };
				BackgroundColor = Color::FromARGB(0xF0F0F0);
				FontSize = FontSizeType::Large;
				FontColor = Colors::Black;
				VerticalAlignment = VerticalAlignType::Center;
				HorizontalAlignment = HorizontalAlignType::Left;
				FontHorizontalAlignment = HorizontalAlignType::Left;
				FontVerticalAlignment = VerticalAlignType::Center;
			}
			with_named (hint, MakeLabel()) {
				Margin = { 0, 0, 35 };
				SpecSize = { 0, 70 };
				FontSize = FontSizeType::Large;
				VerticalAlignment = VerticalAlignType::Center;
				HorizontalAlignment = HorizontalAlignType::Right;
				Text = " ";
			}
		}
		with (MakeGrid({ 0 }, { 320, 0 })) {
			GridPosition = { 0, 1 };

			with_named (history, MakeOverlapPanel()) {
				GridPosition = { 1, 0 };
				Margin = { 50, 0, 50, 50 };
			}

			with (MakeGrid({ 80,80,80,80 }, { 80,80,80,80 })) {
				GridPosition = { 0, 0 };
				Margin = { 20 };

				for (int i = 0; i < 10; ++i) {
					with (MakeBtn()) {
						Text = std::to_string(i);
						Click += [&, i](Element, MouseEventArgs) {
							if (wait_new) input->Text = "  " + std::to_string(i), wait_new = false;
							else input->Text += std::to_string(i);
							Renderer::Invalidated() = true;
						};
						BackgroundColor = Color::FromARGB(0xFAFAFA);
						if (i) GridPosition = { (i - 1) % 3, (i - 1) / 3 };
						else GridPosition = { 1, 3 };
					}
				}

				for (int i = 0; i < 4; ++i) {
					with (MakeBtn()) {
						Text = std::string("+-*/").substr(i, 1);
						Click += [&, i](Element, MouseEventArgs) {
							hint->Text[0] = "+-*/"[i];
							wait_new = true;
							result = as_int(input->Text);
							Renderer::Invalidated() = true;
						};
						GridPosition = { 3, i };
					}
				}

				with (MakeBtn()) {
					Text = "=";
					Click += [&](Element, MouseEventArgs) {
						std::string info = std::to_string(result);
						if (hint->Text[0] != ' ') info += hint->Text + input->Text.substr(2);
						else info = input->Text.substr(2);
						if (hint->Text[0] == '+') input->Text = "  " + std::to_string(result + as_int(input->Text));
						else if (hint->Text[0] == '-') input->Text = "  " + std::to_string(result - as_int(input->Text));
						else if (hint->Text[0] == '*') input->Text = "  " + std::to_string(result * as_int(input->Text));
						else if (hint->Text[0] == '/' && as_int(input->Text)) input->Text = "  " + std::to_string(result / as_int(input->Text));
						hint->Text[0] = ' ';
						result = as_int(input->Text);
						info += "=" + std::to_string(result);
						add_history(info);
						wait_new = true;
						Renderer::Invalidated() = true;
					};
					GridPosition = { 0, 3 };
				}

				with (MakeBtn()) {
					Text = "C";
					Click += [&](Element, MouseEventArgs) {
						input->Text = "  0";
						hint->Text[0] = ' ';
						result = 0;
						wait_new = true;
						Renderer::Invalidated() = true;
					};
					GridPosition = { 2, 3 };
				}
			}
		}
	}

	
}
