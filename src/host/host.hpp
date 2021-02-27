#ifndef HOST_HPP
#define HOST_HPP

#include "settings.hpp"
#include "video.hpp"

namespace E64
{

class host_t {
private:
public:
	host_t();
	~host_t();
	
	settings_t settings;
	video_t *video;
};

}

#endif
