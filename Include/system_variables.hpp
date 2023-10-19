#pragma once
#include <iostream>

namespace albedo {
	class System {
	public:
		static float bg_color[4];
		static int	 texture_size;

		static float light_position[3];
		static float light_ambient[4];
		static float light_intensity;

		static int	num_frames_in_fright;
		static bool is_warp_device;
		static int	max_num_of_entity;
		static int	cbv_srv_buffer_size;
		static int	cbuffer_size;

		static bool	 is_enabled_shadow_mapping;
		static float shadow_mapping_bias;
		static bool	 is_enabled_msaa;
		static bool	 is_enabled_postprocess;
		static bool	 is_enabled_skydome;
	};

	float System::bg_color[4]  = {0.725f, 0.788f, 0.788f, 1.f};
	int	  System::texture_size = 1024;

	int	 System::num_frames_in_fright = 2;
	bool System::is_warp_device		  = false;
	int	 System::max_num_of_entity	  = 5;
	int	 System::cbuffer_size		  = 512;
	int	 System::cbv_srv_buffer_size  = 5;

	float System::light_position[3]			= {6.f, 2.f, 8.0f};
	float System::light_ambient[4]			= {0.1f, 0.1f, 0.1f, 1.f};
	float System::light_intensity			= 1.f;
	bool  System::is_enabled_shadow_mapping = true;
	float System::shadow_mapping_bias		= 0.00015f;
	bool  System::is_enabled_msaa			= true;
	bool  System::is_enabled_postprocess	= false;
	bool  System::is_enabled_skydome		= true;
} // namespace albedo