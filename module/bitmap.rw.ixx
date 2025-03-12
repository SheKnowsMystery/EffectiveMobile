module;

#include <windows.h>
#include <wingdi.h>

#include <algorithm>
#include <concepts>
#include <cstdlib>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <string>
#include <type_traits>

#undef max
#undef min

export module bitmap.rw;

namespace bmp::util
{
	void check(const std::istream& _is)
	{
		if (_is.eof())
			throw std::runtime_error("![ unexpected end of file ]");

		if (_is.fail())
			throw std::runtime_error("![ failed to read from stream ]");
	}

	template<std::default_initializable _Type>
	auto read(std::istream& _is) -> _Type
	{
		_Type data = {};

		char*           buffer;
		std::streamsize size;

		buffer = reinterpret_cast<char*>(&data);
		size   = sizeof(_Type);

		_is.read(buffer, size);

		check(_is);

		return data;
	}

	template <typename _Type>
	void read(std::istream& _is, _Type* _data, std::size_t _size)
	{
		char*           buffer;
		std::streamsize size;

		buffer = reinterpret_cast<char*>(_data);
		size   = _size * sizeof(_Type);

		_is.read(buffer, size);

		check(_is);
	}

	void check(const std::ostream& _os)
	{
		if (_os.eof())
			throw std::runtime_error("![ unexpected end of file while reading from stream ]");

		if (_os.fail())
			throw std::runtime_error("![ failed to read from stream ]");
	}

	template <typename _Type>
	void write(std::ostream& _os, _Type& _data)
	{
		char*           buffer;
		std::streamsize size;

		buffer = reinterpret_cast<char*>(&_data);
		size   = sizeof(_Type);

		_os.write(buffer, size);

		check(_os);
	}

	template <typename _Type>
	void write(std::ostream& _os, _Type* _data, std::size_t _size)
	{
		char*           buffer;
		std::streamsize size;

		buffer = reinterpret_cast<char*>(_data);
		size   = _size * sizeof(_Type);

		_os.write(buffer, size);

		check(_os);
	}

	auto normalize(unsigned char _number) -> std::string
	{
		return std::to_string(+_number + 1000).substr(1);
	}

	struct MatrixSize
	{
		std::size_t columns;
		std::size_t rows;

		operator std::size_t() const
		{ return columns * rows; }
	};

	template <typename _Type>
	class MatrixWrapper
	{
		_Type* const flatten = nullptr;

		const MatrixSize size = {};

		class Proxy
		{
			const MatrixWrapper* matrix;

			std::size_t row;

		public:
			Proxy(const MatrixWrapper* _matrix, std::size_t _row) :
				matrix(_matrix), row(_row) {}

			auto operator[](std::size_t _column) const -> _Type&
			{ return matrix->flatten[(matrix->size.columns * row) + _column]; }

			operator _Type*() const
			{ return matrix->flatten + (matrix->size.columns * row); }
		};

	public:
		auto operator[](std::size_t _row) const -> Proxy
		{ return Proxy(this, _row); }

		MatrixWrapper() = default;

		MatrixWrapper(_Type* _data, MatrixSize _size) :
			flatten(_data), size(_size) {}

		MatrixWrapper(void* _data, MatrixSize _size) :
			flatten(reinterpret_cast<_Type*>(_data)), size(_size) {}
	};
} // namespace bmp::util

export
namespace bmp::util
{
	struct Point
	{
		std::size_t x, y;
	};
} // namespace bmp::util

namespace bmp::env
{
	constexpr WORD  filetype    = 0x4D42; // 0x4D42 == "BM"
	constexpr WORD  bitcount[]  = {
		24,
		32,
	};
	constexpr DWORD compression = BI_RGB;

	template <typename _RGB>
	concept BMP_RGB = std::same_as<std::remove_cv_t<_RGB>, RGBTRIPLE> || std::same_as<std::remove_cv_t<_RGB>, RGBQUAD>;

	enum class RGB_TYPE : bool
	{
		triple,
		quad,
	};

	enum class Color
	{
		black,
		white,
		other,
	};

	template <BMP_RGB _RGB>
	constexpr _RGB white = {255, 255, 255};
	template <BMP_RGB _RGB>
	constexpr _RGB black = {0, 0, 0};

	bool operator==(RGBTRIPLE _left, RGBTRIPLE _right)
	{
		return
			_left.rgbtBlue  == _right.rgbtBlue &&
			_left.rgbtGreen == _right.rgbtGreen &&
			_left.rgbtRed   == _right.rgbtRed;
	}
	bool operator==(RGBQUAD _left, RGBQUAD _right)
	{
		return
			_left.rgbBlue  == _right.rgbBlue &&
			_left.rgbGreen == _right.rgbGreen &&
			_left.rgbRed   == _right.rgbRed;
	}
} // namespace bmp::env

