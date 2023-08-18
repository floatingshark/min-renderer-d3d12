#pragma once
#include <iostream>
#include <vector>
#include <cassert>

namespace arabesque
{
	class Mesh
	{
	public:
		typedef struct Vertex
		{
			float Position[3];
			float Color[4];
		} Vertex;

	public:
		static void create_plane(std::vector<Vertex>& out_vertices, std::vector<int>& out_indices)
		{
			out_vertices = {
				{{-0.5f, 0.5f, 0.f}, {1.f, 0.f, 0.f, 1.f}},
				{{0.5f, 0.5f, 0.f}, {0.f, 1.f, 0.f, 1.f}},
				{{0.5f, -0.5f, 0.f}, {0.f, 0.f, 1.f, 1.f}},
				{{-0.5f, -0.5f, 0.f}, {1.f, 1.f, 1.f, 1.f}}};
			out_indices = {0, 1, 3, 1, 2, 3};
		}
		static void create_cube(Vertex out_vertices[8])
		{
			Vertex vertices[8] = {
				{{-0.5f, -0.5f, -0.5f}, {1.f, 0.f, 0.f, 1.f}},
				{{-0.5f, -0.5f, -0.5f}, {0.f, 1.f, 0.f, 1.f}},
				{{-0.5f, -0.5f, -0.5f}, {0.f, 0.f, 1.f, 1.f}},
				{{-0.5f, -0.5f, -0.5f}, {1.f, 1.f, 1.f, 1.f}},
				{{-0.5f, -0.5f, -0.5f}, {1.f, 0.f, 0.f, 1.f}},
				{{-0.5f, -0.5f, -0.5f}, {0.f, 1.f, 0.f, 1.f}},
				{{-0.5f, -0.5f, -0.5f}, {0.f, 0.f, 1.f, 1.f}},
				{{-0.5f, -0.5f, -0.5f}, {1.f, 1.f, 1.f, 1.f}}};
			out_vertices = vertices;
		}
	};
}