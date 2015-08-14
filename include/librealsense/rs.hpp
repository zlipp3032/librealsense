#ifndef LIBREALSENSE_CPP_INCLUDE_GUARD
#define LIBREALSENSE_CPP_INCLUDE_GUARD

#include "rsutil.h"

#include <array>
#include <stdexcept>

namespace rs
{
	// Modify this function if you wish to change error handling behavior from throwing exceptions to logging / callbacks / global status, etc.
	inline void handle_error(rs_error * error)
	{
		std::string message = std::string("error calling ") + rs_get_failed_function(error) + "(...):\n  " + rs_get_error_message(error);
		rs_free_error(error);
		throw std::runtime_error(message);
		// std::cerr << message << std::endl;
	}

	class auto_error
	{
		rs_error *			error;
	public:
							auto_error()															: error() {}
							auto_error(const auto_error &)											= delete;
							~auto_error()															{ if (error) handle_error(error); }
		auto_error &		operator = (const auto_error &)											= delete;
							operator rs_error ** ()													{ return &error; }
	};

    struct intrinsics : rs_intrinsics
    {


		std::array<float,2>	project(const float point[3]) const										{ std::array<float,2> pixel; rs_project_point_to_pixel(point, *this, pixel.data()); return pixel; }
		std::array<float,2>	project_to_texcoord(const float point[3]) const							{ auto pixel = project(point); return {pixel[0]/image_size[0], pixel[1]/image_size[1]}; }
		std::array<float,2>	project_to_rectified(const float point[3]) const						{ std::array<float,2> pixel; rs_project_point_to_rectified_pixel(point, *this, pixel.data()); return pixel; }
		std::array<float,2>	project_to_rectified_texcoord(const float point[3]) const				{ auto pixel = project_to_rectified(point); return {pixel[0]/image_size[0], pixel[1]/image_size[1]}; }
		std::array<float,3>	deproject_from_rectified(const float pixel[2], float depth) const		{ std::array<float,3> point; rs_deproject_rectified_pixel_to_point(pixel, depth, *this, point.data()); return point; }        	
    };

    struct extrinsics : rs_extrinsics
    {
		std::array<float,3> transform(const float point[3]) const									{ std::array<float,3> p; rs_transform_point_to_point(point, *this, p.data()); return p; }
    };

	class camera
	{
		rs_camera *			cam;
	public:
							camera()																: cam() {}
							camera(rs_camera * cam)													: cam(cam) {}
		explicit			operator bool() const													{ return !!cam; }
        rs_camera *         get_handle() const                                                      { return cam; }

		void				enable_stream(int stream)												{ return rs_enable_stream(cam, stream, auto_error()); }
		int 				configure_streams()														{ return rs_configure_streams(cam, auto_error()); }
		void				start_stream(int stream, int width, int height, int fps, int format)	{ return rs_start_stream(cam, stream, width, height, fps, format, auto_error()); }
        void				start_stream_preset(int stream, int preset)                             { return rs_start_stream_preset(cam, stream, preset, auto_error()); }
		void				stop_stream(int stream)													{ return rs_stop_stream(cam, stream, auto_error()); }
		const uint16_t *	get_depth_image()														{ return rs_get_depth_image(cam, auto_error()); }
		const uint8_t *		get_color_image()														{ return rs_get_color_image(cam, auto_error()); }
		int 				is_streaming()															{ return rs_is_streaming(cam, auto_error()); }
		int					get_camera_index()														{ return rs_get_camera_index(cam, auto_error()); }
		uint64_t			get_frame_count()														{ return rs_get_frame_count(cam, auto_error()); }

		int					get_stream_property_i(int stream, int prop)								{ return rs_get_stream_property_i(cam, stream, prop, auto_error()); }
		float				get_stream_property_f(int stream, int prop)								{ return rs_get_stream_property_f(cam, stream, prop, auto_error()); }

        intrinsics          get_stream_intrinsics(int stream)										{ intrinsics intrin; rs_get_stream_intrinsics(cam, stream, &intrin, auto_error()); return intrin; }
        extrinsics          get_stream_extrinsics(int stream_from, int stream_to)					{ extrinsics extrin; rs_get_stream_extrinsics(cam, stream_from, stream_to, &extrin, auto_error()); return extrin; }
	};

	class context
	{
		rs_context *		ctx;
	public:
							context()																: ctx(rs_create_context(RS_API_VERSION, auto_error())) {}
							context(const auto_error &)												= delete;
							~context()																{ rs_delete_context(ctx, nullptr); } // Deliberately ignore error on destruction
							context & operator = (const context &)									= delete;
        rs_context *        get_handle() const                                                      { return ctx; }

		int					get_camera_count()														{ return rs_get_camera_count(ctx, auto_error()); }
		camera				get_camera(int index)													{ return rs_get_camera(ctx, index, auto_error()); }
	};
}

#endif