namespace bmp
{
	using env::operator==;
}

namespace bmp::check
{
	template <env::BMP_RGB _RGB>
	bool black(_RGB _rgb)
	{
		return _rgb == env::black<_RGB>;
	}

	template <env::BMP_RGB _RGB>
	bool white(_RGB _rgb)
	{
		return _rgb == env::white<_RGB>;
	}
} // namespace bmp::check

namespace bmp::impl
{
	struct BitMapData
	{
		util::MatrixSize size;

		char* const flatten = nullptr;

		BitMapData() = default;

		template <env::BMP_RGB _RGB>
		BitMapData(util::MatrixSize _size) :
			size(_size), flatten(new char[size * sizeof(_RGB)]) {}

		BitMapData(util::MatrixSize _size, std::size_t _rgb_size) :
			size(_size), flatten(new char[size * _rgb_size]) {}

		~BitMapData()
		{ delete[] flatten; }

		template <env::BMP_RGB _RGB>
		auto allocate(util::MatrixSize _size) -> BitMapData&
		{
			this->~BitMapData();
			new (this) BitMapData(_size, sizeof(_RGB));
			return *this;
		}

		template <env::BMP_RGB _RGB>
		auto matrix() -> util::MatrixWrapper<_RGB>
		{ return {flatten, size}; }

		template <env::BMP_RGB _RGB>
		auto matrix() const -> util::MatrixWrapper<const _RGB>
		{ return {flatten, size}; }
	};
} // namespace bmp::impl

namespace bmp::get
{
	auto rgb_type(std::size_t _bitcount) -> env::RGB_TYPE
	{
		switch (_bitcount)
		{
		case 24:
			return env::RGB_TYPE::triple;
		case 32:
			return env::RGB_TYPE::quad;
		default:
			throw std::logic_error("![ unsupported RGB bit depth ]");
		}
	}

	template <env::BMP_RGB _RGB>
	auto color(_RGB _rgb) -> env::Color
	{
		if (check::black(_rgb))
			return env::Color::black;
		if (check::white(_rgb))
			return env::Color::white;
		return env::Color::other;
	}

	auto string(RGBTRIPLE _color) -> std::string
	{
		return util::normalize(_color.rgbtBlue) + ';' + util::normalize(_color.rgbtGreen) + ';' + util::normalize(_color.rgbtRed);
	}

	auto string(RGBQUAD _color) -> std::string
	{
		return util::normalize(_color.rgbBlue) + ';' + util::normalize(_color.rgbGreen) + ';' + util::normalize(_color.rgbRed);
	}

	auto pixel(env::Color _color) -> const char*
	{
		switch (_color)
		{
			using enum env::Color;
		case black:
			return "[ ]";
		case white:
			return "[*]";
		case other:
		default:
			return "[?]";
		}
	}
} // namespace bmp::get

export
namespace bmp
{
	class BitMapRW
	{
		BITMAPFILEHEADER fileheader = {};
		BITMAPINFOHEADER infoheader = {};

		impl::BitMapData data;

		struct Dispatch {
			void (BitMapRW::*read)  (std::istream&)            = nullptr;
			void (BitMapRW::*print) (bool) const               = nullptr;
			bool (BitMapRW::*draw)  (util::Point, util::Point) = nullptr;
			void (BitMapRW::*write) (std::ostream&)            = nullptr;

			template <env::BMP_RGB _RGB>
			static auto get() -> Dispatch
			{
				return {
					.read  = &BitMapRW::read<_RGB>,
					.print = &BitMapRW::print<_RGB>,
					.draw  = &BitMapRW::draw<_RGB>,
					.write = &BitMapRW::write<_RGB>,
				};
			}
			static auto get(env::RGB_TYPE _type) -> Dispatch
			{
				switch (_type)
				{
				case env::RGB_TYPE::triple:
					return get<RGBTRIPLE>();
				case env::RGB_TYPE::quad:
					return get<RGBQUAD>();
				}
			}
		} dispatch;

		auto normalize(util::Point _point) const -> util::Point
		{
			return {
				.x = _point.x,
				.y = data.size.rows - _point.y - 1,
			};
		}

