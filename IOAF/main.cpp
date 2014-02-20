#include <stdio.h>
#include <Windows.h>
#include <tchar.h>
#include <tsk\libtsk.h>
#include <tsk\tsk_tools_i.h>
#include <direct.h>  
#include "sql.h"
#include "module.h"

static TSK_TCHAR *progname;

static uint8_t print_bytes = 0;
static uint8_t recurse = 0;

static int recurse_cnt = 0;
static TSK_DADDR_T recurse_list[64];
static void print_header(const TSK_VS_INFO * vs);
static TSK_WALK_RET_ENUM part_act(TSK_VS_INFO * vs, const TSK_VS_PART_INFO * part, void *ptr);
int info_offset(TCHAR *drive);
void usage();
void drive_info(TCHAR *drive);

void _tmain(int argc, TSK_TCHAR **argv)
{
	int opt=0;
	int deepchk=0;
	int live=1;
	TSK_TCHAR *ImagePath = NULL;
	TSK_TCHAR *CaseName = NULL;
	TSK_TCHAR *DriveName = NULL;
	TSK_TCHAR *VolumeName = NULL;

	int Timeline_num = 50;
	int offset = -1;

	/*    # 7가지 프로그램 실행 옵션 
	-n : case명 입력 (입력받은 이름으로 폴더 생성)
	-v : 분석할 논리 드라이브명 입력 ( @ 라이브시스템 분석 모드 )
	-i : 현재 시스템의 이미지 디스크정보 출력 (독립사용)
	-I : 이미지 분석모드로 디스크 이미지 경로 입력 (대문자 I)
	-d : Deep Scan 모드 (추후 구현예정, 패턴비교 방식의 차이)
	-h : 프로그램 Usage 출력
	-o : offset 설정
	-t : TIMELINE에서 뿌려줄 라인 수 지정 ( 미지정시 기본 값 50 )

	ex)
	라이브 분석)
	$ioaf.exe -n "Case123" -v "C"

	이미지 분석)
	$ioaf.exe -i (디스크 정보 출력)
	$ioaf.exe -n "case123" -I "c:\sample.dd" -o 2048
	*/

	//while(-1 != (opt = GETOPT(argc, argv, _TSK_T("h:v:lo:d:n:"))))
	while ((opt = GETOPT(argc, argv, _TSK_T("n:v:i:I:d:h:t:"))) != -1) 
	{
		switch(opt) 
		{ 
		case _TSK_T('h') :
			usage(); 
			break; 

		case _TSK_T('i') :
			DriveName = OPTARG;
			//wcsncpy(DriveName, OPTARG, sizeof(OPTARG));
			drive_info(DriveName);
			break;

		case _TSK_T('d') :
			deepchk=1;
			break;

		case _TSK_T('n') :
			CaseName = OPTARG;
			//wcsncpy(CaseName, OPTARG, sizeof(OPTARG));
			break;

		case _TSK_T('o') : 
			offset = _ttoi(OPTARG);
			break;

		case _TSK_T('I') : 
			live=0;  // 라이브모드 사용 안함 설정
			ImagePath = OPTARG;
			//wcsncpy(ImagePath, OPTARG, sizeof(OPTARG));
			break;

		case _TSK_T('v') : 
			offset = _ttoi(OPTARG);
			VolumeName = OPTARG;
			//wcsncpy(VolumeName, OPTARG, sizeof(OPTARG));
			break;

		case _TSK_T('t') : 
			Timeline_num = _ttoi(OPTARG);
			break;

		default : 
			printf("Invalid Argument(Default).\n");
			usage();
		}
	}

	if (DriveName==NULL && CaseName == NULL){
		printf("Invalid CaseName\n");
		usage();

	}
	else
	{
		char* casename_char;
		int len; 
		len = WideCharToMultiByte(CP_ACP, 0, CaseName, -1, NULL, 0, NULL,NULL);
		casename_char = new char[len];
		WideCharToMultiByte(CP_ACP, 0, CaseName, -1, casename_char, len, NULL, NULL);

		mkdir(casename_char);

	}

	if ( live == 1 && VolumeName != NULL && CaseName != NULL)
	{
		TCHAR tmp[0x100];
		TSK_IMG_INFO *img;
		TSK_FS_INFO *fs;

		memset(tmp, 0, 0x100);
		printf("Extracting MFT ...\n");

		mft_live_module(VolumeName, CaseName);
		memset(tmp, 0, 0x100);
		wsprintf(tmp, L"\\\\.\\%s", VolumeName);

		if(!open_fs_live(tmp, &img, &fs)){
			printf("Extracting Registry ...\n");
			reg_live_module(fs);

			printf("Extracting ie history ...\n");
			ie_file(fs);

			img_fs_close(&img, &fs);
		}

	}

	else if( live == 0 && ImagePath != NULL && CaseName != NULL)
		//else if( live == 0 && ImagePath != NULL && offset != -1 && CaseName != NULL)
	{
		TCHAR tmp[0x100];
		TSK_IMG_INFO * img;
		TSK_FS_INFO * fs;

		if(!open_fs_image(ImagePath, offset, &img, &fs)){
			printf("Extracting MFT ...\n");
			mft_image_module(ImagePath, CaseName);
			printf("Extracting Registry ...\n");
			reg_image_module(fs);
			printf("Extracting ie history ...\n");
			ie_file(fs);
			img_fs_close(&img, &fs);
		}
	}

	else
	{
		printf("Invaild Operation Mode.\n");
		usage();
	}

	
	if ( deepchk == 1)
	deepscan(CaseName);
	else
	scan(CaseName);
	

	printf("@@@@@@@@@@@@@@@@@@@@@@@@\n");
	WCHAR path[100] = {0,};
	wsprintf(path, L"..\\%s\\info.db", CaseName);
	if(!MoveFile(L".\\info.db", path))
		printf("MoveFile Failed!!!\n");

	/*
	WCHAR path2[100] = {0,};
	wsprintf(path2, L"..\/%s\/match.db", CaseName);
	MoveFile(L"match.db", path2);
	*/
}

