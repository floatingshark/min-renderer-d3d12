#pragma once
#include <cassert>
#include <iostream>
#include <vector>

namespace albedos {
	class Global {
	public:
		static float bg_color[4];
		static float view_position[3];
		static float view_lookat[3];
		static float view_up[3];
		static float projection_FOV;
		static float light_position[3];
		static float light_ambient[4];
		static float light_intensity;
		static bool	 is_enabled_shadow_mapping;
		static float shadow_mapping_bias;

		static bool is_enabled_msaa;
		static bool is_enabled_postprocess;
	};

	float Global::bg_color[4]				= {0.725f, 0.788f, 0.788f, 1.f};
	float Global::view_position[3]			= {-0.f, -6.f, 4.f};
	float Global::view_lookat[3]			= {0.f, 0.f, 1.5f};
	float Global::view_up[3]				= {0.f, 0.f, 1.f};
	float Global::projection_FOV			= 60.f;
	float Global::light_position[3]			= {6.f, 2.f, 8.0f};
	float Global::light_ambient[4]			= {0.1f, 0.1f, 0.1f, 1.f};
	float Global::light_intensity			= 1.f;
	bool  Global::is_enabled_shadow_mapping = true;
	float Global::shadow_mapping_bias		= 0.00015f;

	bool Global::is_enabled_msaa		= true;
	bool Global::is_enabled_postprocess = false;
} // namespace albedos