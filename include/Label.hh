#ifndef LABEL_HH_
#define LABEL_HH_

#include <iostream>
#include "Element.hh"
#include "font/font.cc"

namespace easy {

	enum class FontSizeType {
		VERY_TINY,
		TINY,
		MEDIUM_TINY,
		MEDIAN,
		MEDIUM_LARGE,
		LARGE,
		VERY_LARGE
	};

	struct _Label;

	using Label = std::shared_ptr<_Label>;

	struct _Label : _Element {
	protected:

		const int font_size[7] = {
			16,20,24,28,32,40,48
		};

		const uint8_t* source[7] = {
			font16::data,
			font20::data,
			font24::data,
			font28::data,
			font32::data,
			font40::data,
			font48::data
		};

	public:

		std::string Text;
		Color FontColor = {};
		FontSizeType FontSize = FontSizeType::MEDIAN;
		VerticalAlignType FontVerticalAlignment = VerticalAlignType::Center;
		HorizontalAlignType FontHorizontalAlignment = HorizontalAlignType::Center;

		void Measure(Size size) {
			Size fsize = SpecSize;
			if (fsize.Width == 0) fsize.Width = font_size[static_cast<int>(FontSize)] * static_cast<int>(Text.length()) / 2;
			if (fsize.Height == 0) fsize.Height = font_size[static_cast<int>(FontSize)];
			ActualSize = {
				std::max(0, std::min(size.Width - Margin.Right - Margin.Left, fsize.Width)),
				std::max(0, std::min(size.Height - Margin.Top - Margin.Bottom, fsize.Height))
			};
		}

		void Render() {
			if (!Visible) return;
			_Element::Render();
			Size fsize = Size{ font_size[static_cast<int>(FontSize)] / 2, font_size[static_cast<int>(FontSize)] };
			int len = static_cast<int>(Text.length());
			Pos margin = {};
			if (FontHorizontalAlignment == HorizontalAlignType::Center)
				margin.X = (ActualSize.Width - fsize.Width * len) / 2;
			else if (FontHorizontalAlignment == HorizontalAlignType::Right)
				margin.X = ActualSize.Width - fsize.Width * len;
			if (FontVerticalAlignment == VerticalAlignType::Center)
				margin.Y = (ActualSize.Height - fsize.Height) / 2;
			else if (FontVerticalAlignment == VerticalAlignType::Bottom)
				margin.Y = ActualSize.Height - fsize.Height;
			Rect ActualRect = Rect::BaseOn(ActualPos, ActualSize);
			for (char c : Text) {
				int offset = static_cast<int>(c) * ((fsize.Width + 7) / 8) * fsize.Height;
				Renderer::DrawByMask(Rect::BaseOn(ActualPos + margin, fsize).ClipTo(ActualRect),
									 FontColor,
									 source[static_cast<int>(FontSize)] + offset,
									 fsize);
				margin.X += fsize.Width;
			}
		}



	};


	Label MakeLabel() {
		return Label(new _Label);
	}

}




#endif
