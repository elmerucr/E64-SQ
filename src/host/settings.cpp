//  settings.cpp
//  E64
//
//  Copyright © 2021 elmerucr. All rights reserved.

#include "settings.hpp"
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>

extern uint8_t kernel[];

E64::settings_t::settings_t()
{
	snprintf(settings_path, 256, "%s", getenv("HOME"));     // within apple app container this returns a long path

	// shorten the path name to home and user dir
	char *iterator = settings_path;
	int times = 0;
	while (times < 3) {
		if((*iterator == '/') || (*iterator == '\0')) times++;
		iterator++;
	}
	iterator--;
	*iterator = '\0';   // force '\0' character to terminate string to copy to 'home_dir'
    
	strcpy(home_dir, settings_path);
	strcpy(current_path, home_dir);    // current path defaults to homedir
	printf("[Settings] user home directory: %s\n", home_dir);

	snprintf(iterator, 256, "/.E64");
    
	printf("[Settings] opening settings directory: %s\n", settings_path);
	settings_directory = opendir(settings_path);
	if (settings_directory == NULL) {
		printf("[Settings] error: directory doesn't exist. Trying to make it...\n");
		mkdir(settings_path, 0777);
		settings_directory = opendir(settings_path);
	}

	snprintf(path_to_kernel, 256, "%s/kernel.bin", settings_path);

	// switch to settings path and try to update current_path from settings
	chdir(settings_path);
	update_current_path_from_settings();
}

E64::settings_t::~settings_t()
{
	write_current_path_to_settings();

	if (settings_directory != NULL) {
		printf("[Settings] closing settings directory: %s\n", settings_path);
		closedir(settings_directory);
	}
}

void E64::settings_t::update_current_path_from_settings()
{
	FILE *temp_file = fopen("PATH", "r");
	if(temp_file) {
		fgets(current_path, 256, temp_file);
		size_t ln = strlen(current_path) - 1;
		if (*current_path && current_path[ln] == '\n')
			current_path[ln] = '\0';
		printf("[Settings] current path is now: %s\n", current_path);
		fclose(temp_file);
		chdir(current_path);
	} else {
		printf("[Settings] current path not found in settings, defaulting to: %s\n",
		       current_path);
	}
}

void E64::settings_t::write_current_path_to_settings()
{
	chdir(settings_path);
	FILE *temp_file = fopen("PATH", "w");
	
	if(!temp_file) printf("Error, can't open file 'PATH' for writing\n");

	char *temp_char = current_path;

	while (*temp_char != '\0') {
		fwrite(temp_char, 1, 1, temp_file);
		temp_char++;
	}
	fclose(temp_file);
}
