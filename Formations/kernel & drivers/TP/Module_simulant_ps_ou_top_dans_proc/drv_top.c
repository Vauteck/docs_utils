/********************************************
   Module       : modtop.o
   Loading      : insmod   ./modtop.o
                  modprobe ./modtop.o
   Unloading    : rmmod modtop
   Reading data : cat /proc/top
 ********************************************/

/* Main headers for every modules            */
#include <linux/kernel.h>      /* printk level             */
#include <linux/module.h>      /* kernel version, ...      */
#include <linux/proc_fs.h>     /* all the proc stuff       */
#include <linux/sched.h>       /* for current              */
#include <linux/tty.h>         /* for the tty declarations */
#include <linux/fs.h>          /* filesystem declaration   */
#include <linux/init.h>        /* Needed for the macros    */
#include <linux/errno.h>       /* error system definition  */
#include <linux/mm.h>          /* define vm_area_struct    */

/*#include <sys/types.h>*/
/*#include <stdio.h>*/
/*#include <fcntl.h>*/
/*#include <unistd.h>*/
/*#include <stdlib.h>*/
#include <errno.h>
/*#include <dirent.h>*/
#include <string.h>             /* strings definitions      */
/*#include <math.h>*/
/*#include <ctype.h>*/
/*#include <sys/time.h>*/
/*#include <sys/stat.h>*/
/*#include <sys/vfs.h>*/
/*#include <sys/param.h>*/		/* for HZ         */
/*#include <asm/page.h>*/		/* for PAGE_SHIFT */

/* Driver metadata */
#define PROC_NAME_ENTRY  "top"
#define PROC_RIGHTS      0644
#define DRV_LICENCE      "GPL"
#define DRV_ALIAS        "top"
#define DRV_AUTHOR       "GAYET Thierry"
#define DRV_DESC         "Embedded top"

#define PROCFS               "/proc"
#define HASH_SIZE	     (1003)
#define INITIAL_ACTIVE_SIZE  (256)
#define PROCBLOCK_SIZE       (32)

/* States that a process can take           */ 
/* -1 unrunable, 0 running, 1 sleeping, ... */
#define NPROCSTATES 7
static char* procstatenames[NPROCSTATES+1] = { "unrunable", " running ", " sleeping ", " uninterruptable ", " zombie ", " stopped ", " swapping ", NULL };

/* States that a CPU can take */ 
#define NCPUSTATES 4
static char* cpustatenames[NCPUSTATES+1] = { "user", "nice", "system", "idle", NULL };

/* usefull macros */
#define bytetok(x)	 (((x) + 512) >> 10)
#define pagetok(x)	 ((x) << (PAGE_SHIFT - 10))
#define HASH(x)		 (((x) * 1686629713U) % HASH_SIZE)

/* Definition of the new structures */
struct top_proc
{
    pid_t            pid;
    pid_t            ppid;
    uid_t            uid;
    char             name[64];
    int              pri;
    int              nice;
    unsigned long    size;
    unsigned long    rss;           /* in kB */
    int              state;
    unsigned long    time;
    unsigned long    otime;
    unsigned long    start_code;
    unsigned long    end_code;
    unsigned long    start_stack;
    unsigned int     threads;
    double           pcpu;
    double           wcpu;
    struct top_proc *next;
};

/* Read the parameters given */
typedef enum { e_PARAM_FILTER_PID=0, e_PARAM_FILTER_PROCESS_NAME=1, e_PARAM_FILTER_UNKNOW=2 } e_param_filter_t;
typedef enum { e_DISPLAYED=0, e_HIDDEN=1 } e_type_display_t;
#define PARAM_PID_DEFAULT           -1
#define PARAM_PROCESS_NAME_DEFAULT	""
static int             		paramPid         = PARAM_PID_DEFAULT;
static int             		paramCycle; 
static char*           		paramProcessName = PARAM_PROCESS_NAME_DEFAULT;
static e_param_filter_t     paramFilter      = e_PARAM_FILTER_UNKNOW;
MODULE_PARM(paramPid,         "i"); 	/* Pid number of the process to search */
MODULE_PARM(paramCycle,       "i"); 	/* Duration loop in second             */
MODULE_PARM(paramProcessName, "s"); 	/* name of the process to search       */

/* (Global definition) */
static long                   cp_time[NCPUSTATES];
static long                   cp_old[NCPUSTATES];
static long                   cp_diff[NCPUSTATES];
static struct timeval         lasttime;
static struct proc_dir_entry *pProcTop;
static struct top_proc       *ptable[HASH_SIZE];
static struct top_proc      **pactive;
static struct top_proc      **nextactive;
unsigned int                  activesize = 0;
static int                    cpu_states[NCPUSTATES];
static int                    process_states[NPROCSTATES];
static struct top_proc       *freelist  = NULL;
static struct top_proc       *procblock = NULL;
static struct top_proc       *procmax   = NULL;

/* Function which add a task */
static struct top_proc* proc_top_new_proc(void)
{
	struct top_proc *p;
	
	if (freelist)
	{
		p        = freelist;
		freelist = freelist->next;
	}
	else if (procblock)
	{
		p = procblock;
		if (++procblock >= procmax) { procblock = NULL; }
	}
	else
	{
		p       = procblock = (struct top_proc *)malloc(sizeof(struct top_proc) *PROCBLOCK_SIZE);
		procmax = procblock++ + PROCBLOCK_SIZE;
	}
	
