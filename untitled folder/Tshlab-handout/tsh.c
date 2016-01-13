/*
 * tsh - A tiny shell program with job control
 *
 * <Xinyun (Victor) Zhao Andrew ID: xinyunzh>
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

/* Misc manifest constants */
#define MAXLINE    1024   /* max line size */
#define MAXARGS     128   /* max args on a command line */
#define MAXJOBS      16   /* max jobs at any point in time */
#define MAXJID    1<<16   /* max job ID */

/* Job states */
#define UNDEF         0   /* undefined */
#define FG            1   /* running in foreground */
#define BG            2   /* running in background */
#define ST            3   /* stopped */

/*
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */

/* Parsing states */
#define ST_NORMAL   0x0   /* next token is an argument */
#define ST_INFILE   0x1   /* next token is the input file */
#define ST_OUTFILE  0x2   /* next token is the output file */

/* IO */
#define DEF_MODE S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH

/* Global variables */
extern char **environ;      /* defined in libc */
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
int nextjid = 1;            /* next job ID to allocate */
char sbuf[MAXLINE];         /* for composing sprintf messages */

struct job_t {              /* The job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line */
};
struct job_t job_list[MAXJOBS]; /* The job list */

struct cmdline_tokens {
    int argc;               /* Number of arguments */
    char *argv[MAXARGS];    /* The arguments list */
    char *infile;           /* The input file */
    char *outfile;          /* The output file */
    enum builtins_t {       /* Indicates if argv[0] is a builtin command */
        BUILTIN_NONE,
        BUILTIN_QUIT,
        BUILTIN_JOBS,
        BUILTIN_BG,
        BUILTIN_FG} builtins;
};

/* End global variables */


/* Function prototypes */
void eval(char *cmdline);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, struct cmdline_tokens *tok);
void sigquit_handler(int sig);

void clearjob(struct job_t *job);
void initjobs(struct job_t *job_list);
int maxjid(struct job_t *job_list);
int addjob(struct job_t *job_list, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *job_list, pid_t pid);
pid_t fgpid(struct job_t *job_list);
struct job_t *getjobpid(struct job_t *job_list, pid_t pid);
struct job_t *getjobjid(struct job_t *job_list, int jid);
int pid2jid(pid_t pid);
void listjobs(struct job_t *job_list, int output_fd);

void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

/* Proto from csapp.h */
void Sigfillset(sigset_t *set);
void Sigemptyset(sigset_t *set);
void Sigaddset(sigset_t *set, int signum);
void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
void Execve(const char *filename, char *const argv[], char *const envp[]);
pid_t Fork(void);
int Sigsuspend(const sigset_t *set);
int Dup2(int fd1, int fd2);
int Open(const char *pathname, int flags, mode_t mode);
void Close(int fd);
void Kill(pid_t pid, int signum);
pid_t Waitpid(pid_t pid, int *iptr, int options);
void Setpgid(pid_t pid, pid_t pgid);
pid_t Getpgrp(void);

/* My helper proto */
void open_dup2_in(char *outfile);
void open_dup2_out(char *infile);
void builtin_jobs_handler(struct cmdline_tokens *tok);
struct job_t * id2job(struct cmdline_tokens *tok, int argc);
void wait_fg_job(int state);
void builtin_fg_handler(struct cmdline_tokens *tok);
void builtin_bg_handler(struct cmdline_tokens *tok);
int builtin_command(struct cmdline_tokens *tok);




/*
 * main - The shell's main routine
 */
int
main(int argc, char **argv)
{
    char c;
    char cmdline[MAXLINE];    /* cmdline for fgets */
    int emit_prompt = 1; /* emit prompt (default) */


    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
            break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
            break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
            break;
        default:
            usage();
        }
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */
    Signal(SIGTTIN, SIG_IGN);
    Signal(SIGTTOU, SIG_IGN);

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler);

    /* Initialize the job list */
    initjobs(job_list);


    /* Execute the shell's read/eval loop */
    while (1) {

        if (emit_prompt) {
            printf("%s", prompt);
            fflush(stdout);
        }
        if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
            app_error("fgets error");
        if (feof(stdin)) {
            /* End of file (ctrl-d) */
            printf ("\n");
            fflush(stdout);
            fflush(stderr);
            exit(0);
        }

        /* Remove the trailing newline */
        cmdline[strlen(cmdline)-1] = '\0';

        /* Evaluate the command line */
        eval(cmdline);

        fflush(stdout);
        fflush(stdout);
    }

    exit(0); /* control never reaches here */
}

