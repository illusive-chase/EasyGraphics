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
	char op = '\0';
	bool wait_new = true;
	
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
			op = "+-*/"[i];
			wait_new = true;
			result = as_int(input->Text);
			Renderer::Invalidated() = true;
		};
		btns->Set(i, 3, btn);
	}

	Label eq = MakeBtn();
	eq->Text = "=";
	eq->Click += [&](Element, MouseEventArgs) {
		if (op == '+') input->Text = "  " + std::to_string(result + as_int(input->Text));
		else if (op == '-') input->Text = "  " + std::to_string(result - as_int(input->Text));
		else if (op == '*') input->Text = "  " + std::to_string(result * as_int(input->Text));
		else if (op == '/' && as_int(input->Text)) input->Text = "  " + std::to_string(result / as_int(input->Text));
		op = '\0';
		result = as_int(input->Text);
		wait_new = true;
		Renderer::Invalidated() = true;
	};
	btns->Set(3, 0, eq);

	Label cl = MakeBtn();
	cl->Text = "C";
	cl->Click += [&](Element, MouseEventArgs) {
		input->Text = "  0";
		op = '\0';
		result = 0;
		wait_new = true;
		Renderer::Invalidated() = true;
	};
	btns->Set(3, 2, cl);


	Grid form = MakeGrid({ 130, 0 }, { 0 });
	form->BackgroundColor = Color::FromARGB(0xE6E6E6);
	form->Set(0, 0, input);
	form->Set(1, 0, btns);
	Renderer::MainLoop(form);
}