		template <env::BMP_RGB _RGB>
		void read(std::istream& _is)
		{
			auto size = util::MatrixSize{
				.columns = static_cast<std::size_t>(std::abs(infoheader.biWidth)),
				.rows    = static_cast<std::size_t>(std::abs(infoheader.biHeight)),
			};

			auto bitcount = infoheader.biBitCount;

			data.allocate<_RGB>(size);

			std::streamoff padding = 4 - (bitcount / 8 * size.columns % 4);

			auto matrix = data.matrix<_RGB>();

			for (std::size_t row = 0; row < size.rows; row++)
			{
				util::read<_RGB>(_is, matrix[row], size.columns);
				_is.seekg(padding, std::ios::cur);
			}
		}

		template <env::BMP_RGB _RGB>
		void print(bool _raw = false) const
		{
			auto matrix = data.matrix<_RGB>();

			if (_raw)
				for (std::size_t row = 0; row < data.size.rows; row++)
				{
					for (std::size_t column = 0; column < data.size.columns; column++)
					{
						auto color = matrix[data.size.rows - row - 1][column];
						std::cout << get::string(color) << ' ';
					}
					std::cout << std::endl;
				}
			else
				for (std::size_t row = 0; row < data.size.rows; row++)
				{
					for (std::size_t column = 0; column < data.size.columns; column++)
						std::cout << get::pixel(get::color(matrix[data.size.rows - row - 1][column]));
					std::cout << std::endl;
				}
		}

		template <env::BMP_RGB _RGB>
		bool draw(util::Point _begin, util::Point _end)
		{
			_begin = normalize(_begin);
			_end   = normalize(_end);

			auto begin = util::Point{
				.x = std::min(_begin.x, _end.x),
				.y = std::min(_begin.y, _end.y),
			};
			auto end = util::Point{
				.x = std::max(_begin.x, _end.x),
				.y = std::max(_begin.y, _end.y),
			};

			if (end.x >= data.size.columns || end.y >= data.size.rows)
				return false;

			auto matrix = data.matrix<_RGB>();

			// Bresenham's line algorithm implementation

			const std::size_t rows    = end.y - begin.y + 1;
			const std::size_t columns = end.x - begin.x + 1;

			std::size_t row    = 0;
			std::size_t column = 0;

			std::size_t error = 0;

			const std::size_t increment = std::min(rows, columns);
			const std::size_t boundary  = std::max(rows, columns);

			std::size_t& step   = rows > columns ? row : column;
			std::size_t& follow = rows > columns ? column : row;

			while (row < rows && column < columns)
			{
				matrix[begin.y + row][begin.x + column] = env::black<_RGB>;
				matrix[begin.y + row][end.x   - column] = env::black<_RGB>;
				error += increment;
				if (error >= boundary)
				{
					error -= boundary;
					follow++;
				}
				step++;
			}

			return true;
		}

		template <env::BMP_RGB _RGB>
		void write(std::ostream& _os)
		{
			auto bitcount = infoheader.biBitCount;

			std::streamoff padding = 4 - (bitcount / 8 * data.size.columns % 4);

			constexpr char buff[4] = {};

			auto matrix = data.matrix<_RGB>();

			for (std::size_t row = 0; row < data.size.rows; row++)
			{
				util::write<_RGB>(_os, matrix[row], data.size.columns);
				_os.write(buff, padding);
			}
		}

	public:
		void read(std::istream& _is)
		{
			fileheader = util::read<BITMAPFILEHEADER>(_is);
			infoheader = util::read<BITMAPINFOHEADER>(_is);

			bool valid = true;

			auto bitcount = infoheader.biBitCount;

			valid &= fileheader.bfType == env::filetype;
			valid &= std::ranges::find(env::bitcount, bitcount) != std::ranges::end(env::bitcount);
			valid &= infoheader.biCompression == env::compression;

			if (!valid)
				throw std::logic_error("![ invalid file ]");

			dispatch = Dispatch::get(get::rgb_type(bitcount));

			auto offset = fileheader.bfOffBits;

			_is.seekg(offset, std::ios::beg);

			if (_is.eof())
				throw std::logic_error("![ unexpected end of file ]");

			std::invoke(dispatch.read, this, _is);
		}

		void print(bool _raw = false) const
		{
			std::invoke(dispatch.print, this, _raw);
		}

		bool draw(util::Point _begin, util::Point _end)
		{
			return std::invoke(dispatch.draw, this, _begin, _end);
		}

		void write(std::ostream& _os)
		{
			util::write<BITMAPFILEHEADER>(_os, fileheader);
			util::write<BITMAPINFOHEADER>(_os, infoheader);

			auto offset = fileheader.bfOffBits;

			_os.seekp(offset, std::ios::beg);

			std::invoke(dispatch.write, this, _os);
		}
	};
} // namespace bmp