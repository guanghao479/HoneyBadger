// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 LIBRSYNCDLL_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// LIBRSYNCDLL_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef LIBRSYNCDLL_EXPORTS
#define LIBRSYNCDLL_API __declspec(dllexport)
#else
#define LIBRSYNCDLL_API __declspec(dllimport)
#endif

// 此类是从 LibRsyncDll.dll 导出的
class LIBRSYNCDLL_API CLibRsyncDll {
public:
	CLibRsyncDll(void);
	// TODO: 在此添加您的方法。
};

extern LIBRSYNCDLL_API int nLibRsyncDll;

LIBRSYNCDLL_API int fnLibRsyncDll(void);

LIBRSYNCDLL_API bool rsync_sig(char* oldfile,char* sigfile);

LIBRSYNCDLL_API bool rsync_delta(char* newfile,char* sigfile,char* deltafile);

LIBRSYNCDLL_API bool rsync_patch(char* oldfile,char* newfile,char* deltafile);
