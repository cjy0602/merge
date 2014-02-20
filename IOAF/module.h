#include"define.h"
#include "sql.h"
#include "exreg.h"

#pragma once
void mft_live_module(TCHAR *volName, TCHAR * CaseName);
void mft_image_module(TCHAR * ImagePath, TCHAR * CaseName);

void reg_live_module(TCHAR *volName,  TCHAR* CaseName);
void reg_image_module(TCHAR *ImagePath, int offset,  TCHAR* CaseName);

void ie_hist_module(int argc, char** argv);

void deepscan(TCHAR * CaseName);
void scan(TCHAR * CaseName);