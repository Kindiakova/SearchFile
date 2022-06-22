#pragma once

#include <iostream>
#include <fstream>
#include <windows.h>
#include <string.h>
#include <vector>

#include "SearchFile.h";

using namespace std;

string FileName = "";
string FullFilePath = "";
const int MaxThreads = 8; // max thread amount

bool search_in_directory(string path);

bool files_in_directory(string filepath) // search the file in the current directory
{
    WIN32_FIND_DATAW wfd;

    wstring filepathW = wstring(filepath.begin(), filepath.end());
    LPCWSTR filepathL = filepathW.c_str();

    HANDLE const hFind = FindFirstFileW(filepathL, &wfd);
    setlocale(LC_ALL, "");

    if (INVALID_HANDLE_VALUE != hFind)
    {
        do {
            if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

                //convert from wide char to narrow char array
                char ch[260];
                char DefChar = ' ';
                WideCharToMultiByte(CP_ACP, 0, wfd.cFileName, -1, ch, 260, &DefChar, NULL);
                FullFilePath = string(ch);
                return true;
            }
        } while (NULL != FindNextFileW(hFind, &wfd));

        FindClose(hFind);
    }
    return false;
}

bool subdirectories(string path) // search subdirectories of the current directory
{
    WIN32_FIND_DATAW wfd;
    string path_ = path + "*";
    wstring pathW = std::wstring(path_.begin(), path_.end());
    LPCWSTR pathL = pathW.c_str();

    HANDLE const hFind = FindFirstFileW(pathL, &wfd);
    setlocale(LC_ALL, "");

    if (INVALID_HANDLE_VALUE != hFind)
    {
        do {
            if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && wfd.cFileName[0] != '.') {

                //convert from wide char to narrow char array
                char ch[260];
                char DefChar = ' ';
                WideCharToMultiByte(CP_ACP, 0, wfd.cFileName, -1, ch, 260, &DefChar, NULL);

                string DirName(ch);
                if (search_in_directory(path + DirName + "\\")) return true;   // search in the subdirectory

            }

        } while (NULL != FindNextFileW(hFind, &wfd));

        FindClose(hFind);
    }
    return false;
}

bool search_in_directory(string path)
{
    if (files_in_directory(path + FileName + ".*")) return true;
    else return(subdirectories(path));
}
void get_all_root_subdir(string path, vector<string>& res) // get all subdirections of the root direction
{
    WIN32_FIND_DATAW wfd;
    string path_ = path + "*";
    wstring pathW = std::wstring(path_.begin(), path_.end());
    LPCWSTR pathL = pathW.c_str();

    HANDLE const hFind = FindFirstFileW(pathL, &wfd);
    setlocale(LC_ALL, "");

    if (INVALID_HANDLE_VALUE != hFind)
    {
        do {
            if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && wfd.cFileName[0] != '.') {

                //convert from wide char to narrow char array
                char ch[260];
                char DefChar = ' ';
                WideCharToMultiByte(CP_ACP, 0, wfd.cFileName, -1, ch, 260, &DefChar, NULL);
                string DirName(ch);
                res.push_back(path + DirName + "\\");
            }

        } while (NULL != FindNextFileW(hFind, &wfd));

        FindClose(hFind);
    }
    return;
}

string FindTheFile(string name) {
    const string root = "C:\\";
    FileName = name;

    if (files_in_directory(root + FileName + ".*")) return FullFilePath;

    vector<string> subdirs;
    get_all_root_subdir(root, subdirs);

    bool FileWasFound = false;
#pragma omp parallel  for num_threads(MaxThreads)
    for (size_t i = 0; i < subdirs.size(); ++i) {
        auto& elem = subdirs[i];
        if (search_in_directory(elem)) {
            FileWasFound = true;
#pragma omp cancel for 
        }
#pragma omp cancellation point for
    }

    if (!FileWasFound) return "The file with name " + name + " does not exist\n";
    else return FullFilePath;
}
