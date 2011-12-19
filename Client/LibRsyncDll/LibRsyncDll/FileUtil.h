#pragma once
#include <stdio.h>
class FileUtil
{
public:
	FileUtil(void);
	~FileUtil(void);
	static FILE * rs_file_open(char const *filename, char const *mode);
	static int rs_file_close(FILE * f);
};

