#pragma once

#if BUILD_RELEASE
	#define LOG(level, format, ...)
	#define ERROR(level, format, ...)
#else
	#define LOG(level, format, ...) \
		fprintf(stdout, "[" #level "] " format "\n", ##__VA_ARGS__);

	#define ERROR(level, format, ...) \
		fprintf(stderr, "[" #level "]" __FILE__ ":%u: " format "\n", __LINE__, ##__VA_ARGS__);
#endif