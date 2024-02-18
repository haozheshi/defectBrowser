#ifndef FILE_H
#define FILE_H
#ifndef SYSTEM_ABSTRACTIONS_FILE_HPP
#define SYSTEM_ABSTRACTIONS_FILE_HPP

/**
 * @file File.hpp
 *
 * This module declares the SystemAbstractions::File class.
 *
 * © 2013-2018 by Richard Walters
 */

#include <memory>
#include <string>
#include <vector>
#undef CreateDirectory
    /**
     * This class represents a file accessed through the
     * native operating system.
     */
    class File{
    public:
        
		// Lifecycle Management
         static bool IsDirectory(std::wstring path);
         static bool IsExisting(std::wstring file);
         static bool CreateDirectory(std::wstring directory);
         static void ListDirectory(const std::wstring directory, std::vector< std::wstring >& list);
         static bool DeleteDirectory(const std::wstring directory);
         static bool CopyDirectory(
            const std::wstring existingDirectory,
            const std::wstring newDirectory
        );
        static void GetDirectoryRoots(std::vector<std::wstring>& );
        static void GetWorkingDirectory(std::wstring& );
		
		//
		static bool IsDirectory(std::string path);
		static bool IsExisting(std::string file);
		static bool CreateDirectory(std::string directory);
		static void ListDirectory(const std::string directory, std::vector< std::string >& list);
		static bool DeleteDirectory(const std::string directory);
		static bool CopyDirectory(
			const std::string existingDirectory,
			const std::string newDirectory
		);
		static void GetDirectoryRoots(std::vector<std::string>& );
		static void GetWorkingDirectory(std::string& );

    };

#endif /* SYSTEM_ABSTRACTIONS_FILE_HPP */
#endif // FILE_H
