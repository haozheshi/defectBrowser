#pragma once
#include "lmdb++.h"
#include <memory>
#include <Windows.h>
#pragma comment(lib, "Advapi32.lib")

class ImageDB
{
public:
    ImageDB() :m_bReadOnly(false){}
	// ������·������ת����Ӣ��·��
	static std::string chineseToHash(const wchar_t* inputPath);
	// �������־��С
	int set(int mbsize=512);
	// �򿪷־����ݿ�
	int open(const char * directory, const wchar_t* subdir);
	// ���ͼ�����ݵ�id
	int putImage(int id, char* val, int length, int& sec);
	// ��ȡsubSec�־���id������
    int getImage(int subSec, int id, std::shared_ptr<char>& result, int& nsize);
    // ����ֻ��
    void setReadOnly();
private:
	// ������ݵ���ǰ�򿪵ķ־�
	int putImageInternal(const char* id, char* val, int length);
	// ��ȡ�����ڵ�ǰ�򿪵ķ־�
    int getImageInternal(const char* id, std::shared_ptr<char>& result, int& nsize);
	// ��subIndex�ķ־�
	int openSubSection(int subIndex);
private:
	lmdb::env _env{nullptr};
private:
	//��ǰ�����ݿ�ķ־��
	int _nCurSecIndex;
	//���ݿ�־��������洢�ֽ�
	unsigned long _nMaxSecSize;
	//��ǰд������д���ֽ���
	int _nCurWriteSecSize;
	//��ǰд���ľ��
	int _nCurWirteSecIndex;
	//��ǰ���ݿ�
	std::string _homePath;
    //���ݿ��Ƿ�Ϊֻ��ģʽ
    bool m_bReadOnly;
};
