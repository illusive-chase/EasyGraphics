#ifndef LINEAR_TYPE_HH_
#define LINEAR_TYPE_HH_
#include <cstdint>

namespace easy {


	template<typename T, typename V, unsigned N> struct Linear;

	template<typename T, typename V>
	struct Linear<T, V, 2> {
	private:
		using SV = std::make_signed_t<V>;

	public:

		using linear_length = std::integral_constant<unsigned, 2>;

		constexpr T& operator +=(const T& rhs) {
			auto& [x, y] = *static_cast<T*>(this);
			auto [dx, dy] = rhs;
			x += dx;
			y += dy;
			return *static_cast<T*>(this);
		}

		constexpr T operator +(const T& rhs) const {
			auto [x, y] = *static_cast<const T*>(this);
			auto [dx, dy] = rhs;
			return { static_cast<V>(static_cast<SV>(x + dx)), static_cast<V>(static_cast<SV>(y + dy)) };
		}

		constexpr T& operator -=(const T& rhs) {
			auto& [x, y] = *static_cast<T*>(this);
			auto [dx, dy] = rhs;
			x -= dx;
			y -= dy;
			return *static_cast<T*>(this);
		}

		constexpr T operator -(const T& rhs) const {
			auto [x, y] = *static_cast<const T*>(this);
			auto [dx, dy] = rhs;
			return { static_cast<V>(static_cast<SV>(x - dx)), static_cast<V>(static_cast<SV>(y - dy)) };
		}

		constexpr T& operator *=(double k) {
			auto& [x, y] = *static_cast<T*>(this);
			x *= k;
			y *= k;
			return *static_cast<T*>(this);
		}

		constexpr T operator *(double k) const {
			auto [x, y] = *static_cast<const T*>(this);
			return { static_cast<V>(static_cast<SV>(x * k)), static_cast<V>(static_cast<SV>(y * k)) };
		}

		constexpr bool operator ==(const T& rhs) const {
			auto [x, y] = *static_cast<const T*>(this);
			auto [dx, dy] = rhs;
			return x == dx && y == dy;
		}
		constexpr bool operator !=(const T& rhs) const { return !(*this == rhs); }

	};

	template<typename T, typename V>
	struct Linear<T, V, 4> {
	private:
		using SV = std::make_signed_t<V>;

	public:

		using linear_length = std::integral_constant<unsigned, 4>;

		constexpr T& operator +=(const T& rhs) {
			auto& [x, y, z, w] = *static_cast<T*>(this);
			auto [dx, dy, dz, dw] = rhs;
			x += dx;
			y += dy;
			z += dz;
			w += dw;
			return *static_cast<T*>(this);
		}

		constexpr T operator +(const T& rhs) const {
			auto [x, y, z, w] = *static_cast<const T*>(this);
			auto [dx, dy, dz, dw] = rhs;
			return { static_cast<V>(static_cast<SV>(x + dx)), static_cast<V>(static_cast<SV>(y + dy)), static_cast<V>(static_cast<SV>(z + dz)), static_cast<V>(static_cast<SV>(w + dw)) };
		}

		constexpr T& operator -=(const T& rhs) {
			auto& [x, y, z, w] = *static_cast<T*>(this);
			auto [dx, dy, dz, dw] = rhs;
			x -= dx;
			y -= dy;
			z -= dz;
			w -= dw;
			return *static_cast<T*>(this);
		}

		constexpr T operator -(const T& rhs) const {
			auto [x, y, z, w] = *static_cast<const T*>(this);
			auto [dx, dy, dz, dw] = rhs;
			return { static_cast<V>(static_cast<SV>(x - dx)), static_cast<V>(static_cast<SV>(y - dy)), static_cast<V>(static_cast<SV>(z - dz)), static_cast<V>(static_cast<SV>(w - dw)) };
		}

		constexpr T& operator *=(double k) {
			auto& [x, y, z, w] = *static_cast<T*>(this);
			x *= k;
			y *= k;
			z *= k;
			w *= k;
			return *static_cast<T*>(this);
		}

		constexpr T operator *(double k) const {
			auto [x, y, z, w] = *static_cast<const T*>(this);
			return { static_cast<V>(static_cast<SV>(x * k)), static_cast<V>(static_cast<SV>(y * k)), static_cast<V>(static_cast<SV>(z * k)), static_cast<V>(static_cast<SV>(w * k)) };
		}

		constexpr bool operator ==(const T& rhs) const {
			auto [x, y, z, w] = *static_cast<const T*>(this);
			auto [dx, dy, dz, dw] = rhs;
			return x == dx && y == dy && z == dz && w == dw;
		}
		constexpr bool operator !=(const T& rhs) const { return !(*this == rhs); }

	};