	return p;
} /* proc_top_new_proc */

/* Function which clean the task list */
static void proc_top_free_proc(struct top_proc *proc)
{
	proc->next = freelist;
	freelist   = proc;
} /* proc_top_free_proc */

/* Function which give a listing of the running task */
static int proc_top_list_tasks(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int                 size      = 0; 			/* size                         					*/
	e_type_display_t	display   = e_HIDDEN;	/* say if the process must be displayed or hidded   */
	int                 nbtask    = 0; 			/* number of tasks displayed    					*/
	struct task_struct* p;             			/* pointer to the task list     					*/

	/* Lock the task listing */
	read_lock(&tasklist_lock);
		
	/* Display a header at the top of the /proc page */
	size = sprintf(page+size,                              \
					   "%5s%7s%7s%7s%10s%7s%7s  %s\n",     \ 
					   "PID",                              \
					   "UID",                              \
					   "PRID",                             \
					   "NICE",                             \
					   "STATE",                            \
					   "USER",                             \
					   "SYST",                             \
					   "COMMAND");
		
	/* Read all the task one by one */
	for_each_task(p)
	{
			printk( KERN_INFO "Current process: '%s' with a PID %d ", (char*)p->comm, 
																	  (int)p->pid );
			
			/* Check if a filter should be applied */
			switch (paramFilter)
			{
				case e_PARAM_FILTER_PID:
						if ((int)p->pid == paramPid) 					display = e_DISPLAYED; break;
						else                         					display = e_HIDDEN;	   break;
				case e_PARAM_FILTER_PROCESS_NAME:	
						if (0 == strcmp(p->comm, paramProcessName))		display = e_DISPLAYED; break;
						else                         					display = e_HIDDEN;	   break;
				case e_PARAM_FILTER_UNKNOW:								display = e_DISPLAYED; break;
				default:												display = e_DISPLAYED; break;
			} /* SWITCH */

			/* manage the information to display */
			if (e_DISPLAYED == display)
			{
				size += sprintf(LINE page+size, 					\
								"%5d%7d%7d%7d%10s%7d%7d%ld %s\n",	\
								(int)p->pid,                      	\
								(int)p->uid,                        \
								(int)p->rt_priority,                \
								(int)p->nice,                       \
								procstatenames[((int)p->state)],    \
								(int)p->times.tms_utime,            \
								(int)p->times.tms_stime,            \
								(unsigned long)p->mm->rss,          \
								p->comm);
			} /* IF */
				
			/* Count the number of process display */
			nbtask++;
	} /* IF */
		
	/* Display an error msg */
	if (nbtask==0)
	{ 
		printk( KERN_INFO "No process in the task list" ); 
		size += sprintf(page+size, "%s", "No process in the task list" );
	} /* IF */
		
	/* Unlock the task listing */
	read_unlock(&tasklist_lock);
	
	return(size);
} /* proc_top_list_tasks */

/* Init callback */
int init_proc_top_module(void)
{
	int	rc_init = 0;
	
	printk( KERN_INFO "--> init_proc_top_module() " );
	
	/* Detect if one filter have been provided either by a pid number or a process name */
	/* If not, it will display all the process                                          */ 
	if (e_PARAM_FILTER_PID_DEFAULT != paramPid) 
	{
		printk( KERN_INFO "Filter by PID #%d ", paramPid );
		paramFilter = e_PARAM_FILTER_PID;
	} else if (0 != strcmp(paramProcessName, PARAM_PROCESS_NAME_DEFAULT) {
		printk( KERN_INFO "Filter by process name '%s' ", paramProcessName );
		paramFilter = e_PARAM_FILTER_PROCESS_NAME;
	} else { 
		printk( KERN_INFO "No filter set." );
	} /* IF */

	/* Creating the /proc/top entry that will be used in read-only */
	if (NULL == create_proc_read_entry(PROC_NAME_ENTRY, PROC_RIGHTS, NULL, proc_top_list_task, NULL))
	{
		printk( KERN_INFO "Creating the /proc/%s entry : NOK ",  PROC_NAME_ENTRY );
		rc_init = -ENOMEM;
	} else {
		printk( KERN_INFO "Creating the /proc/%s entry : OK ",   PROC_NAME_ENTRY );
		printk( KERN_INFO "For reading the data : cat /proc/%s", PROC_NAME_ENTRY );
	} /* IF */
	
	/* Final return code */
	printk( KERN_INFO "<-- init_proc_top_module() - rc_init=%d ", rc_init );
	return(rc_init);
} /* init_proc_top_module */

/* Exit callback */
void cleanup_proc_top_module(void)
{
	printk( KERN_INFO "--> cleanup_proc_top_module() " );
	
	remove_proc_entry(PROC_NAME_ENTRY, NULL);
	printk( KERN_INFO "The '%s' module is now unloaded.", THIS_MODULE->name );
	
	printk( KERN_INFO "<-- cleanup_proc_top_module() " );
} /* cleanup_proc_top_module */

module_init(init_proc_top_module);
module_exit(cleanup_proc_top_module);

MODULE_LICENSE(DRV_LICENCE);
MODULE_AUTHOR(DRV_AUTHOR);
MODULE_ALIAS(DRV_ALIAS);
MODULE_DESCRIPTION(DRV_DESC);

/* EOF */
