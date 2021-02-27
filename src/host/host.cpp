#include <cstdio>

#include "host.hpp"
#include "common.hpp"

E64::host_t::host_t()
{
	printf("[host] E64-II (C)%i by elmerucr - version %i.%i.%i\n",
	       E64_SQ_YEAR, E64_SQ_MAJOR_VERSION, E64_SQ_MINOR_VERSION,
	       E64_SQ_BUILD);
	
	video = new video_t();
}

E64::host_t::~host_t()
{
	printf("[host] closing E64-SQ\n");
	
	delete video;
}