void usage()
{
	printf("-h : help\n-n : case name\n-o : offset\n-i <disk>: disk info\n-I <image path>: image mode \n-v : volume \n\n");
	exit(0);
}


static TSK_WALK_RET_ENUM part_act(TSK_VS_INFO * vs, const TSK_VS_PART_INFO * part, void *ptr)
{
	if (part->flags & TSK_VS_PART_FLAG_META)
		tsk_printf("%.2" PRIuPNUM ":  Meta    ", part->addr);

	/* Neither table or slot were given */
	else if ((part->table_num == -1) && (part->slot_num == -1))
		tsk_printf("%.2" PRIuPNUM ":  -----   ", part->addr);

	/* Table was not given, but slot was */
	else if ((part->table_num == -1) && (part->slot_num != -1))
		tsk_printf("%.2" PRIuPNUM ":  %.2" PRIu8 "      ",
		part->addr, part->slot_num);

	/* The Table was given, but slot wasn't */
	else if ((part->table_num != -1) && (part->slot_num == -1))
		tsk_printf("%.2" PRIuPNUM ":  -----   ", part->addr);

	/* Both table and slot were given */
	else if ((part->table_num != -1) && (part->slot_num != -1))
		tsk_printf("%.2" PRIuPNUM ":  %.2d:%.2d   ",
		part->addr, part->table_num, part->slot_num);

	if (print_bytes) {
		TSK_OFF_T size;
		char unit = 'B';
		size = part->len * part->vs->block_size;

		if (size > 1024) {
			size /= 1024;
			unit = 'K';
		}

		if (size > 1024) {
			size /= 1024;
			unit = 'M';
		}

		if (size > 1024) {
			size /= 1024;
			unit = 'G';
		}

		if (size > 1024) {
			size /= 1024;
			unit = 'T';
		}


		/* Print the layout */
		tsk_printf("%.10" PRIuDADDR "   %.10" PRIuDADDR "   %.10" PRIuDADDR
			"   %.4" PRIuOFF "%c   %s\n", part->start,
			(TSK_DADDR_T) (part->start + part->len - 1), part->len, size,
			unit, part->desc);
	}
	else {
		/* Print the layout */
		tsk_printf("%.10" PRIuDADDR "   %.10" PRIuDADDR "   %.10" PRIuDADDR
			"   %s\n", part->start,
			(TSK_DADDR_T) (part->start + part->len - 1), part->len,
			part->desc);
	}

	if ((recurse) && (vs->vstype == TSK_VS_TYPE_DOS)
		&& (part->flags == TSK_VS_PART_FLAG_ALLOC)) {
			if (recurse_cnt < 64)
				recurse_list[recurse_cnt++] = part->start * part->vs->block_size;
	}

	return TSK_WALK_CONT;
}

