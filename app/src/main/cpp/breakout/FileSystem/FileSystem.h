#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include <stdio.h>
#include <android/asset_manager.h>


FILE* android_fopen(const char* fname, const char* mode);
void android_fopen_set_asset_manager(AAssetManager* manager);


#define fopen(fname, mode) android_fopen(fname, mode);

#endif //_FILESYSTEM_H
