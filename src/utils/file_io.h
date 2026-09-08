#pragma once

#include <cstdint>
#include <cstdio>
#include <sys/stat.h>

#include "src/utils/arena_allocator.h"
#include "src/utils/log.h"


namespace GameFileIO
{
    // ================================ HELPER FUNCTIONS ================================

    /*
        returns the timestamp of when the file at filepath provided was last
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
        NOTE(harsh): if you require the FILE* don't use this instead call fopen() yourself
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


    // ================================ CORE FUNCTIONS ================================

    /*
        Reads a file in read-only binary mode i.e. "rb" and write it into a file_buffer
        and returns it. Also write the filesize to the int* filesize passed in.
        Allocates the buffer using the temp_allocator.
        Returns char* to the buffer on success, nullptr otherwise.
    */
    inline uint8_t* ReadFile(
        ArenaAllocator* temp_allocator,
        const char* filepath,
        int* filesize)
    {
        LOG_ASSERT(temp_allocator, "Invalid temp_allocator pointer provided");
        LOG_ASSERT(filepath, "No filepath provided");
        LOG_ASSERT(filesize, "Invalid filesize pointer provided");

        // open the file
        FILE* file = fopen(filepath, "rb");
        if (!file)
        {
            return nullptr;
        }

        // get the file size
        *filesize = GameFileIO::GetFileSize(file);
        if (*filesize < 0)
        {
            fclose(file);
            return nullptr;
        }

        // allocate the buffer to read the file into of size filsize
        uint8_t* file_buffer = ArenaAlloc(temp_allocator, *filesize);
        if (!file_buffer)
        {
            fclose(file);
            return nullptr;
        }

        // read the file into the file_buffer
        size_t bytes_read = fread(file_buffer, sizeof(char), *filesize, file);
        if (bytes_read != (size_t)*filesize)
        {
            fclose(file);
            return nullptr;
        }

        // close the file
        fclose(file);

        return file_buffer;
    }


    // TODO(harsh): implement this in the future when required
    inline void WriteFile()
    {
    }

} // namespace GameFileIO
