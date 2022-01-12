#ifndef SH_TOOLS
#define SH_TOOLS
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float  f32;
typedef double f64;

typedef struct bhdr {
	int size;
	int cap;
} bhdr;

#define buf__hdr(b) ((bhdr*)(((int*)b) - 2))

#define buf_end(b) ((b) + buf_len(b))
#define buf_len(b) ((b) ? buf__hdr(b)->size: 0)
#define buf_cap(b) ((b) ? buf__hdr(b)->cap: 0)
#define buf_fit(b, n) ((n) <= buf_cap(b) ? 0 : ((b) = buf__grow(b, buf_len(b) + (n), sizeof(*(b)))))
#define buf_push(b, ...) ( buf_fit((b), 1 + buf_len(b)), (b)[buf__hdr(b)->size++] = (__VA_ARGS__))
#define buf_pop(b)		 ( buf_len(b) ? ((b) + --buf__hdr(b)->size) : NULL )
#define buf_free(b) ((b) ? (free(buf__hdr(b)), (b) = NULL): 0)
#define buf_clear(b) ((b) ? buf__hdr(b)->size = 0: 0)

void* buf__grow(void *buf, int items, int item_size) {
	int new_cap = items + 2*buf_cap(buf);
	int new_size = sizeof(int)*2 + new_cap*item_size;
	bhdr *nhdr = NULL;
	if(buf) {
		nhdr = (bhdr*) realloc(buf__hdr(buf), new_size);
	} else {
		nhdr = (bhdr *)malloc(new_size);
		nhdr->size = 0;
	}

	nhdr->cap = new_cap;
	return ((int*)nhdr) + 2;
}


FILETIME sh_get_file_last_write(char *filename) {
	WIN32_FILE_ATTRIBUTE_DATA file_attrib;
	int success = GetFileAttributesExA(filename, GetFileExInfoStandard, &file_attrib);

	if(success == 0) {
		printf("suomething is fucke\n");
	}

	assert(success == 1);

	return file_attrib.ftLastWriteTime;

}

bool sh_check_file_changed(char *filename, FILETIME *last_write_time, FILETIME *last_write_time_out) {
	WIN32_FILE_ATTRIBUTE_DATA file_attrib;
	bool success = GetFileAttributesExA(filename, GetFileExInfoStandard, &file_attrib);
	/* i32 last_error = GetLastError(); */
	
	bool result = 0;
	if(CompareFileTime(last_write_time, &file_attrib.ftLastWriteTime) == -1) {
		result = 1;
	}

	if(success && last_write_time_out != NULL) {
		*last_write_time_out = file_attrib.ftLastWriteTime;
	}
	
	return result && success;
}

char* read_file(const char *filename, size_t *size) {
	DWORD cur_size = 0;

	HANDLE file = CreateFile(
			filename,
			GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
			NULL);
	 cur_size = GetFileSize(file, NULL);

	char* mem = NULL;

	if(cur_size != INVALID_FILE_SIZE) {
		mem = (char*)malloc(cur_size+1);
		DWORD read = 0;
		ReadFile(file, mem, cur_size, &read, NULL);
		mem[cur_size] = 0;
		assert(read == cur_size);

		if(size) {
			*size = cur_size;
		}
	}

	CloseHandle(file);

	return mem;
}

void write_file(const char *filename, char *to_write, i32 to_write_bytes) {

	HANDLE file = CreateFile(
			filename,
			GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
			NULL);


	char* mem = NULL;

	DWORD written_bytes = 0;
	WriteFile(file,to_write, to_write_bytes, &written_bytes, NULL);

	assert(written_bytes == to_write_bytes);

	CloseHandle(file);
}



#define assert_exit(assert_condition, fmt, ...) \
	_assert_exit(assert_condition, __LINE__, __FILE__, __func__, fmt, __VA_ARGS__)

void _assert_exit(i32 assert_condition, i32 line_number, const char *file, const char *func, const char *fmt, ...) {

	if(!assert_condition) {
		va_list var_list;
		va_start(var_list, fmt);
		printf("\n%s(%d):(%s) ", file, line_number, func);
		vprintf(fmt, var_list);
		va_end(var_list);
		exit(1);
	}
}


#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#endif
