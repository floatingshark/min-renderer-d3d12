#pragma once
#include <cassert>
#include <d3d12.h>
#include <fstream>
#include <iostream>
#include <vector>

namespace albedos {
	class Texture {
	public:
		enum struct Type { Monochrome, CheckerBoard, Image, Max };

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
		static std::vector<byte> create_checker_board(int tex_size, byte color[4], int checker_num) {
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
		static void read_bmp_file(const char* file_name, std::vector<byte>& out_texture) {

			int			  i;
			FILE*		  file = fopen(file_name, "rb");
			unsigned char info[54];

			if (!file) {
				const char* name = strlen(file_name) == 0 ? "empty file name" : file_name;
				std::cout << "[Texture]Read Error : " << name << std::endl;
				return;
			}

			// read the 54-byte header
			fread(info, sizeof(unsigned char), 54, file);

			// extract image height and width from header
			int width  = *(int*)&info[18];
			int height = *(int*)&info[22];

			std::cout << "[Texture]Name  : " << file_name << std::endl;
			std::cout << "[Texture]Width : " << width << std::endl;
			std::cout << "[Texture]Height: " << height << std::endl;

			// allocate 3 bytes per pixel
			int			   size = 3 * width * height;
			unsigned char* data = new unsigned char[size];

			// read the rest of the data at once
			fread(data, sizeof(unsigned char), size, file);
			fclose(file);

			out_texture.clear();

			for (i = 0; i < size; i += 3) {
				// flip the order of every 3 bytes
				out_texture.push_back(data[i]);
				out_texture.push_back(data[i + 1]);
				out_texture.push_back(data[i + 2]);
				out_texture.push_back(255);
			}
		}
		static void read_bmp_cube_file(const char* file_name, std::vector<std::vector<byte>>& out_cubes,
									   const int texture_size) {

			int			  i;
			FILE*		  file = fopen(file_name, "rb");
			unsigned char info[54];

			if (!file) {
				std::cout << "[Texture]Read Error : " << file_name << std::endl;
				return;
			}

			// read the 54-byte header
			fread(info, sizeof(unsigned char), 54, file);

			// extract image height and width from header
			int width  = *(int*)&info[18];
			int height = *(int*)&info[22];

			std::cout << "[Texture]Name  : " << file_name << std::endl;
			std::cout << "[Texture]Width : " << width << std::endl;
			std::cout << "[Texture]Height: " << height << std::endl;

			// allocate 3 bytes per pixel
			int			   size = 3 * width * height;
			unsigned char* data = new unsigned char[size];

			// read the rest of the data at once
			fread(data, sizeof(unsigned char), size, file);
			fclose(file);

			out_cubes.clear();
			out_cubes.resize(6);

			for (i = 0; i < size; i += 3) {
				const int x_coord = (i / 3) % (texture_size * 6);
				const int num	  = floor(x_coord / texture_size);

				if (num == 2 || num == 3) {
					const int pseudo_num = num == 2 ? 3 : 2;
					out_cubes[pseudo_num].push_back(data[i + 2]); // いつか何とかする
					out_cubes[pseudo_num].push_back(data[i + 1]);
					out_cubes[pseudo_num].push_back(data[i + 0]);
					out_cubes[pseudo_num].push_back(255);
				} else {
					out_cubes[num].push_back(data[i + 2]);
					out_cubes[num].push_back(data[i + 1]);
					out_cubes[num].push_back(data[i + 0]);
					out_cubes[num].push_back(255);
				}
			}
		}
	};
} // namespace albedos