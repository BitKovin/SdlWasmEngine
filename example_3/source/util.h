/* date = February 17th 2024 1:00 am */

#ifndef UTIL_H
#define UTIL_H




u32 get_file_size(char* filename);
void* read_file(char *filename);
void read_from_file_offset(char* file, u64 offset, u64 size, void* dest);
b32 write_file(char *filename, u64 memory_size, void* memory);

#ifdef _WIN32

u32 get_file_size(char* filename)
{
    FILE* fp;
    errno_t err = fopen_s(&fp, filename, "rb"); 
    if (err != 0 || fp == NULL) { 
        printf("File Not Found!\n"); 
        return 1; 
    } 
    fseek(fp, 0L, SEEK_END); 
    long int res = ftell(fp); 
    fclose(fp); 
    return (u32)res;
}

void* read_file(char *filename)
{
    u32 mem_size = get_file_size(filename);
    void* mem = malloc(mem_size);
    
    
    
    FILE* fp;
    errno_t err = fopen_s(&fp, filename, "rb"); 
    if (err != 0 || fp == NULL) { 
        printf("File Not Found!\n"); 
        return NULL; 
    } 
    fread(mem, mem_size, 1, fp);
    fclose(fp);
    
    return mem;
}

void read_from_file_offset(char* filename, u64 offset, u64 size, void* dest)
{
    
    
    FILE* fp;
    errno_t err = fopen_s(&fp, filename, "rb"); 
    if (err != 0 || fp == NULL) { 
        printf("File Not Found!\n"); 
        return; 
    } 
    
    fseek(fp, (i64)offset,SEEK_SET);
    fread(dest, size, 1, fp);
    fclose(fp);
}

b32 write_file(char *filename, u64 memory_size, void* memory)
{
    
    
    FILE* fp;
    errno_t err = fopen_s(&fp, filename, "rb"); 
    if (err != 0 || fp == NULL) { 
        printf("Unable to open file!\n"); 
        return false; 
    } 
    fwrite(memory,1,memory_size, fp);
    fclose(fp);
    return true;
}



#else



u32 get_file_size(char* filename)
{
    FILE* fp = fopen(filename, "rb"); 
    if (fp == NULL) { 
        printf("File Not Found!\n"); 
        return 1; 
    } 
    fseek(fp, 0L, SEEK_END); 
    long int res = ftell(fp); 
    fclose(fp); 
    return (u32)res;
}

void* read_file(char *filename)
{
    u32 mem_size = get_file_size(filename);
    void* mem = malloc(mem_size);
    
    
    FILE* fp = fopen(filename, "rb");
    fread(mem, mem_size, 1, fp);
    fclose(fp);
    
    return mem;
}

void read_from_file_offset(char* file, u64 offset, u64 size, void* dest)
{
    
    FILE* fp = fopen(file, "rb");
    fseek(fp, (i64)offset,SEEK_SET);
    fread(dest, size, 1, fp);
    fclose(fp);
}

b32 write_file(char *filename, u64 memory_size, void* memory)
{
    
    FILE* fp = fopen(filename, "wb");
    fwrite(memory,1,memory_size, fp);
    fclose(fp);
    return true;
}


#endif


#endif //UTIL_H
