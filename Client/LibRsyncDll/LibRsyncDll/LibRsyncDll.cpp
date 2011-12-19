// LibRsyncDll.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "LibRsyncDll.h"
#include "librsync.h"
#include "FileUtil.h"
#include <stdio.h>


// 这是导出变量的一个示例
LIBRSYNCDLL_API int nLibRsyncDll=0;

// 这是导出函数的一个示例。
LIBRSYNCDLL_API int fnLibRsyncDll(void)
{
	return 42;
}

// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 LibRsyncDll.h
CLibRsyncDll::CLibRsyncDll()
{
	return;
}

size_t blk_len=RS_DEFAULT_BLOCK_LEN;
size_t stg_len=RS_DEFAULT_STRONG_LEN;

bool rsync_sig(char* oldfile,char* sigfile){
	FILE* old=FileUtil::rs_file_open(oldfile,"rb");
	FILE* sig=FileUtil::rs_file_open(sigfile,"wb");
	rs_result rs=rs_sig_file(old,sig,blk_len,stg_len,NULL);
	FileUtil::rs_file_close(old);
	FileUtil::rs_file_close(sig);
	if(rs==RS_DONE){
		return true;}else{
		return false;}
}

bool rsync_delta(char* newfile,char* sigfile,char* deltafile){
	FILE* sig=FileUtil::rs_file_open(sigfile,"rb");
	
	rs_signature_t* sumset;
	rs_result rs=rs_loadsig_file(sig,&sumset,NULL);
	if(rs!=RS_DONE){
		FileUtil::rs_file_close(sig);
	return false;}
	rs=rs_build_hash_table(sumset);
	if(rs!=RS_DONE){
		FileUtil::rs_file_close(sig);
	return false;}
	FILE* newf=FileUtil::rs_file_open(newfile,"rb");
	FILE* delta=FileUtil::rs_file_open(deltafile,"wb");
	rs=rs_delta_file(sumset,newf,delta,NULL);
	FileUtil::rs_file_close(sig);
	FileUtil::rs_file_close(newf);
	FileUtil::rs_file_close(delta);
	if(rs!=RS_DONE){
	return false;}
	return true;
}

bool rsync_patch(char* oldfile,char* newfile,char* deltafile){
	FILE* old=FileUtil::rs_file_open(oldfile,"rb");
	FILE* delta=FileUtil::rs_file_open(deltafile,"rb");
	FILE* newf=FileUtil::rs_file_open(newfile,"wb");
	rs_result rs=rs_patch_file(old,delta,newf,NULL);
	FileUtil::rs_file_close(old);
	FileUtil::rs_file_close(newf);
	FileUtil::rs_file_close(delta);
	if(rs!=RS_DONE){
		return false;}else{
		return true;}
}
