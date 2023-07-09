#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include <dirent.h>
#include <windows.h>

void patch_dir(const char *name) {
	char target[256], target2[256];
	sprintf(target, "patched/%s", name);
	sprintf(target2, "pvr/%s", name);
	mkdir(target, 0777);
	mkdir(target2, 0777);
	DIR *d = opendir(name);
	struct dirent *entry;
	int i = 1;
	while (entry = readdir(d)) {
		if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
			char fname[256];
			sprintf(fname, "%s/%s", name, entry->d_name);
			if (entry->d_type == DT_DIR) {
				patch_dir(fname);
			} else if (strstr(fname, ".png")) {
				int w, h;
				uint8_t *buf = stbi_load(fname, &w, &h, NULL, 4);
				char params[1024];
				int w8 = w;
				while (w8 % 8) {
					w8++;
				}
				int h8 = h;
				while (h8 % 8) {
					h8++;
				}
				printf("(%d) Patching %s\n", i++, fname);
				sprintf(target2, "pvr/%s", fname);
				target2[strlen(target2) - 2] = 'v';
				target2[strlen(target2) - 1] = 'r';
				sprintf(params, "-ics lRGB -f PVRTCII_2BPP,UB,lRGB -i %s -o %s -rcanvas %d,%d", fname, target2, w8, h8);
				SHELLEXECUTEINFO ShExecInfo = {0};
				ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
				ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
				ShExecInfo.hwnd = NULL;
				ShExecInfo.lpVerb = "open";
				ShExecInfo.lpFile = "PVRTexToolCLI.exe";        
				ShExecInfo.lpParameters = params;   
				ShExecInfo.lpDirectory = NULL;
				ShExecInfo.nShow = SW_SHOW;
				ShExecInfo.hInstApp = NULL; 
				ShellExecuteEx(&ShExecInfo);
				WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
				CloseHandle(ShExecInfo.hProcess);
				memcpy(buf, fname, strlen(fname) + 1);
				sprintf(target, "patched/%s", fname);
				stbi_write_png(target, w, h, 4, buf, 4 * w);
				free(buf);
			}
		}
	}
	closedir(d);
}

int main() {
	mkdir("patched");
	mkdir("pvr");
	patch_dir("images");
	return 0;
}