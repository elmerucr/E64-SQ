#include <cstdio>

#include "host.hpp"
#include "common.hpp"

E64::host_t::host_t()
{
	printf("[host] E64 (C)%i by elmerucr - version %i.%i.%i\n",
	       E64_YEAR, E64_MAJOR_VERSION, E64_MINOR_VERSION,
	       E64_BUILD);
	
	video = new video_t();
}

E64::host_t::~host_t()
{
	printf("[host] closing E64\n");
	
	delete video;
}
