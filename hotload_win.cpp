#include "includes.h"

internal Array<Hotloaded_File *> files;
internal HANDLE dir_change_notification;

void hotload_init() {
    char buf[1024] = {};
    GetCurrentDirectory(1024, buf);
    char *dir = format("%s\\data\\", buf);
	dir_change_notification = FindFirstChangeNotificationA(dir, TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE); //@todo: check if this works on other machines
    int err = GetLastError();
}

void hotload_shutdown() {
	FindCloseChangeNotification(dir_change_notification);
	for(int i = 0; i < files.count; i++) free(files[i]);
}

void hotload_add_file(const char *filename, void *data, Hotload_Callback callback) {
	Hotloaded_File *file = alloc(Hotloaded_File);
	file->filename = filename;
	file->callback = callback;
	file->data = data;
    array_add(&files, file);
    
	FILETIME file_time;
	HANDLE file_handle = CreateFileA(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	GetFileTime(file_handle, NULL, NULL, &file_time);
	CloseHandle(file_handle);
	file->last_write_low = file_time.dwLowDateTime;
	file->last_write_high = file_time.dwHighDateTime;
}

void hotload_check_files_non_blocking() {
	if (WaitForSingleObject(dir_change_notification, 0) == WAIT_OBJECT_0) {
		for(int i = 0; i < files.count; i++) {
			auto it = files[i];
			HANDLE file_handle = CreateFileA(it->filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (file_handle == INVALID_HANDLE_VALUE) { // could fail if an editor program is still holding onto the file
				continue;
			}
            
			FILETIME file_time;
			BOOL success = GetFileTime(file_handle, NULL, NULL, &file_time); // could fail if an editor program is still holding onto the file
			CloseHandle(file_handle);
            
			if (success) {
				FILETIME last_write_time;
				last_write_time.dwLowDateTime = it->last_write_low;
				last_write_time.dwHighDateTime = it->last_write_high;
				
				int res = CompareFileTime(&file_time, &last_write_time);
				if(res) {
					printf("Hotloading %s\n", it->filename); //@console
					it->last_write_low = file_time.dwLowDateTime;
					it->last_write_high = file_time.dwHighDateTime;
					it->callback(it->filename, it->data);
					break;
				}
                
				FindNextChangeNotification(dir_change_notification);
			}
		}
	}
}