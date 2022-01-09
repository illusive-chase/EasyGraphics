#ifndef RENDER_HH_
#define RENDER_HH_

#include <algorithm>
#include "LinearType.hh"
#include "Timer.hh"

namespace easy {


	struct Renderer {
		static void Render() {
			RegisterRender(nullptr)();
		}

		static bool& Invalidated() {
			static bool invalidate = true;
			return invalidate;
		}

		static auto RegisterData(uint8_t* (*setter)()) ->uint8_t* (*)() {
			static auto instance = setter;
			return instance;
		}

		static auto RegisterRender(void (*setter)()) -> void(*)() {
			static auto instance = setter;
			return instance;
		}

		static auto RegisterMouseClick(int* (*setter)()) -> int*(*)() {
			static auto instance = setter;
			return instance;
		}

		static auto RegisterMouseMove(int* (*setter)()) -> int*(*)() {
			static auto instance = setter;
			return instance;
		}

		static void RegisterSize(int& width, int& height) {
			static int m_width = 0;
			static int m_height = 0;
			if (m_width) width = m_width;
			else m_width = width;
			if (m_height) height = m_height;
			else m_height = height;
		}

		

		static uint8_t* Data() {
			return RegisterData(nullptr)();
		}

		static void FillPixel(uint8_t* data, Color c) {
			data[0] = c.Blue;
			data[1] = c.Green;
			data[2] = c.Red;
			data[3] = c.Alpha;
		}

		static void DrawFilledRect(Rect r, Color c) {
			Size size = {};
			RegisterSize(size.Width, size.Height);
			Rect t = r.ClipTo(size);
			if (t.Left >= t.Right || t.Top >= t.Bottom) return;
			uint8_t* data = Data();
			size_t line_size = size.Width * size_t(4);
			for (int i = t.Left; i < t.Right; ++i) {
				for (int j = t.Top; j < t.Bottom; ++j) {
					FillPixel(data + (j * line_size + i * size_t(4)), c);
				}
			}
		}

		static void DrawRect(Rect r, Color c, Rect thickness) {
			if (!thickness.Left && !thickness.Right && !thickness.Top && !thickness.Bottom)
				return;
			Rect border = {
				r.Left - thickness.Left,
				r.Top - thickness.Top,
				r.Right + thickness.Right,
				r.Bottom + thickness.Bottom
			};
			Size size = {};
			RegisterSize(size.Width, size.Height);
			Rect t = r.ClipTo(size);
			border = border.ClipTo(size);
			if (t.Left >= t.Right || t.Top >= t.Bottom) return;
			DrawFilledRect({ border.Left, border.Top, t.Left, border.Bottom }, c);
			DrawFilledRect({ t.Right, border.Top, border.Right, border.Bottom }, c);
			DrawFilledRect({ border.Left, t.Bottom, border.Right, border.Bottom }, c);
			DrawFilledRect({ border.Left, border.Top, border.Right, t.Top }, c);
		}

		static void DrawByMask(Rect r, Color c, const uint8_t* mask, Size shape) {
			Size size = {};
			RegisterSize(size.Width, size.Height);
			Rect t = r.ClipTo(size);
			if (t.Left >= t.Right || t.Top >= t.Bottom) return;
			uint8_t* data = Data();
			size_t line_size = size.Width * size_t(4);
			for (int i = t.Left; i < t.Right && i < t.Left + shape.Width; ++i) {
				for (int j = t.Top; j < t.Bottom && j < t.Top + shape.Height; ++j) {
					int index = (i - t.Left) + (j - t.Top) * ((shape.Width + 7) / 8 * 8);
					if ((mask[index / 8] >> (7 - (index % 8))) & 0x1) {
						FillPixel(data + (j * line_size + i * size_t(4)), c);
					}
				}
			}
		}

		template<typename T>
		static void MainLoop(T root, double FPS = 40.0);
	};
}



#endif