/*
 * eval - Evaluate the command line that the user has just typed in
 *
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.
 */
void
eval(char *cmdline)
{
    int bg;              /* should the job run in bg or fg? */
    struct cmdline_tokens tok;
    sigset_t prev, mask_all;
    int state;
    pid_t pid;
    struct job_t *job_ins;

    /* Parse command line */
    bg = parseline(cmdline, &tok);

    if (bg == -1) /* parsing error */
        return;
    if (tok.argv[0] == NULL) /* ignore empty lines */
        return;

    /* Fig. 8.40 */
    if (!builtin_command(&tok)) {
        Sigemptyset(&mask_all);
        Sigaddset(&mask_all, SIGCHLD);
        Sigaddset(&mask_all, SIGINT);
        Sigaddset(&mask_all, SIGTSTP);
        Sigprocmask(SIG_BLOCK, &mask_all, &prev); /* Block at beginning */

        if ((pid = Fork()) == 0) { /* Child process */
            Sigprocmask(SIG_SETMASK, &prev, NULL); /* Unblock inside child */
	    Setpgid(0, 0); /* one proc and shell in pgrp */

	    open_dup2_in(tok.infile);
	    open_dup2_out(tok.outfile);
	    if (execve(tok.argv[0], tok.argv, environ) < 0) {
	        printf("%s: Command not found.\n", tok.argv[0]);
	        exit(0);
	    }
	}

        /* Parent process */

        state = bg ? BG : FG;
        addjob(job_list, pid, state, cmdline);
        job_ins = getjobpid(job_list, pid);

        Sigprocmask(SIG_SETMASK, &prev, NULL); /* Unblock 3 signals */

        if (bg) {
            printf("[%d] (%d) %s", pid2jid(pid), pid, cmdline);
        } else {
            wait_fg_job(job_ins -> state);
        }
    }

    return;
}

/*
 * parseline - Parse the command line and build the argv array.
 *
 * Parameters:
 *   cmdline:  The command line, in the form:
 *
 *                command [arguments...] [< infile] [> oufile] [&]
 *
 *   tok:      Pointer to a cmdline_tokens structure. The elements of this
 *             structure will be populated with the parsed tokens. Characters
 *             enclosed in single or double quotes are treated as a single
 *             argument.
 * Returns:
 *   1:        if the user has requested a BG job
 *   0:        if the user has requested a FG job
 *  -1:        if cmdline is incorrectly formatted
 *
 * Note:       The string elements of tok (e.g., argv[], infile, outfile)
 *             are statically allocated inside parseline() and will be
 *             overwritten the next time this function is invoked.
 */
