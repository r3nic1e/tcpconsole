//
// Created by mlosev on 31.03.17.
//

#include "defines.h"
#include "io.h"

int dump_virtual_console(int fd_out, int fd_in) {
	struct {
		char lines, cols, x, y;
	} scrn;
	int x, y;

	if (lseek(fd_in, 0, SEEK_SET) == -1) {
		return sockerror(fd_out, "lseek");
	}

	if (read(fd_in, &scrn, 4) == -1) {
		return sockerror(fd_out, "read on vcs");
	}

	for (y = 0; y < scrn.lines; y++) {
		int nspaces = 0;

		for (x = 0; x < scrn.cols; x++) {
			int loop;
			char ca[2];

			if (read(fd_in, ca, 2) == -1) {
				return sockerror(fd_out, "read on vcs (data)");
			}

			if (ca[0] != ' ') {
				for (loop = 0; loop < nspaces; loop++)
					sockprint(fd_out, " ");
				nspaces = 0;

				sockprint(fd_out, "%c", ca[0]);
			} else {
				nspaces++;
			}
		}

		if (sockprint(fd_out, "\n") == -1) {
			return -1;
		}
	}

	return 0;
}

int dump_dmesg(int fd, char *dmesg_buffer, int dmesg_buffer_size, char clear) {
	unsigned int loop;
	unsigned long nread = (unsigned long) klogctl(clear ? 4 : 3, dmesg_buffer, dmesg_buffer_size);
	if (nread <= 0) {
		return sockerror(fd, "klogctl(3)");
	}

	dmesg_buffer[nread] = 0x00;

	for (loop = 0; loop < nread; loop++) {
		if ((dmesg_buffer[loop] < 32 && dmesg_buffer[loop] != 10)
			|| dmesg_buffer[loop] > 126) {
			dmesg_buffer[loop] = ' ';
		}
	}

	return WRITE(fd, dmesg_buffer, nread);
}

int set_dmesg_loglevel(int fd, int level) {
	if (klogctl(8, NULL, level) == -1) {
		return sockerror(fd, "klogctl(8)");
	}

	return sockprint(fd, "dmesg loglevel set to %d\n", level);
}

int dump_loadavg(int fd) {
	double avg[3];

	if (getloadavg(avg, 3) == -1) {
		return sockerror(fd, "getloadavg(3)");
	}

	return sockprint(fd, "load: 1min: %f, 5min: %f, 15min: %f\n", avg[0],
					 avg[1], avg[2]);
}

int dump_ps(int fd) {
	int rc = 0, tnprocs = 0, tnthreads = 0, use_cmdline = 0;
	struct dirent *de;
	char *entered;
	DIR *dirp = opendir("/proc");

	static char process_name[120];

	// get process name
	if (sockprint(fd, "Process name (q to abort) []: ") == -1)
		return -1;

	entered = get_string(fd);
	if (!entered)
		return -1;

	if (strcmp(entered, "q") == 0) {
		free(entered);
		return 0;
	}

	strcpy(process_name, entered);

	// get use_cmdline option
	if (sockprint(fd, "Use cmdline? (0-1, q to abort) [0]: ") == -1)
		return -1;

	entered = get_string(fd);
	if (!entered)
		return -1;

	if (strcmp(entered, "q") == 0) {
		free(entered);
		return 0;
	}

	use_cmdline = atoi(entered);

	while ((de = readdir(dirp)) != NULL) {
		if (!isdigit(de->d_name[0]))
			continue;

		FILE *fh;
		static char path[128];
		int found = 0;

		static char fname[4096], dummystr[2];
		int dummy = 0, nthreads = 0, ppid = 0, rss = 0;

		snprintf(path, sizeof(path), "/proc/%s/stat", de->d_name);

		fh = fopen(path, "r");
		if (fh) {
			rc = fscanf(fh,
						"%d %s %c %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
						&dummy, fname, &dummystr[0], &ppid,
						&dummy, &dummy, &dummy, &dummy, &dummy,
						&dummy, &dummy, &dummy, &dummy, &dummy,
						&dummy, &dummy, &dummy, &dummy, &dummy,
						&nthreads, &dummy, &dummy, &dummy, &rss);

			if (rc == -1) {
				fclose(fh);
				sockerror(fd, "fscanf error");
				continue;
			}

			if (strstr(fname, process_name) != NULL) {
				sockprint(fd, "%2s %5s, ppid %5d, threads: %2d, rss: %5d, comm: %18s",
						  dummystr, de->d_name, ppid, nthreads, rss, fname);

				tnprocs++;
				tnthreads += nthreads;

				found = 1;
			}

			fclose(fh);
		}

		if (!use_cmdline) {
			if (found)
				sockprint(fd, "\n");
			continue;
		}

		snprintf(path, sizeof(path), "/proc/%s/cmdline", de->d_name);
		fh = fopen(path, "r");
		if (fh) {
			size_t len;
			unsigned int loop;
			static char cmdline[4096];

			len = fread(cmdline, 1, sizeof(cmdline) - 1, fh);
			if (len > 0) {
				cmdline[len] = 0x00;
				for (loop = 0; loop < len; loop++) {
					if (cmdline[loop] == 0x00)
						cmdline[loop] = ' ';
				}
			} else {
				memset(&cmdline[0], 0, sizeof(cmdline));
			}

			if (strstr(cmdline, process_name) != NULL) {
				if (!found) {
					sockprint(fd, "%2s %5s, ppid %5d, threads: %2d, rss: %5d, comm: %18s",
							  dummystr, de->d_name, ppid, nthreads, rss, fname);

					tnprocs++;
					tnthreads += nthreads;
				}

				sockprint(fd, " %s", cmdline);

				found = 1;
			}

			fclose(fh);
		}

		if (found)
			sockprint(fd, "\n");
	}

	closedir(dirp);

	rc |= sockprint(fd, "# procs: %d, # threads: %d\n", tnprocs, tnthreads);

	return rc;
}