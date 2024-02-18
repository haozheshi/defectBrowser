#pragma once
#include "lmdb++.h"
#include <memory>
#include <Windows.h>
#pragma comment(lib, "Advapi32.lib")

class ImageDB
{
public:
    ImageDB() :m_bReadOnly(false){}
	// 将中文路径名称转换成英文路径
	static std::string chineseToHash(const wchar_t* inputPath);
	// 设置最大分卷大小
	int set(int mbsize=512);
	// 打开分卷数据库
	int open(const char * directory, const wchar_t* subdir);
	// 存放图像数据到id
	int putImage(int id, char* val, int length, int& sec);
	// 获取subSec分卷上id的数据
    int getImage(int subSec, int id, std::shared_ptr<char>& result, int& nsize);
    // 设置只读
    void setReadOnly();
private:
	// 存放数据到当前打开的分卷
	int putImageInternal(const char* id, char* val, int length);
	// 获取数据在当前打开的分卷
    int getImageInternal(const char* id, std::shared_ptr<char>& result, int& nsize);
	// 打开subIndex的分卷
	int openSubSection(int subIndex);
private:
	lmdb::env _env{nullptr};
private:
	//当前打开数据库的分卷号
	int _nCurSecIndex;
	//数据库分卷允许最大存储字节
	unsigned long _nMaxSecSize;
	//当前写入卷的已写入字节数
	int _nCurWriteSecSize;
	//当前写入卷的卷号
	int _nCurWirteSecIndex;
	//当前数据库
	std::string _homePath;
    //数据库是否为只读模式
    bool m_bReadOnly;
};
