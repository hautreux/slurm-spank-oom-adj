/***************************************************************************\
 * slurm-spank-oom-adj.c - SLURM SPANK OOM Score Adjust plugin
 ***************************************************************************
 * Copyright  CEA/DAM/DIF (2008)
 *
 * Written by Matthieu Hautreux <matthieu.hautreux@cea.fr>
 *
 * Based on spank man page renice example.
 *
 * This file is part of slurm-spank-oom-adj, a SLURM SPANK Plugin aiming at 
 * adjusting OOM score of tasks spawned by SLURM.
 *
 * slurm-spank-oom-adj is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by the 
 * Free Software Foundation; either version 2 of the License, or (at your 
 * option) any later version.
 *
 * slurm-spank-oom-adj is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with slurm-spank-oom-adj; if not, write to the Free Software Foundation, 
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
\***************************************************************************/
/* Note : To compile: gcc -fPIC -shared -o spank-oom-adj.so spank-oom-adj.c */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <errno.h>

#include <slurm/spank.h>

/*
 * All spank plugins must define this macro for the SLURM plugin loader.
 */
SPANK_PLUGIN(oom-adj, 1);

static int oom_adj = 0;

static int _str2adj (const char *str, int *p2int);
static int _set_oom_adj(pid_t pid, int adj);

/*
 *  Do not provide an option
 */
struct spank_option spank_options[] =
{
	SPANK_OPTIONS_TABLE_END
};

/*
 *  Called from both srun and slurmd.
 */
int slurm_spank_init (spank_t sp, int ac, char **av)
{
	int i;

	/* Don’t do anything in sbatch/salloc
	 */
	if (spank_context () == S_CTX_ALLOCATOR)
		return (0);

	for (i = 0; i < ac; i++) {
		if (strncmp ("oom_adj=", av[i], 8) == 0) {
			const char *optarg = av[i] + 8;
			if (_str2adj (optarg, &oom_adj) < 0)
				slurm_error ("Ignoring invalid oom_adj "
					     "value: %s", av[i]);
		}
		else {
			slurm_error ("oom-adj: invalid option: %s", av[i]);
		}
	}

	if (!spank_remote (sp))
		slurm_verbose ("oom-adj: oom_adj = %d", oom_adj);

	return (0);
}

int slurm_spank_task_init_privileged (spank_t sp, int ac, char **av)
{
	pid_t pid;
	int taskid;

	if ( oom_adj == 0 )
		return (0);

	spank_get_item (sp, S_TASK_GLOBAL_ID, &taskid);
	pid = getpid();

	slurm_info ("set oom_adj of task%d pid %ld to %ld", 
		    taskid, pid, oom_adj);
	
	if (_set_oom_adj(pid, oom_adj) < 0) {
		slurm_error ("oom-adj: unable to set oom_adj: %m");
		return (-1);
	}

	return (0);
}

static int _str2adj (const char *str, int *p2int)
{
	long int l;
	char *p;

	l = strtol (str, &p, 10);
	if ((*p != '\0') || (l < -17) || (l > 15))
		return (-1);

	*p2int = (int) l;

	return (0);
}

static int _set_oom_adj(pid_t pid, int adj)
{
	int fd;
	char oom_adj_file[128];
	char oom_adj[16];

	if (snprintf(oom_adj_file, 128, "/proc/%ld/oom_adj", pid) >= 128) {
		return -1;
	}

	fd = open(oom_adj_file, O_WRONLY);
	if (fd < 0) {
		if (errno == ENOENT)
			debug("failed to open %s: %m",oom_adj_file);
		else
			error("failed to open %s: %m",oom_adj_file);
		return -1;
	}
	if (snprintf(oom_adj, 16, "%d", adj) >= 16) {
		close(fd);
		return -1;
	}
	while ((write(fd, oom_adj, strlen(oom_adj)) < 0) && (errno == EINTR))
		;
	close(fd);

	return 0;

}
