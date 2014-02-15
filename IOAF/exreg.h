#include<stdio.h>
#include<tsk\tsk_tools_i.h>
#include<tsk\libtsk.h>

#pragma once

int carving_hive(TCHAR * path, TSK_FS_INFO *fs, TSK_INUM_T inode);

uint8_t hive_extract(TSK_FS_INFO * fs, TSK_INUM_T inum,
    TSK_FS_ATTR_TYPE_ENUM type, uint8_t type_used,
    uint16_t id, uint8_t id_used, TSK_FS_FILE_WALK_FLAG_ENUM flags);

static TSK_WALK_RET_ENUM
	hive_action(TSK_FS_FILE * fs_file, TSK_OFF_T a_off, TSK_DADDR_T addr,
    char *buf, size_t size, TSK_FS_BLOCK_FLAG_ENUM flags, void *ptr);
