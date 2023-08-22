#pragma once
#include <iostream>
#include <vector>
#include <cassert>

namespace arabesques
{
	class Mesh
	{
	public:
		typedef struct Vertex
		{
			float Position[3];
			float Color[4];
			float Normal[3];
		} Vertex;

	public:
		static void create_plane(std::vector<Vertex> &out_vertices, std::vector<int> &out_indices)
		{
			out_vertices = {
				{{-1.f, 1.f, 0.f}, {1.f, 0.f, 0.f, 1.f}, {0.f, 0.f, 1.f}},
				{{1.f, 1.f, 0.f}, {0.f, 1.f, 0.f, 1.f}, {0.f, 0.f, 1.f}},
				{{1.f, -1.f, 0.f}, {0.f, 0.f, 1.f, 1.f}, {0.f, 0.f, 1.f}},
				{{-1.f, -1.f, 0.f}, {1.f, 1.f, 1.f, 1.f}, {0.f, 0.f, 1.f}}};
			out_indices = {0, 1, 3, 1, 2, 3};
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
	};
}