	template<typename T, typename V = typename T::linear_length>
	struct linear_length : V {};

	template<typename T>
	struct linear_length<T, void> : std::integral_constant<unsigned, 0> {};

	template<typename T, std::enable_if_t<linear_length<T>::value == 1, int> I = 0>
	static constexpr T& assign_as_mixture(T& dest, double ratio1, const T& s1, const T& s2) {
		double ratio2 = 1 - ratio1;
		dest = static_cast<T>(ratio1 * s1 + ratio2 * s2);
		return dest;
	}

	template<typename T, std::enable_if_t<linear_length<T>::value == 2, int> I = 0>
	static constexpr T& assign_as_mixture(T& dest, double ratio1, const T& s1, const  T& s2) {
		auto [x1, y1] = s1;
		auto [x2, y2] = s2;
		auto& [x, y] = dest;
		double ratio2 = 1 - ratio1;
		x = static_cast<decltype(x)>(ratio1 * x1 + ratio2 * x2);
		y = static_cast<decltype(y)>(ratio1 * y1 + ratio2 * y2);
		return dest;
	}

	template<typename T, std::enable_if_t<linear_length<T>::value == 4, int> I = 0>
	static constexpr T& assign_as_mixture(T& dest, double ratio1, const T& s1, const  T& s2) {
		auto [x1, y1, z1, w1] = s1;
		auto [x2, y2, z2, w2] = s2;
		auto& [x, y, z, w] = dest;
		double ratio2 = 1 - ratio1;
		x = static_cast<decltype(x)>(ratio1 * x1 + ratio2 * x2);
		y = static_cast<decltype(y)>(ratio1 * y1 + ratio2 * y2);
		z = static_cast<decltype(z)>(ratio1 * z1 + ratio2 * z2);
		w = static_cast<decltype(w)>(ratio1 * w1 + ratio2 * w2);
		return dest;
	}



	template<typename T>
	inline constexpr T fetch(const std::initializer_list<T>& il, size_t pos) {
		return (il.size() <= pos) ? 0 : *(il.begin() + pos);
	}

	struct Color : Linear<Color, uint8_t, 4>{
		uint8_t Alpha, Red, Green, Blue;
		constexpr Color(std::initializer_list<uint8_t>&& il) :
			Alpha(fetch(il, 0)),
			Red(fetch(il, 1)),
			Green(fetch(il, 2)),
			Blue(fetch(il, 3)) {}

		constexpr static Color FromARGB(uint32_t ARGB) {
			return Color{
				static_cast<uint8_t>((ARGB & 0xff000000) >> 24),
				static_cast<uint8_t>((ARGB & 0xff0000) >> 16),
				static_cast<uint8_t>((ARGB & 0xff00) >> 8),
				static_cast<uint8_t>(ARGB & 0xff)
			};
		}
	};

	namespace Colors {
		constexpr Color
			White = Color::FromARGB(0xffffff),
			Black = Color::FromARGB(0x000000),
			Red = Color::FromARGB(0xd71345),
			Blue = Color::FromARGB(0x426ab3),
			Green = Color::FromARGB(0x7fb80e),
			Yellow = Color::FromARGB(0xffd400),
			Purple = Color::FromARGB(0x9b95c9),
			Brown = Color::FromARGB(0x74531f),
			Trasparent = Color::FromARGB(0xff000000);
	}

	

	struct Size : Linear<Size, int, 2> {
		int Width, Height;
		constexpr Size(std::initializer_list<int>&& il) : Width(fetch(il, 0)), Height(fetch(il, 1)) {}
	};

	struct Pos : Linear<Pos, int, 2> {
		int X, Y;
		constexpr Pos(std::initializer_list<int>&& il) : X(fetch(il, 0)), Y(fetch(il, 1)) {}
	};

	struct Rect : Linear<Rect, int, 4> {
		int Left, Top, Right, Bottom;

		constexpr Rect(std::initializer_list<int>&& il) :
			Left(fetch(il, 0)),
			Top(fetch(il, 1)),
			Right(fetch(il, 2)),
			Bottom(fetch(il, 3)) {}

		constexpr Rect ClipTo(Size size) const {
			return {
				std::max(0, Left), std::max(0, Top),
				std::min(size.Width, Right), std::min(size.Height, Bottom)
			};
		}

		constexpr Rect ClipTo(Rect rect) const {
			return {
				std::max(rect.Left, Left), std::max(rect.Top, Top),
				std::min(rect.Right, Right), std::min(rect.Bottom, Bottom)
			};
		}

		constexpr static Rect BaseOn(Pos pos, Size size) {
			return {
				pos.X,
				pos.Y,
				pos.X + size.Width,
				pos.Y + size.Height
			};
		}

	};

}


#endif