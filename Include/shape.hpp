#pragma once
#include <iostream>
#include <cassert>
#include <math.h>
#include <vector>

namespace albedos
{
	class Shape
	{
	public:
		typedef struct Vertex
		{
			float Position[3];
			float Color[4];
			float Normal[3];
			float UV[2];
		} Vertex;

		enum struct Type
		{
			Plane,
			Cube,
			Torus,
			Max
		};

	public:
		static void create_plane(std::vector<Vertex> &out_vertices, std::vector<int> &out_indices)
		{
			out_vertices = {
				{{-1.f, -1.f, 0.f}, {1.f, 0.f, 0.f, 1.f}, {0.f, 0.f, 1.f}, {0.f, 0.f}},
				{{-1.f, 1.f, 0.f}, {0.f, 1.f, 0.f, 1.f}, {0.f, 0.f, 1.f}, {0.f, 1.f}},
				{{1.f, 1.f, 0.f}, {0.f, 0.f, 1.f, 1.f}, {0.f, 0.f, 1.f}, {1.f, 1.f}},
				{{1.f, -1.f, 0.f}, {1.f, 1.f, 1.f, 1.f}, {0.f, 0.f, 1.f}, {1.f, 0.f}},
				{{1.f, -1.f, 0.f}, {1.f, 1.f, 1.f, 1.f}, {0.f, 0.f, 1.f}, {1.f, 0.f}}}; // something goes wrong
			out_indices = {0, 1, 2, 0, 2, 3};
		}
		static void create_cube(std::vector<Vertex> &out_vertices, std::vector<int> &out_indices)
		{

			std::vector<std::vector<float>> positions = {
				// Left
				{-1.0f, -1.0f, -1.0f},
				{-1.0f, -1.0f, 1.0f},
				{-1.0f, 1.0f, 1.0f},
				{-1.0f, 1.0f, -1.0f},
				// Back
				{1.0f, -1.0f, -1.0f},
				{-1.0f, -1.0f, -1.0f},
				{-1.0f, 1.0f, -1.0f},
				{1.0f, 1.0f, -1.0f},
				// Bottom
				{-1.0f, -1.0f, -1.0f},
				{1.0f, -1.0f, -1.0f},
				{1.0f, -1.0f, 1.0f},
				{-1.0f, -1.0f, 1.0f},
				// Right
				{1.0f, -1.0f, 1.0f},
				{1.0f, -1.0f, -1.0f},
				{1.0f, 1.0f, -1.0f},
				{1.0f, 1.0f, 1.0f},
				// Top
				{-1.0f, 1.0f, -1.0f},
				{-1.0f, 1.0f, 1.0f},
				{1.0f, 1.0f, 1.0f},
				{1.0f, 1.0f, -1.0f},
				// Front
				{-1.0f, -1.0f, 1.0f},
				{1.0f, -1.0f, 1.0f},
				{1.0f, 1.0f, 1.0f},
				{-1.0f, 1.0f, 1.0f}};
			std::vector<std::vector<float>> colors = {
				// Left
				{1.0f, 0.0f, 0.0f, 1.0f},
				{0.0f, 0.0f, 1.0f, 1.0f},
				{0.0f, 1.0f, 0.0f, 1.0f},
				{1.0f, 1.0f, 1.0f, 1.0f},
				// Bottom
				{0.0f, 1.0f, 0.0f, 1.0f},
				{1.0f, 0.0f, 0.0f, 1.0f},
				{1.0f, 1.0f, 1.0f, 1.0f},
				{0.0f, 0.0f, 1.0f, 1.0f},
				// Back
				{1.0f, 0.0f, 0.0f, 1.0f},
				{0.0f, 1.0f, 0.0f, 1.0f},
				{1.0f, 1.0f, 1.0f, 1.0f},
				{0.0f, 0.0f, 1.0f, 1.0f},
				// Right
				{1.0f, 1.0f, 1.0f, 1.0f},
				{0.0f, 1.0f, 0.0f, 1.0f},
				{0.0f, 0.0f, 1.0f, 1.0f},
				{1.0f, 0.0f, 0.0f, 1.0f},
				// Top
				{1.0f, 1.0f, 1.0f, 1.0f},
				{0.0f, 1.0f, 0.0f, 1.0f},
				{1.0f, 0.0f, 0.0f, 1.0f},
				{0.0f, 0.0f, 1.0f, 1.0f},
				// Front
				{0.0f, 0.0f, 1.0f, 1.0f},
				{1.0f, 1.0f, 1.0f, 1.0f},
				{1.0f, 0.0f, 0.0f, 1.0f},
				{0.0f, 1.0f, 0.0f, 1.0f},
			};
			std::vector<std::vector<float>> normals = {
				// Left
				{-1.0f, 0.0f, 0.0f},
				{-1.0f, 0.0f, 0.0f},
				{-1.0f, 0.0f, 0.0f},
				{-1.0f, 0.0f, 0.0f},
				// Back
				{0.0f, 0.0f, -1.0f},
				{0.0f, 0.0f, -1.0f},
				{0.0f, 0.0f, -1.0f},
				{0.0f, 0.0f, -1.0f},
				// Bottom
				{0.0f, -1.0f, 0.0f},
				{0.0f, -1.0f, 0.0f},
				{0.0f, -1.0f, 0.0f},
				{0.0f, -1.0f, 0.0f},
				// Right
				{1.0f, 0.0f, 0.0f},
				{1.0f, 0.0f, 0.0f},
				{1.0f, 0.0f, 0.0f},
				{1.0f, 0.0f, 0.0f},
				// Top
				{0.0f, 1.0f, 0.0f},
				{0.0f, 1.0f, 0.0f},
				{0.0f, 1.0f, 0.0f},
				{0.0f, 1.0f, 0.0f},
				// Front
				{0.0f, 0.0f, 1.0f},
				{0.0f, 0.0f, 1.0f},
				{0.0f, 0.0f, 1.0f},
				{0.0f, 0.0f, 1.0f}};
			std::vector<std::vector<int>> index_vector = {
				// 左
				{0, 1, 2},
				{0, 2, 3},
				// 裏
				{4, 5, 6},
				{4, 6, 7},
				// 下
				{8, 9, 10},
				{8, 10, 11},
				// 右
				{12, 13, 14},
				{12, 14, 15},
				// 上
				{16, 17, 18},
				{16, 18, 19},
				// 前
				{20, 21, 22},
				{20, 22, 23}};

			out_vertices.resize(24);
			for (int i = 0; i < out_vertices.size(); i++)
			{
				std::copy(positions[i].begin(), positions[i].end(), out_vertices[i].Position);
				std::copy(colors[i].begin(), colors[i].end(), out_vertices[i].Color);
				std::copy(normals[i].begin(), normals[i].end(), out_vertices[i].Normal);
			}

			for (int j = 0; j < index_vector.size(); j++)
			{
				out_indices.insert(out_indices.end(), index_vector[j].begin(), index_vector[j].end());
			}
		}
		static void create_torus(std::vector<Vertex> &out_vertices, std::vector<int> &out_indices)
		{
			const int radial_div = 50;
			const int tubular_div = 50;
			const float radius = 1.f;
			const float thickness = 0.3f;

			for (int i = 0; i < radial_div; i++)
			{
				for (int j = 0; j < tubular_div; j++)
				{
					Vertex vertex;
					const float u = i * M_PI * 2.0 / radial_div;
					const float v = j * M_PI * 2.0 / tubular_div;
					const float x = (radius + thickness * cos(v)) * cos(u);
					const float y = (radius + thickness * cos(v)) * sin(u);
					const float z = thickness * std::sin(v);
					vertex.Position[0] = x;
					vertex.Position[1] = y;
					vertex.Position[2] = z;

					vertex.Color[0] = cos(v);
					vertex.Color[1] = sin(v);
					vertex.Color[2] = 0.f;
					vertex.Color[3] = 1.f;

					const float n_x = x - radius * cos(u);
					const float n_y = y - radius * sin(u);
					const float norm = sqrt(n_x * n_x + n_y * n_y + z * z);
					vertex.Normal[0] = n_x / norm;
					vertex.Normal[1] = n_y / norm;
					vertex.Normal[2] = z / norm;

					vertex.UV[0] = abs(cos(v));
					vertex.UV[1] = abs(sin(v));

					out_vertices.push_back(vertex);

					// Create Torus Indices
					const int index = i * tubular_div + j;
					std::vector<int> index_vector = {index, index + 1, index + tubular_div, index + 1, index + 1 + tubular_div, index + tubular_div};
					if (j == tubular_div - 1)
					{
						index_vector = {index, i * tubular_div, index + tubular_div, i * tubular_div, (i + 1) * tubular_div, index + tubular_div};
					}
					if (i == radial_div - 1)
					{
						index_vector = {index, index + 1, j, index + 1, j + 1, j};
						if (j == tubular_div - 1)
						{
							index_vector = {index, i * tubular_div, j, i * tubular_div, 0, j};
						}
					}

					out_indices.insert(out_indices.end(), index_vector.begin(), index_vector.end());
				}
			}
		}
	};
}