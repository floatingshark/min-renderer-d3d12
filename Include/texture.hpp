#pragma once
#include <iostream>
#include <cassert>
#include <vector>
#include <d3d12.h>

namespace arabesques
{
	class Texture
	{
	public:
		static std::vector<byte> create_checker(int tex_size, int checker_num)
		{
			std::vector<byte> ret;
			const std::vector<byte> white = {255, 255, 255, 255};
			const std::vector<byte> black = {0, 0, 0, 255};
			const int check_size = tex_size / checker_num;

			for (int i = 0; i < tex_size; i++)
			{
				for (int j = 0; j < tex_size; j++)
				{
					if ((i / check_size + j / check_size) % 2 == 0)
					{
						ret.insert(ret.end(), white.begin(), white.end());
					}
					else
					{
						ret.insert(ret.end(), black.begin(), black.end());
					}
				}
			}
			return ret;
		}
	};
}