int
parseline(const char *cmdline, struct cmdline_tokens *tok)
{

    static char array[MAXLINE];          /* holds local copy of command line */
    const char delims[10] = " \t\r\n";   /* argument delimiters (white-space) */
    char *buf = array;                   /* ptr that traverses command line */
    char *next;                          /* ptr to the end of the current arg */
    char *endbuf;                        /* ptr to end of cmdline string */
    int is_bg;                           /* background job? */

    int parsing_state;                   /* indicates if the next token is the
                                            input or output file */

    if (cmdline == NULL) {
        (void) fprintf(stderr, "Error: command line is NULL\n");
        return -1;
    }

    (void) strncpy(buf, cmdline, MAXLINE);
    endbuf = buf + strlen(buf);

    tok->infile = NULL;
    tok->outfile = NULL;

    /* Build the argv list */
    parsing_state = ST_NORMAL;
    tok->argc = 0;

    while (buf < endbuf) {
        /* Skip the white-spaces */
        buf += strspn (buf, delims);
        if (buf >= endbuf) break;

        /* Check for I/O redirection specifiers */
        if (*buf == '<') {
            if (tok->infile) {
                (void) fprintf(stderr, "Error: Ambiguous I/O redirection\n");
                return -1;
            }
            parsing_state |= ST_INFILE;
            buf++;
            continue;
        }
        if (*buf == '>') {
            if (tok->outfile) {
                (void) fprintf(stderr, "Error: Ambiguous I/O redirection\n");
                return -1;
            }
            parsing_state |= ST_OUTFILE;
            buf ++;
            continue;
        }

        if (*buf == '\'' || *buf == '\"') {
            /* Detect quoted tokens */
            buf++;
            next = strchr (buf, *(buf-1));
        } else {
            /* Find next delimiter */
            next = buf + strcspn (buf, delims);
        }

        if (next == NULL) {
            /* Returned by strchr(); this means that the closing
               quote was not found. */
            (void) fprintf (stderr, "Error: unmatched %c.\n", *(buf-1));
            return -1;
        }

        /* Terminate the token */
        *next = '\0';

        /* Record the token as either the next argument or the i/o file */
        switch (parsing_state) {
        case ST_NORMAL:
            tok->argv[tok->argc++] = buf;
            break;
        case ST_INFILE:
            tok->infile = buf;
            break;
        case ST_OUTFILE:
            tok->outfile = buf;
            break;
        default:
            (void) fprintf(stderr, "Error: Ambiguous I/O redirection\n");
            return -1;
        }
        parsing_state = ST_NORMAL;

        /* Check if argv is full */
        if (tok->argc >= MAXARGS-1) break;

        buf = next + 1;
    }

    if (parsing_state != ST_NORMAL) {
        (void) fprintf(stderr,
                       "Error: must provide file name for redirection\n");
        return -1;
    }

    /* The argument list must end with a NULL pointer */
    tok->argv[tok->argc] = NULL;

    if (tok->argc == 0)  /* ignore blank line */
        return 1;

    if (!strcmp(tok->argv[0], "quit")) {                 /* quit command */
        tok->builtins = BUILTIN_QUIT;
    } else if (!strcmp(tok->argv[0], "jobs")) {          /* jobs command */
        tok->builtins = BUILTIN_JOBS;
    } else if (!strcmp(tok->argv[0], "bg")) {            /* bg command */
        tok->builtins = BUILTIN_BG;
    } else if (!strcmp(tok->argv[0], "fg")) {            /* fg command */
        tok->builtins = BUILTIN_FG;
    } else {
        tok->builtins = BUILTIN_NONE;
    }

    /* Should the job run in the background? */
    if ((is_bg = (*tok->argv[tok->argc-1] == '&')) != 0)
        tok->argv[--tok->argc] = NULL;

    return is_bg;
}


/*****************
 * Signal handlers
 *****************/

/*
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP, SIGTSTP, SIGTTIN or SIGTTOU signal. The
 *     handler reaps all available zombie children, but doesn't wait
 *     for any other currently running children to terminate.
 */
