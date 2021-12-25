#include "include/OverlapPanel.hh"
#include "include/Grid.hh"
#include "include/Label.hh"
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
	Register<Renderer>();

	int result = 0;
	bool wait_new = true;

    OverlapPanel history = MakeOverlapPanel();
    history->Margin = { 50, 0, 50, 50 };
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
        int total = history->ElementSize();
        for (int i = 0; begin != end; ++i, ++begin) {
            if (i > total - 6)
                (*begin)->BeginAnimation(
                    *begin,
                    &_Element::Margin,
                    (*begin)->Margin,
                    Rect { 0, 60 + 50 * (total - 1 - i) },
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
	
	Label input = MakeLabel();
	input->Margin = { 20 };
	input->SpecSize = { 700, 70 };
	input->BackgroundColor = Color::FromARGB(0xF0F0F0);
	input->FontSize = FontSizeType::LARGE;
	input->FontColor = Colors::Black;
	input->VerticalAlignment = VerticalAlignType::Center;
	input->HorizontalAlignment = HorizontalAlignType::Left;
	input->FontHorizontalAlignment = HorizontalAlignType::Left;
	input->FontVerticalAlignment = VerticalAlignType::Center;

    Label hint = MakeLabel();
    hint->Margin = { 0, 0, 35 };
    hint->SpecSize.Height = 70;
    hint->FontSize = FontSizeType::LARGE;
    hint->VerticalAlignment = VerticalAlignType::Center;
	hint->HorizontalAlignment = HorizontalAlignType::Right;
    hint->Text = " ";

    OverlapPanel hint_panel = MakeOverlapPanel();
    hint_panel->Add(input);
    hint_panel->Add(hint);

	Grid btns = MakeGrid({ 80,80,80,80 }, { 80,80,80,80 });
	btns->Margin = { 20 };


	auto MakeBtn = []() {
		Label btn = MakeLabel();
		btn->SpecSize = { 76, 76 };
		btn->VerticalAlignment = VerticalAlignType::Center;
		btn->HorizontalAlignment = HorizontalAlignType::Center;
		btn->BackgroundColor = Color::FromARGB(0xF0F0F0);
		return btn;
	};

	for (int i = 0; i < 10; ++i) {
		Label btn = MakeBtn();
		btn->Text = std::to_string(i);
		btn->Click += [&, i](Element, MouseEventArgs) {
			if (wait_new) input->Text = "  " + std::to_string(i), wait_new = false;
			else input->Text += std::to_string(i);
			Renderer::Invalidated() = true;
		};
		btn->BackgroundColor = Color::FromARGB(0xFAFAFA);
		if (i) btns->Set((i - 1) / 3, (i - 1) % 3, btn);
		else btns->Set(3, 1, btn);
	}
	

	for (int i = 0; i < 4; ++i) {
		Label btn = MakeBtn();
		btn->Text = "+-*/"[i];
		btn->Click += [&, i](Element, MouseEventArgs) {
			hint->Text[0] = "+-*/"[i];
			wait_new = true;
			result = as_int(input->Text);
			Renderer::Invalidated() = true;
		};
		btns->Set(i, 3, btn);
	}

	Label eq = MakeBtn();
	eq->Text = "=";
	eq->Click += [&](Element, MouseEventArgs) {
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
	btns->Set(3, 0, eq);

	Label cl = MakeBtn();
	cl->Text = "C";
	cl->Click += [&](Element, MouseEventArgs) {
		input->Text = "  0";
		hint->Text[0] = ' ';
		result = 0;
		wait_new = true;
		Renderer::Invalidated() = true;
	};
	btns->Set(3, 2, cl);    
    

    Grid panel = MakeGrid({ 0 }, { 320, 0 });
    panel->Set(0, 0, btns);
    panel->Set(0, 1, history);

	Grid form = MakeGrid({ 130, 0 }, { 0 });
	form->BackgroundColor = Color::FromARGB(0xE6E6E6);
	form->Set(0, 0, hint_panel);
	form->Set(1, 0, panel);
	Renderer::MainLoop(form);
}
