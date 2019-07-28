#ifndef _FILECHECKSUM_H
#define _FILECHECKSUM_H

#include "stdtypes.h"

typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;

typedef enum
{
	CHECKSUM_MATCHES_MANIFEST = 1,
	CHECKSUM_MATCHES_PATCH,
} CheckStatus;

typedef struct
{
	S64			size;
	U32			values[4];
} Checksum;

typedef struct
{
	char		*name;
	char		*pig_name;
	Checksum	full;
	CheckStatus	full_status;
	Checksum	*partials;
	int			partial_count,partial_max;
	U32			del : 1;
	U32			added : 1;
	U32			mod : 1;
	U32			in_patch : 1;
	U32			sort_by_filename : 1;
	U32			is_compressed : 1;

	U32			data_size;
	U32			data_pack_size;
	U32			data_pos;
	U32			timestamp;
} FileCheck;

typedef struct
{
	char		*fname;
	char		*build_name;
	FileCheck	**files;
	int			file_count,file_max;
	StashTable	filenames;
	char		*manifest_string;
	U32			timestamp;
	S64			bytecount;
	U32			pigs_opened : 1;
	U32			exists_on_server : 1;
} ImageCheck;

typedef enum
{
	CHECKSUM_IF_QUICK_FAILS,
	CHECKSUM_ALWAYS,
	CHECKSUM_NEVER
} ImageCheckType;

int checksumMatch(const Checksum *a,const Checksum *b);
ImageCheck *checksumDuplicate(ImageCheck *image,int include_pigged);
int checksumLoad(char *checksum_name,ImageCheck *image);
int checksumLoadFromMem(char *mem,ImageCheck *image,int outputErrors);
void checksumWrite(ImageCheck *image,char *checksum_name);
FileCheck *checksumAddFile(ImageCheck *image,const char *fname,char *pig_name,int outputErrors);
ImageCheck *matchImage(ImageCheck **images,int image_count,ImageCheck *client,char *err_msg);
void checksumMem(void *mem,U32 len,U32 result[4]);
void checksumFile(char *fname,Checksum *check,int update_stats);
void checksumMultipleFiles(char **fnames, unsigned int nFiles, Checksum *check, int update_stats);
int checksumVerify(char *image_dir,ImageCheck *image,ImageCheck *patch,ImageCheckType full_check,int check_pigs);
int checksumOpenPigs(ImageCheck *image,char *image_dir,int validate_piggs);
void checksumFree(ImageCheck *image);
void checksumImage(const char *image_dir,const char *checksum_name,const char *image_name);
int safeOverwriteFileAndChecksum(char *fname,char *how,void *mem,U32 len,Checksum *dst_check,int fatal);
char *checksumMakeString(ImageCheck *image,char *build_name,int full_manifest);
void checksumMakeEmpty(char *fname,char *build_name);
int getPatchFileChecksum(char *fname, Checksum *check);
void checksumUpdateFile(ImageCheck *image,FileCheck *file);

#endif