void
sigchld_handler(int sig)
{
    /* Fig. 8.41 8.18 */
    int olderrno = errno;
    int status;
    pid_t p_id;
    struct job_t *job_ins;
    sigset_t prev, mask_all;

    Sigemptyset(&mask_all);
    Sigaddset(&mask_all, SIGCHLD);
    Sigaddset(&mask_all, SIGINT);
    Sigaddset(&mask_all, SIGTSTP);

    while((p_id = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        /* Normally finish/exit */
        if (WIFEXITED(status)) {
          Sigprocmask(SIG_BLOCK, &mask_all, &prev); /* Block */
          deletejob(job_list, p_id);
          Sigprocmask(SIG_SETMASK, &prev, NULL); /* Unblock */
        } else if (WIFSIGNALED(status)) {
            /* Terminated by other signal */
            /* Order is fixed, otherwise it cause bugs. */
            printf("Job [%d] (%d) terminated by signal %d\n",
	          pid2jid(p_id), p_id, WTERMSIG(status));
            Sigprocmask(SIG_BLOCK, &mask_all, &prev); /* Block */
            deletejob(job_list, p_id);
            Sigprocmask(SIG_SETMASK, &prev, NULL); /* Unblock */
        } else if (WIFSTOPPED(status)) {
            /* Stopped by signal, need to modified the state to stopped */
            /* Order is fixed, otherwise it cause bugs. */
            printf("Job [%d] (%d) stopped by signal %d\n",
	          pid2jid(p_id), p_id, WSTOPSIG(status));
            job_ins = getjobpid(job_list, p_id);
            job_ins -> state = ST;
        }
    }

    errno = olderrno;
}

/*
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.
 */
void
sigint_handler(int sig)
{
    /* Just send the signal to the target group that's it */
    int olderrno = errno;
    pid_t p_id;

    if ((p_id = fgpid(job_list)) != 0) {
        Kill(-p_id, SIGINT);
    }

    errno = olderrno;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.
 */
void
sigtstp_handler(int sig)
{
    int olderrno = errno;
    pid_t p_id;

    if ((p_id = fgpid(job_list)) != 0) {
        Kill(-p_id, SIGTSTP);
    }

    errno = olderrno;
}

/*********************
 * End signal handlers
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/

/* clearjob - Clear the entries in a job struct */
void
clearjob(struct job_t *job) {
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void
initjobs(struct job_t *job_list) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
        clearjob(&job_list[i]);
}

/* maxjid - Returns largest allocated job ID */
int
maxjid(struct job_t *job_list)
{
    int i, max=0;

    for (i = 0; i < MAXJOBS; i++)
        if (job_list[i].jid > max)
            max = job_list[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
int
addjob(struct job_t *job_list, pid_t pid, int state, char *cmdline)
{
    int i;

    if (pid < 1)
        return 0;

    for (i = 0; i < MAXJOBS; i++) {
        if (job_list[i].pid == 0) {
            job_list[i].pid = pid;
            job_list[i].state = state;
            job_list[i].jid = nextjid++;
            if (nextjid > MAXJOBS)
                nextjid = 1;
            strcpy(job_list[i].cmdline, cmdline);
            if(verbose){
                printf("Added job [%d] %d %s\n",
                       job_list[i].jid,
                       job_list[i].pid,
                       job_list[i].cmdline);
            }
            return 1;
        }
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int
deletejob(struct job_t *job_list, pid_t pid)
{
    int i;

    if (pid < 1)
        return 0;

    for (i = 0; i < MAXJOBS; i++) {
        if (job_list[i].pid == pid) {
            clearjob(&job_list[i]);
            nextjid = maxjid(job_list)+1;
            return 1;
        }
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t
fgpid(struct job_t *job_list) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
        if (job_list[i].state == FG)
            return job_list[i].pid;
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
struct job_t
*getjobpid(struct job_t *job_list, pid_t pid) {
    int i;

    if (pid < 1)
        return NULL;
    for (i = 0; i < MAXJOBS; i++)
        if (job_list[i].pid == pid)
            return &job_list[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *job_list, int jid)
{
    int i;

    if (jid < 1)
        return NULL;
    for (i = 0; i < MAXJOBS; i++)
        if (job_list[i].jid == jid)
            return &job_list[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID */
int
pid2jid(pid_t pid)
{
    int i;

    if (pid < 1)
        return 0;
    for (i = 0; i < MAXJOBS; i++)
        if (job_list[i].pid == pid) {
            return job_list[i].jid;
        }
    return 0;
}

/* listjobs - Print the job list */
void
listjobs(struct job_t *job_list, int output_fd)
{
    int i;
    char buf[MAXLINE];

    for (i = 0; i < MAXJOBS; i++) {
        memset(buf, '\0', MAXLINE);
        if (job_list[i].pid != 0) {
            sprintf(buf, "[%d] (%d) ", job_list[i].jid, job_list[i].pid);
            if(write(output_fd, buf, strlen(buf)) < 0) {
                fprintf(stderr, "Error writing to output file\n");
                exit(1);
            }
            memset(buf, '\0', MAXLINE);
            switch (job_list[i].state) {
            case BG:
                sprintf(buf, "Running    ");
                break;
            case FG:
                sprintf(buf, "Foreground ");
                break;
            case ST:
                sprintf(buf, "Stopped    ");
                break;
            default:
                sprintf(buf, "listjobs: Internal error: job[%d].state=%d ",
                        i, job_list[i].state);
            }
            if(write(output_fd, buf, strlen(buf)) < 0) {
                fprintf(stderr, "Error writing to output file\n");
                exit(1);
            }
            memset(buf, '\0', MAXLINE);
            sprintf(buf, "%s\n", job_list[i].cmdline);
            if(write(output_fd, buf, strlen(buf)) < 0) {
                fprintf(stderr, "Error writing to output file\n");
                exit(1);
            }
        }
    }
}
/******************************
 * end job list helper routines
 ******************************/


/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void
usage(void)
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 */
void
unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
void
app_error(char *msg)
{
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 */
handler_t
*Signal(int signum, handler_t *handler)
{
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
        unix_error("Signal error");
    return (old_action.sa_handler);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void
sigquit_handler(int sig)
{
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}

/* helper from csapp.c */
void Sigfillset(sigset_t *set)
{
    if (sigfillset(set) < 0)
	unix_error("Sigfillset error");
    return;
}

void Sigemptyset(sigset_t *set)
{
    if (sigemptyset(set) < 0)
	unix_error("Sigemptyset error");
    return;
}

void Sigaddset(sigset_t *set, int signum)
{
    if (sigaddset(set, signum) < 0)
	      unix_error("Sigaddset error");
    return;
}

void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
    if (sigprocmask(how, set, oldset) < 0)
	      unix_error("Sigprocmask error");
    return;
}

void Execve(const char *filename, char *const argv[], char *const envp[])
{
    if (execve(filename, argv, envp) < 0)
	      unix_error("Execve error");
}

pid_t Fork(void)
{
    pid_t pid;

    if ((pid = fork()) < 0)
	      unix_error("Fork error");
    return pid;
}

int Sigsuspend(const sigset_t *set)
{
    int rc = sigsuspend(set); /* always returns -1 */
    if (errno != EINTR)
        unix_error("Sigsuspend error");
    return rc;
}

int Dup2(int fd1, int fd2)
{
    int rc;

    if ((rc = dup2(fd1, fd2)) < 0)
	      unix_error("Dup2 error");
    return rc;
}

int Open(const char *pathname, int flags, mode_t mode)
{
    int rc;

    if ((rc = open(pathname, flags, mode))  < 0)
	      unix_error("Open error");
    return rc;
}

void Close(int fd)
{
    int rc;

    if ((rc = close(fd)) < 0)
	      unix_error("Close error");
}

void Kill(pid_t pid, int signum)
{
    int rc;

    if ((rc = kill(pid, signum)) < 0)
	      unix_error("Kill error");
}

pid_t Waitpid(pid_t pid, int *iptr, int options)
{
    pid_t retpid;

    if ((retpid  = waitpid(pid, iptr, options)) < 0)
	      unix_error("Waitpid error");
    return(retpid);
}

void Setpgid(pid_t pid, pid_t pgid) {
    int rc;

    if ((rc = setpgid(pid, pgid)) < 0)
	      unix_error("Setpgid error");
    return;
}

pid_t Getpgrp(void) {
    return getpgrp();
}

/* Helper by myself */

/*
 * builtin_command - 8.24 If first arg is a builtin command
 * , run it and return true
 */
int builtin_command(struct cmdline_tokens *tok) {
  switch(tok -> builtins) {
  case BUILTIN_QUIT: /* quit command */
      exit(0);
  case BUILTIN_JOBS:
      builtin_jobs_handler(tok);
      return 1;
  case BUILTIN_BG:
      builtin_bg_handler(tok);
      return 1;
  case BUILTIN_FG:
      builtin_fg_handler(tok);
      return 1;
  default: /* include BUILTIN_NONE */
      return 0;
  }
}


/*
 * builtin_bg_handler - Change a stopped background job
 * into a running background job
 */
void builtin_bg_handler(struct cmdline_tokens *tok) {
    /* TODO */
    sigset_t prev, mask_all;

    Sigemptyset(&mask_all);
    Sigaddset(&mask_all, SIGCHLD);
    Sigaddset(&mask_all, SIGINT);
    Sigaddset(&mask_all, SIGTSTP);
    struct job_t *job_ins = id2job(tok, tok -> argc);

    /* Fire the signal */
    if (ST == (job_ins -> state)) {
      Sigprocmask(SIG_BLOCK, &mask_all, &prev); /* Block */
      job_ins -> state = BG;
      Sigprocmask(SIG_SETMASK, &prev, NULL); /* Unblock */

        Kill(job_ins -> pid, SIGCONT);
        printf("[%d] (%d) %s\n",
          job_ins -> jid, job_ins -> pid, job_ins -> cmdline);
    }
}

/*
 * builtin_fg_handler - Change a stopped or running
 * background job into a running foreground job
 */
void builtin_fg_handler(struct cmdline_tokens *tok) {
    /* TODO refactored */
    sigset_t prev, mask_all;

    Sigemptyset(&mask_all);
    Sigaddset(&mask_all, SIGCHLD);
    Sigaddset(&mask_all, SIGINT);
    Sigaddset(&mask_all, SIGTSTP);

    struct job_t *job_ins = id2job(tok, tok -> argc);

    /* Modify the status of candidate process, fire the signal */
    Sigprocmask(SIG_BLOCK, &mask_all, &prev); /* Block */
    job_ins -> state = FG;
    Sigprocmask(SIG_SETMASK, &prev, NULL); /* Unblock */

    Kill(job_ins -> pid, SIGCONT);

    /* Wait to stop */
    wait_fg_job(job_ins -> state);
}

/* wait_fg_job - Use Sigsuspend to save some time :-) */
void wait_fg_job(int state) {
    sigset_t emptymask;
    Sigemptyset(&emptymask);
    while ((fgpid(job_list) != 0) && (FG == state)) {
        Sigsuspend(&emptymask);
    }
}

/* Updated: Make it safer */
/* id2job - parse the id and get job */
struct job_t * id2job(struct cmdline_tokens *tok, int argc) {
    struct job_t *job_ins = NULL;
    int job_id;
    int p_id;

    if ((tok -> argv[argc - 1][0]) == '%') {
        /* Start with %, parse it as jid */
        job_id = atoi(tok -> argv[argc - 1] + 1); /* Skip % */
        if ((job_ins = getjobjid(job_list, job_id)) == NULL) {
            printf("%%%d: No such job\n", job_id);
        }
    } else if (isdigit(tok -> argv[argc - 1][0])) {
        /* Start with number, parse it as pid */
        p_id = atoi(tok -> argv[argc - 1]);
        if ((job_ins = getjobpid(job_list, p_id)) == NULL) {
            printf("(%d): No such process\n", p_id);
        }
    } else {
        if (BUILTIN_BG == (tok -> builtins)) {
            printf("bg: argument must be a pid or %%jobid\n");
        } else if (BUILTIN_FG == (tok -> builtins)) {
            printf("fg: argument must be a pid or %%jobid\n");
        }
    }

    return job_ins;
}

/* builtin_jobs_handler - List the running and stopped background jobs */
void builtin_jobs_handler(struct cmdline_tokens *tok) {
    int fd;

    if (tok -> outfile != NULL) {
        if ((fd = Open(tok -> outfile, O_CREAT | O_WRONLY, DEF_MODE)) > 0) {
            listjobs(job_list, fd);
            Close(fd);
        }
    } else {
        listjobs(job_list, STDOUT_FILENO);
    }
}

/* open_dup2_in - redirect stdin to input file descripter */
void open_dup2_in(char *infile) {
    int fd;

    if (infile != NULL) {
        if ((fd = Open(infile, O_RDONLY, 0)) > 0) {
            if (Dup2(fd, STDIN_FILENO) > 0) {
	        Close(fd);
            }
        }
    }
}

/* open_dup2_out - redirect stdout to output file descripter */
void open_dup2_out(char *outfile) {
    int fd;

    if (outfile != NULL) {
        /* If not exist, then create */
        if ((fd = Open(outfile, O_CREAT | O_WRONLY, DEF_MODE)) > 0) {
            if (Dup2(fd, STDOUT_FILENO) > 0) {
	        Close(fd);
            }
        }
    }
}
