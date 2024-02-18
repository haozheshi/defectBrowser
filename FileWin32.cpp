/**
 * @file FileWin32.cpp
 *
 * This module contains the Win32 specific part of the
 * implementation of the SystemAbstractions::File class.
 *
 * © 2013-2018 by Richard Walters
 */

/**
 * Windows.h should always be included first because other Windows header
 * files, such as KnownFolders.h, don't always define things properly if
 * you don't include Windows.h first.
 */
#include <Windows.h>
#include <io.h>
#include <KnownFolders.h>
#include <memory>
#include <regex>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <stddef.h>
#include <stdio.h>
#include "File.h"

// Ensure we link with Windows shell utility libraries.
#pragma comment(lib, "Shlwapi")
#pragma comment(lib, "Shell32")

namespace {

    std::wstring FixPathDelimiters(const std::wstring& in) {
        std::wstring out;
        for (auto c: in) {
            if (c == L'\\') {
                out.push_back(L'/');
            } else {
                out.push_back(c);
            }
        }
        return out;
    }

	std::string FixPathDelimiters(const std::string& in) {
		std::string out;
		for (auto c : in) {
			if (c == '\\') {
				out.push_back('/');
			}
			else {
				out.push_back(c);
			}
		}
		return out;
	}
}

bool File::IsDirectory(std::wstring path) {
    const DWORD attr = GetFileAttributesW(path.c_str());
    if (
        (attr == INVALID_FILE_ATTRIBUTES)
        || ((attr & FILE_ATTRIBUTE_DIRECTORY) == 0)
    ) {
        return false;
    }
    return true;
}

