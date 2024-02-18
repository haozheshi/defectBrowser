#include "imdb.h"
#include "File.h"
#include <algorithm>


std::string ImageDB::chineseToHash(const wchar_t * inputPath)
{
	int len = wcslen(inputPath);
	char buff[MAX_PATH] = { 0 };
	for (int i = 0; i < len; i++)
	{
		unsigned char hi = inputPath[i] & 0xff00;
		unsigned char lo = inputPath[i] & 0x00ff;
		char tmp[4] = { 0 };
		sprintf_s(tmp, 4, "%x", lo);
		buff[2 * i ] = tmp[0];
		buff[2 * i + 1] = tmp[1];
	}
	return std::string(buff);
}

int ImageDB::set(int mbsize)
{
	_nMaxSecSize = 1024UL * 1024UL * mbsize;
	return 0;
}

void ImageDB::setReadOnly()
{
    m_bReadOnly = true;
}

int ImageDB::open(const char * directory, const wchar_t* subdir)
{
	std::string hashName = chineseToHash(subdir);
	std::string dirPath = std::string(directory) + std::string("/") + hashName;
	_homePath = dirPath;
	_nCurWirteSecIndex = 1;
	if (File::IsDirectory(_homePath))
	{
		std::vector<std::string> result;
		std::vector<int> subSec;
		File::ListDirectory(_homePath, result);
		for (int i = 0; i < result.size(); i++)
		{
			int npos = result[i].rfind("/");
			if (npos >= 0)
			{
				std::string strIndex = result[i].substr(npos + 1);
				subSec.push_back(atoi(strIndex.c_str()));
			}			
		}
		if (!subSec.empty())
		{
			std::sort(subSec.begin(), subSec.end());
			_nCurWirteSecIndex = subSec.back();
		}
	}
	int rc = openSubSection(_nCurWirteSecIndex);
	if (rc == 0)
	{	
		try
		{
			auto _txn = lmdb::txn::begin(_env);
			auto dbi = lmdb::dbi::open(_txn, 0);
			auto cursor = lmdb::cursor::open(_txn, dbi);
			MDB_val k;
			MDB_val v;
			_nCurWriteSecSize = 0;
			while (lmdb::cursor_get(cursor.handle(), &k, &v, MDB_NEXT))
			{
				_nCurWriteSecSize += v.mv_size;
				_nCurWriteSecSize += k.mv_size;
				//std::vector<uchar> bytes;
				//char* img = (char*)v.mv_data;
				//for (int j = 0; j < v.mv_size; j++)
				//{
				//	bytes.push_back(img[j]);
				//}
				//cv::Mat im = cv::imdecode(bytes, 0);
			}
			cursor.close();
			_txn.abort();
		}
		catch (const lmdb::error&)
		{
			return 1;
		}
	}
	return rc;
}

int ImageDB::putImage(int id, char * val, int length, int& sec)
{
	int rc = 0;
	if (_nCurWriteSecSize + length > _nMaxSecSize * 0.8)
	{
		rc = openSubSection(_nCurWirteSecIndex + 1);
		if (rc == 0)
		{
			_nCurWirteSecIndex = _nCurWirteSecIndex + 1;
			_nCurWriteSecSize = 0;
		}
		else
		{
			return rc;
		}
	}
	if (_nCurSecIndex != _nCurWirteSecIndex)
	{
		rc = openSubSection(_nCurWirteSecIndex);
	}
	if (rc == 0)
	{
		std::string idstr = std::to_string(id);
		rc = putImageInternal(idstr.c_str(), val, length);
		sec = _nCurWirteSecIndex;
	}
	return rc;
}

int ImageDB::getImage(int subSec, int id, std::shared_ptr<char>& result, int& nsize)
{	
	int rc = 0;
	std::string subPath = std::string(_homePath) + "/" + std::to_string(subSec);
	if (File::IsDirectory(subPath))
	{
		if (subSec != _nCurSecIndex)
		{
			rc = openSubSection(subSec);
			if (rc == 0)
			{
				std::string idstr = std::to_string(id);
                rc = getImageInternal(idstr.c_str(), result, nsize);
			}		
		}
		else
		{
			std::string idstr = std::to_string(id);
            rc = getImageInternal(idstr.c_str(), result, nsize);
		}
	}
	else
	{
		rc = 1;
	}
	return rc;
}

int ImageDB::putImageInternal(const char * id, char * val, int length)
{
	try
	{
		auto _txn = lmdb::txn::begin(_env);
		auto dbi = lmdb::dbi::open(_txn, 0);
		MDB_val k;

		k.mv_size = strlen(id);
		k.mv_data = (void*)id;
		MDB_val v;
		v.mv_size = length;
		v.mv_data = val;
		if(lmdb::dbi_put(_txn, dbi.handle(), &k, &v, MDB_NOOVERWRITE))
		{
			_nCurWriteSecSize += length;
			_nCurWriteSecSize += strlen(id);
			_txn.commit();
			return 0;
		}
		else
		{
			_txn.commit();
			return 1;
		}	
	}
	catch (const lmdb::error&)
	{
		return 1;
	}
}

int ImageDB::getImageInternal(const char * id, std::shared_ptr<char>& result, int& nsize)
{
    nsize = 0;
	try
	{
        lmdb::txn _txn{nullptr};
        if(m_bReadOnly)
        {
          _txn = lmdb::txn::begin(_env,nullptr, MDB_RDONLY);
        }
        else
        {
          _txn = lmdb::txn::begin(_env);
        }
		auto dbi = lmdb::dbi::open(_txn, 0);
		auto cursor = lmdb::cursor::open(_txn, dbi);
		MDB_val k;
		k.mv_size = strlen(id);
		k.mv_data = (void*)id;
		MDB_val v;
		if(lmdb::cursor_get(cursor.handle(), &k, &v, MDB_SET))
		{
			result = std::shared_ptr<char>(new char[v.mv_size]);
			memcpy(result.get(), v.mv_data, v.mv_size);
            nsize = v.mv_size;
			cursor.close();
			_txn.abort();
			return 0;
		}
		else
		{
			cursor.close();
			_txn.abort();
			return 1;
		}
	}
	catch (const lmdb::error&)
	{
		return 1;
	}
}

int ImageDB::openSubSection(int subIndex)
{
	std::string subPath = std::string(_homePath) + "/" + std::to_string(subIndex);
	if (!File::IsDirectory(subPath))
	{
		File::CreateDirectory(subPath + "/");
	}
	try
	{
		_env = lmdb::env::create();
		_env.set_mapsize(_nMaxSecSize);
		_env.open(subPath.c_str(), 0, 0664);
		_nCurSecIndex = subIndex;
		return 0;
	}
	catch (const lmdb::error&)
	{
		return 1;
	}
}
