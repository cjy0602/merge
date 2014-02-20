#include <tchar.h>
#pragma once

int mft_image(_TCHAR *image);
int mft_live(char *path, char *casename);
int mft_image2db(char* case_path, char* case_name);
int iehist(int argc, char** argv);