static void print_header(const TSK_VS_INFO * vs)
{
	tsk_printf("%s\n", tsk_vs_type_todesc(vs->vstype));
	tsk_printf("Offset Sector: %" PRIuDADDR "\n",
		(TSK_DADDR_T) (vs->offset / vs->block_size));
	tsk_printf("Units are in %d-byte sectors\n\n", vs->block_size);
	if (print_bytes)
		tsk_printf
		("     Slot    Start        End          Length       Size    Description\n");
	else
		tsk_printf
		("     Slot    Start        End          Length       Description\n");
}


int info_offset(TCHAR *drive)
{
	TSK_VS_INFO *vs;
	int ch;
	TSK_OFF_T imgaddr = 0;
	int flags = 0;
	TSK_IMG_INFO *img;
	TSK_IMG_TYPE_ENUM imgtype = TSK_IMG_TYPE_DETECT;
	TSK_VS_TYPE_ENUM vstype = TSK_VS_TYPE_DETECT;
	uint8_t hide_meta = 0;
	TSK_TCHAR **argv;
	unsigned int ssize = 0;
	TSK_TCHAR *cp;



	progname = drive;

	/* open the image */
	img = tsk_img_open(1, &progname ,imgtype, ssize);

	if (img == NULL) {
		tsk_error_print(stderr);
		exit(1);
	}
	if ((imgaddr * img->sector_size) >= img->size) {
		tsk_fprintf(stderr,
			"Sector offset supplied is larger than disk image (maximum: %"
			PRIu64 ")\n", img->size / img->sector_size);
		exit(1);
	}

	/* process the partition tables */
	vs = tsk_vs_open(img, imgaddr * img->sector_size, vstype);
	if (vs == NULL) {
		tsk_error_print(stderr);
		if (tsk_error_get_errno() == TSK_ERR_VS_UNSUPTYPE)
			tsk_vs_type_print(stderr);
		tsk_img_close(img);
		exit(1);
	}

	print_header(vs);

	if (tsk_vs_part_walk(vs, 0, vs->part_count - 1,
		(TSK_VS_PART_FLAG_ENUM) flags, part_act, NULL)) {
			tsk_error_print(stderr);
			tsk_vs_close(vs);
			tsk_img_close(img);
			exit(1);
	}

	if ((recurse) && (vs->vstype == TSK_VS_TYPE_DOS)) {
		int i;
		/* disable recursing incase we hit another DOS partition
		* future versions may support more layers */
		recurse = 0;

		for (i = 0; i < recurse_cnt; i++) {
			TSK_VS_INFO *vs2;
			vs2 = tsk_vs_open(img, recurse_list[i], TSK_VS_TYPE_DETECT);
			if (vs2 != NULL) {
				tsk_printf("\n\n");
				print_header(vs2);
				if (tsk_vs_part_walk(vs2, 0, vs2->part_count - 1,
					(TSK_VS_PART_FLAG_ENUM) flags, part_act, NULL)) {
						tsk_error_reset();
				}
				tsk_vs_close(vs2);
			}
			else {
				/* Ignore error in this case and reset */
				tsk_error_reset();
			}
		}
	}

	tsk_vs_close(vs);
	tsk_img_close(img);
	exit(0);
}

void drive_info(TCHAR * drive)
{
	printf("# The Physical Drives of this Machine : \n");
	//getPhysicalDrive();
	system("wmic diskdrive get Caption, Name");
	printf("\n");

	printf("# The Logical Drives of this Machine : \n");
	system("wmic logicaldisk get Description, Name, FileSystem, SystemName");
	if (drive != NULL)
		info_offset(drive);
	exit(1);
}