bool File::IsExisting(std::wstring file) {
    const DWORD attr = GetFileAttributesW(file.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    return true;
}

bool File::CreateDirectory(std::wstring path) {
    const size_t delimiter = path.find_last_of(L"/\\");
    if (delimiter == std::string::npos) {
        return false;
    }
    std::wstring oneLevelUp(path.substr(0, delimiter));
    if (CreateDirectoryW(oneLevelUp.c_str(), NULL) != 0) {
        return true;
    }
    const DWORD error = GetLastError();
    if (error == ERROR_ALREADY_EXISTS) {
        return true;
    }
    if (error != ERROR_PATH_NOT_FOUND) {
        return false;
    }
    if (!CreateDirectory(oneLevelUp)) {
        return false;
    }
    if (CreateDirectoryW(oneLevelUp.c_str(), NULL) == 0) {
        return false;
    }
    return true;
}

    void File::ListDirectory(const std::wstring directory, std::vector< std::wstring >& list) {
        std::wstring directoryWithSeparator(directory);
        if (
            (directoryWithSeparator.length() > 0)
            && (directoryWithSeparator[directoryWithSeparator.length() - 1] != L'\\')
            && (directoryWithSeparator[directoryWithSeparator.length() - 1] != L'/')
        ) {
            directoryWithSeparator += L'\\';
        }
        std::wstring listGlob(directoryWithSeparator);
        listGlob += L"*.*";
        WIN32_FIND_DATAW findFileData;
        const HANDLE searchHandle = FindFirstFileW(listGlob.c_str(), &findFileData);
        list.clear();
        if (searchHandle != INVALID_HANDLE_VALUE) {
            do {
                std::wstring name(findFileData.cFileName);
                if (
                    (name == L".")
                    || (name == L"..")
                ) {
                    continue;
                }
                std::wstring filePath(directoryWithSeparator);
                filePath += name;
                list.push_back(FixPathDelimiters(filePath));
            } while (FindNextFileW(searchHandle, &findFileData) == TRUE);
            FindClose(searchHandle);
        }
    }

    bool File::DeleteDirectory(const std::wstring directory) {
        std::wstring directoryWithSeparator(directory);
        if (
            (directoryWithSeparator.length() > 0)
            && (directoryWithSeparator[directoryWithSeparator.length() - 1] != L'\\')
            && (directoryWithSeparator[directoryWithSeparator.length() - 1] != L'/')
        ) {
            directoryWithSeparator += L'\\';
        }
        std::wstring listGlob(directoryWithSeparator);
        listGlob += L"*.*";
        WIN32_FIND_DATAW findFileData;
        const HANDLE searchHandle = FindFirstFileW(listGlob.c_str(), &findFileData);
        if (searchHandle != INVALID_HANDLE_VALUE) {
            do {
                std::wstring name(findFileData.cFileName);
                if (
                    (name == L".")
                    || (name == L"..")
                ) {
                    continue;
                }
                std::wstring filePath(directoryWithSeparator);
                filePath += name;
                if (PathIsDirectoryW(filePath.c_str())) {
                    if (!DeleteDirectory(filePath.c_str())) {
                        return false;
                    }
                } else {
                    if (DeleteFileW(filePath.c_str()) == 0) {
                        return false;
                    }
                }
            } while (FindNextFileW(searchHandle, &findFileData) == TRUE);
            FindClose(searchHandle);
        }
        return (RemoveDirectoryW(directory.c_str()) != 0);
    }

    bool File::CopyDirectory(
        const std::wstring existingDirectory,
        const std::wstring newDirectory
    ) {
        std::wstring existingDirectoryWithSeparator(existingDirectory);
        if (
            (existingDirectoryWithSeparator.length() > 0)
            && (existingDirectoryWithSeparator[existingDirectoryWithSeparator.length() - 1] != L'\\')
            && (existingDirectoryWithSeparator[existingDirectoryWithSeparator.length() - 1] != L'/')
        ) {
            existingDirectoryWithSeparator += L'\\';
        }
        std::wstring newDirectoryWithSeparator(newDirectory);
        if (
            (newDirectoryWithSeparator.length() > 0)
            && (newDirectoryWithSeparator[newDirectoryWithSeparator.length() - 1] != L'\\')
            && (newDirectoryWithSeparator[newDirectoryWithSeparator.length() - 1] != L'/')
        ) {
            newDirectoryWithSeparator += L'\\';
        }
        if (!File::CreateDirectory(newDirectoryWithSeparator)) {
            return false;
        }
        std::wstring listGlob(existingDirectoryWithSeparator);
        listGlob += L"*.*";
        WIN32_FIND_DATAW findFileData;
        const HANDLE searchHandle = FindFirstFileW(listGlob.c_str(), &findFileData);
        if (searchHandle != INVALID_HANDLE_VALUE) {
            do {
                std::wstring name(findFileData.cFileName);
                if (
                    (name == L".")
                    || (name == L"..")
                ) {
                    continue;
                }
                std::wstring filePath(existingDirectoryWithSeparator);
                filePath += name;
                std::wstring newFilePath(newDirectoryWithSeparator);
                newFilePath += name;
                if (PathIsDirectoryW(filePath.c_str())) {
                    if (!CopyDirectory(filePath, newFilePath)) {
                        return false;
                    }
                } else {
                    if (!CopyFileW(filePath.c_str(), newFilePath.c_str(), FALSE)) {
                        return false;
                    }
                }
            } while (FindNextFileW(searchHandle, &findFileData) == TRUE);
            FindClose(searchHandle);
        }
        return true;
    }

    void File::GetDirectoryRoots(std::vector< std::wstring>& roots) {
        const auto driveStringsBufferSize = (size_t)GetLogicalDriveStringsW(0, nullptr);
        std::shared_ptr< wchar_t > driveStringsBuffer(
            new wchar_t[driveStringsBufferSize],
            [](wchar_t* p){ delete[] p; }
        );
        if (GetLogicalDriveStringsW((DWORD)driveStringsBufferSize, driveStringsBuffer.get()) > 0) {
            std::vector< std::wstring > ;
            size_t i = 0;
            for (;;) {
                size_t j = i;
                while (driveStringsBuffer.get()[j] != 0) {
                    ++j;
                }
                if (i == j) {
                    break;
                }
                size_t k = j;
                while (
                    (k > i)
                    && (
                        (driveStringsBuffer.get()[k - 1] == L'/')
                        || (driveStringsBuffer.get()[k - 1] == L'\\')
                    )
                ) {
                    --k;
                }
                roots.emplace_back(
                    driveStringsBuffer.get() + i,
                    driveStringsBuffer.get() + k
                );
                i = j + 1;
            }
        } 
		else 
		{
            return ;
        }
    }

    void File::GetWorkingDirectory(std::wstring& result) {
        std::vector< wchar_t > workingDirectory(MAX_PATH);
        (void)GetCurrentDirectoryW((DWORD)workingDirectory.size(), &workingDirectory[0]);
		result = FixPathDelimiters(&workingDirectory[0]);
    }

	bool File::IsDirectory(std::string path)
	{
		const DWORD attr = GetFileAttributesA(path.c_str());
		if (
			(attr == INVALID_FILE_ATTRIBUTES)
			|| ((attr & FILE_ATTRIBUTE_DIRECTORY) == 0)
			) {
			return false;
		}
		return true;
	}

	bool File::IsExisting(std::string file)
	{
		const DWORD attr = GetFileAttributesA(file.c_str());
		if (attr == INVALID_FILE_ATTRIBUTES) {
			return false;
		}
		return true;
	}

	bool File::CreateDirectory(std::string path)
	{
		const size_t delimiter = path.find_last_of("/\\");
		if (delimiter == std::string::npos) {
			return false;
		}
		std::string oneLevelUp(path.substr(0, delimiter));
		if (CreateDirectoryA(oneLevelUp.c_str(), NULL) != 0) {
			return true;
		}
		const DWORD error = GetLastError();
		if (error == ERROR_ALREADY_EXISTS) {
			return true;
		}
		if (error != ERROR_PATH_NOT_FOUND) {
			return false;
		}
		if (!CreateDirectory(oneLevelUp)) {
			return false;
		}
		if (CreateDirectoryA(oneLevelUp.c_str(), NULL) == 0) {
			return false;
		}
		return true;
	}

	void File::ListDirectory(const std::string directory, std::vector<std::string>& list)
	{
		std::string directoryWithSeparator(directory);
		if (
			(directoryWithSeparator.length() > 0)
			&& (directoryWithSeparator[directoryWithSeparator.length() - 1] != L'\\')
			&& (directoryWithSeparator[directoryWithSeparator.length() - 1] != L'/')
			) {
			directoryWithSeparator += L'\\';
		}
		std::string listGlob(directoryWithSeparator);
		listGlob += "*.*";
		WIN32_FIND_DATAA findFileData;
		const HANDLE searchHandle = FindFirstFileA(listGlob.c_str(), &findFileData);
		list.clear();
		if (searchHandle != INVALID_HANDLE_VALUE) {
			do {
				std::string name(findFileData.cFileName);
				if (
					(name == ".")
					|| (name == "..")
					) {
					continue;
				}
				std::string filePath(directoryWithSeparator);
				filePath += name;
				list.push_back(FixPathDelimiters(filePath));
			} while (FindNextFileA(searchHandle, &findFileData) == TRUE);
			FindClose(searchHandle);
		}
	}

	bool File::DeleteDirectory(const std::string directory)
	{
		std::string directoryWithSeparator(directory);
		if (
			(directoryWithSeparator.length() > 0)
			&& (directoryWithSeparator[directoryWithSeparator.length() - 1] != L'\\')
			&& (directoryWithSeparator[directoryWithSeparator.length() - 1] != L'/')
			) {
			directoryWithSeparator += L'\\';
		}
		std::string listGlob(directoryWithSeparator);
		listGlob += "*.*";
		WIN32_FIND_DATAA findFileData;
		const HANDLE searchHandle = FindFirstFileA(listGlob.c_str(), &findFileData);
		if (searchHandle != INVALID_HANDLE_VALUE) {
			do {
				std::string name(findFileData.cFileName);
				if (
					(name == ".")
					|| (name == "..")
					) {
					continue;
				}
				std::string filePath(directoryWithSeparator);
				filePath += name;
				if (PathIsDirectoryA(filePath.c_str())) {
					if (!DeleteDirectory(filePath.c_str())) {
						return false;
					}
				}
				else {
					if (DeleteFileA(filePath.c_str()) == 0) {
						return false;
					}
				}
			} while (FindNextFileA(searchHandle, &findFileData) == TRUE);
			FindClose(searchHandle);
		}
		return (RemoveDirectoryA(directory.c_str()) != 0);
	}

	bool File::CopyDirectory(const std::string existingDirectory, const std::string newDirectory)
	{
		std::string existingDirectoryWithSeparator(existingDirectory);
		if (
			(existingDirectoryWithSeparator.length() > 0)
			&& (existingDirectoryWithSeparator[existingDirectoryWithSeparator.length() - 1] != L'\\')
			&& (existingDirectoryWithSeparator[existingDirectoryWithSeparator.length() - 1] != L'/')
			) {
			existingDirectoryWithSeparator += L'\\';
		}
		std::string newDirectoryWithSeparator(newDirectory);
		if (
			(newDirectoryWithSeparator.length() > 0)
			&& (newDirectoryWithSeparator[newDirectoryWithSeparator.length() - 1] != L'\\')
			&& (newDirectoryWithSeparator[newDirectoryWithSeparator.length() - 1] != L'/')
			) {
			newDirectoryWithSeparator += L'\\';
		}
		if (!File::CreateDirectory(newDirectoryWithSeparator)) {
			return false;
		}
		std::string listGlob(existingDirectoryWithSeparator);
		listGlob += "*.*";
		WIN32_FIND_DATAA findFileData;
		const HANDLE searchHandle = FindFirstFileA(listGlob.c_str(), &findFileData);
		if (searchHandle != INVALID_HANDLE_VALUE) {
			do {
				std::string name(findFileData.cFileName);
				if (
					(name == ".")
					|| (name == "..")
					) {
					continue;
				}
				std::string filePath(existingDirectoryWithSeparator);
				filePath += name;
				std::string newFilePath(newDirectoryWithSeparator);
				newFilePath += name;
				if (PathIsDirectoryA(filePath.c_str())) {
					if (!CopyDirectory(filePath, newFilePath)) {
						return false;
					}
				}
				else {
					if (!CopyFileA(filePath.c_str(), newFilePath.c_str(), FALSE)) {
						return false;
					}
				}
			} while (FindNextFileA(searchHandle, &findFileData) == TRUE);
			FindClose(searchHandle);
		}
		return true;
	}

	void File::GetDirectoryRoots(std::vector< std::string>& roots) {
		const auto driveStringsBufferSize = (size_t)GetLogicalDriveStringsA(0, nullptr);
		std::shared_ptr< char > driveStringsBuffer(
			new char[driveStringsBufferSize],
			[](char* p) { delete[] p; }
		);
		if (GetLogicalDriveStringsA((DWORD)driveStringsBufferSize, driveStringsBuffer.get()) > 0) {
			std::vector< std::string >;
			size_t i = 0;
			for (;;) {
				size_t j = i;
				while (driveStringsBuffer.get()[j] != 0) {
					++j;
				}
				if (i == j) {
					break;
				}
				size_t k = j;
				while (
					(k > i)
					&& (
					(driveStringsBuffer.get()[k - 1] == L'/')
						|| (driveStringsBuffer.get()[k - 1] == L'\\')
						)
					) {
					--k;
				}
				roots.emplace_back(
					driveStringsBuffer.get() + i,
					driveStringsBuffer.get() + k
				);
				i = j + 1;
			}
		}
		else
		{
			return;
		}
	}

	void File::GetWorkingDirectory(std::string & result)
	{
		std::vector< char > workingDirectory(MAX_PATH);
		(void)GetCurrentDirectoryA((DWORD)workingDirectory.size(), &workingDirectory[0]);
		result = FixPathDelimiters(&workingDirectory[0]);
	}