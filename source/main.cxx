#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <format>

import bitmap.rw;

namespace filenames
{
	constexpr auto blank    = "./assets/blank.bmp";
	constexpr auto modified = "./assets/modified.bmp";
}

int main()
{
	try
	{
		std::string filename;

		std::cout << "Enter *.bmp image filename to read from: ";
		std::getline(std::cin, filename);

		auto blank = std::ifstream(filename, std::ios::binary);

		if (!blank.is_open())
			throw std::runtime_error(std::format("![ failed to open file \"{}\" ]", filename));

		auto bitmap = bmp::BitMapRW();

		bitmap.read(blank);

		std::cout << std::endl;
		bitmap.print();
		std::cout << std::endl;

		for (;;)
		{
			bmp::util::Point begin, end;

			std::cout << "Enter 2 points (x, y) to draw cross on image: ";
			std::cin >> begin.x >> begin.y >> end.x >> end.y;
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

			if (std::cin.fail())
			{
				std::cout << "Incorrect input!" << std::endl;
				std::cin.clear();
				continue;
			}

			if (bitmap.draw(begin, end))
				break;

			std::cout << "Couldn't draw on this coords!" << std::endl;
		}

		std::cout << std::endl;
		bitmap.print();
		std::cout << std::endl;

		std::cout << "Enter *.bmp image filename to write to: ";
		std::getline(std::cin, filename);

		auto modified = std::ofstream(filename, std::ios::binary | std::ios::trunc);

		if (!blank.is_open())
			throw std::runtime_error(std::format("![ failed to open file \"{}\" ]", filename));

		bitmap.write(modified);
	}
	catch(const std::exception& error)
	{
		std::cerr << error.what() << std::endl;
	}
}