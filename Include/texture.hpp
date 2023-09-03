#pragma once
#include <cassert>
#include <d3d12.h>
#include <iostream>
#include <vector>

namespace albedos {
	class Texture {
	public:
		enum struct Type { Monochrome, CheckBoard, Max };

		static std::vector<byte> create_monochromatic(int tex_size, byte color[4]) {
			std::vector<byte> ret;
			ret.resize(tex_size * tex_size * 4);

			for (int i = 0; i < tex_size * tex_size; i++) {
				const int index = i * 4;
				ret[index]		= color[0];
				ret[index + 1]	= color[1];
				ret[index + 2]	= color[2];
				ret[index + 3]	= color[3];
			}

			return ret;
		}
		static std::vector<byte> create_checker(int tex_size, byte color[4], int checker_num) {
			std::vector<byte>		ret;
			const std::vector<byte> white	   = {255, 255, 255, 255};
			const std::vector<byte> black	   = {color[0], color[1], color[2], 255};
			const int				check_size = tex_size / checker_num;

			for (int i = 0; i < tex_size; i++) {
				for (int j = 0; j < tex_size; j++) {
					if ((i / check_size + j / check_size) % 2 == 0) {
						ret.insert(ret.end(), white.begin(), white.end());
					} else {
						ret.insert(ret.end(), black.begin(), black.end());
					}
				}
			}
			return ret;
		}
	};
} // namespace albedos