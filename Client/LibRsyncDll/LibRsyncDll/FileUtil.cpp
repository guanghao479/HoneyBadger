#include "StdAfx.h"
#include "FileUtil.h"


FileUtil::FileUtil(void)
{
}


FileUtil::~FileUtil(void)
{
}


FILE *
FileUtil::rs_file_open(char const *filename, char const *mode)
{
    FILE           *f;
    int		    is_write;

    is_write = mode[0] == 'w';

    if (!filename  ||  !strcmp("-", filename)) {
	if (is_write)
	    return stdout;
	else
	    return stdin;
    }

    if (!(f = fopen(filename, mode))) {
	return NULL;
    }
    
    return f;
}

int FileUtil::rs_file_close(FILE * f)
{
    if ((f == stdin) || (f == stdout)) return 0;
    return fclose(f);
}