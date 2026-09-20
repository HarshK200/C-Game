#pragma once

#include "src/pch.h"

#include "src/utils/arena_allocator.h"
#include "src/utils/log.h"


namespace GameFileIO
{
    // =================================================================================
    //                                  HELPER FUNCTIONS
    // =================================================================================

    /*
        filepath can be absolute or relative to the build exe.
        Returns the timestamp of when the file at filepath provided was last
        modified in seconds.
    */
    inline long long GetLastModifiedTime(const char* filepath)
    {
        LOG_ASSERT(filepath, "No filepath provided");

        struct stat file_stat = {};
        stat(filepath, &file_stat);
        return file_stat.st_mtime; // last modified timestamp
    }

    /*
        NOTE(harsh): calls fclose(file);
        If you require the FILE* don't use this instead call fopen() yourself
        as this closes the file after checking if it exists.

        returns a true if file exists, false otherwise.
    */
    inline bool FileExists(const char* filepath)
    {
        LOG_ASSERT(filepath, "No filepath provided");

        // open file in read-binary mode i.e. "rb"
        FILE* file = fopen(filepath, "rb");
        if (!file)
            return false;
        fclose(file);

        return true;
    }
    /*
        NOTE(harsh): calls fclose(file);
        If you require the FILE* don't use this instead call fopen() yourself
        as this closes the file after checking if it exists.

        returns a true if file exists, false otherwise.
    */
    inline bool FileExists(const wchar_t* filepath)
    {
        LOG_ASSERT(filepath, "No filepath provided");

        // open file in read-binary mode i.e. "rb"
        FILE* file = _wfopen(filepath, L"rb");
        if (!file)
            return false;
        fclose(file);

        return true;
    }

    /*
        NOTE(harsh): This function does not close the file after reading it size, you *MUST* call
        fclose() explicitly yourself.
        This function also sets the file pointer to SEEK_SET i.e. start of the file before returning.

        returns the filesize in bytes for the FILE* provided.
        Returns -1 on failure
    */
    inline long GetFileSize(FILE* file)
    {
        LOG_ASSERT(file, "Invalid FILE* provided");

        long filesize = -1;
        if (fseek(file, 0, SEEK_END) != 0)
            return -1;

        filesize = ftell(file);
        if (filesize < 0)
            return -1;

        if (fseek(file, 0, SEEK_SET) != 0)
            return -1;

        return filesize;
    }


    // =================================================================================
    //                                  CORE FUNCTIONS
    // =================================================================================

    /*
        Reads a file in read-only binary mode i.e. "rb" and write it into a file_buffer
        and returns it. Also write the filesize to the int* filesize passed in.
        Allocates the buffer using the temp_allocator.

        Returns char* to the buffer on success, nullptr otherwise.
    */
    inline char* ReadFileBinary(
        ArenaAllocator* temp_allocator,
        const char* filepath,
        int* filesize_ptr)
    {
        LOG_ASSERT(temp_allocator, "Invalid temp_allocator pointer provided");
        LOG_ASSERT(filepath, "No filepath provided");
        LOG_ASSERT(filesize_ptr, "Invalid filesize pointer provided");

        // open the file
        FILE* file = fopen(filepath, "rb");
        if (!file)
        {
            return nullptr;
        }

        // get the file size
        *filesize_ptr = GameFileIO::GetFileSize(file);
        if (*filesize_ptr < 0)
        {
            fclose(file);
            return nullptr;
        }

        // allocate the buffer to read the file into of size filsize
        char* file_buffer = ArenaAlloc<char>(temp_allocator, *filesize_ptr);
        if (!file_buffer)
        {
            fclose(file);
            return nullptr;
        }

        // read the file into the file_buffer
        size_t bytes_read = fread(file_buffer, sizeof(char), *filesize_ptr, file);
        if (bytes_read != (size_t)*filesize_ptr)
        {
            fclose(file);
            return nullptr;
        }

        // close the file
        fclose(file);

        return file_buffer;
    }


    /*
        Write a buffer to a file at the provided file_path. The file_path can be relative
        to the executable or absolute. If a file with same file_path exists its content will
        be discarded and overwritten by the new content in file_buffer.

        Returns true if write successfully, false otherwise.
    */
    inline bool WriteFileBinary(
        const char* file_path,
        const char* file_buffer,
        size_t filesize)
    {
        LOG_ASSERT(file_path, "No filepath provided");
        LOG_ASSERT(file_buffer, "No filebuffer provided");
        // LOG_ASSERT(filesize < 1, "Invalid file size");

        // open file in mode "wb" where "w" is for create file and write discard existing
        // file if any, "b" is for read and write in binary format
        FILE* file = fopen(file_path, "wb");
        if (!file)
        {
            LOG_ASSERT(file, "Failed to load file at: %s", file_path);
            return false;
        }

        // write the content. since sizeof(char) is 1 byte, filesize == element count
        size_t written = fwrite(file_buffer, sizeof(char), filesize, file);
        if (written != filesize)
        {
            LOG_ERRORF(
                "GameFileIO::WriteFile didn't complete successfully, %zu elements written out of %zu",
                written,
                filesize);
            fclose(file);
            return false;
        }

        fclose(file);
        return true;
    }

#undef CopyFile
    inline bool CopyFile(ArenaAllocator* temp_allocator, const char* src, const char* dest)
    {
        int filesize = 0;
        const char* data = ReadFileBinary(temp_allocator, src, &filesize);
        if (!data)
        {
            LOG_ASSERT(false, "Failed to read file at %s", src);
            return false;
        }
        if (!dest)
        {
            LOG_ASSERT(false, "invalid dest provided");
            return false;
        }

        if (!WriteFileBinary(dest, data, (size_t)filesize))
        {
            LOG_ASSERT(false, "Falied to write file at %s", dest);
            return false;
        }

        return true;
    }

} // namespace GameFileIO
