/*
 * Démon:
 *   Enregistre les messages dans le journal central (via syslog)
 *   Utilise un verrou sur fichier pour prévenir plus d'un lancement du programme
 *   Change l'utilisateur effectif
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>

/* Nom du démon */
#define DAEMON_NAME "mydaemon"

/* Nom du fichier verrou */
#define LOCKFILE "/tmp/mylock"

/* Utilisateur effectif en lieu de root */
#define RUN_AS_USER "nobody"


static void child_handler(int signum)
{
	switch(signum) {
	case SIGALRM:
		exit(EXIT_FAILURE);
		break;
	case SIGUSR1:
		exit(EXIT_SUCCESS);
		break;
	case SIGCHLD:
		exit(EXIT_FAILURE);
		break;
	default:
		break;
	}
}

static void daemonize(const char *lockfile)
{
	pid_t pid, sid, parent;
	int lfp = -1;

	/* Déjà un démon */
	if (getppid() == 1) {
		return;
	}

	/* Création du fichier verrou */
	if (lockfile && lockfile[0]) {
		lfp = open(lockfile, O_RDWR | O_CREAT | O_EXCL, 0666);
		if (lfp < 0) {
			syslog(LOG_ERR, "unable to create lock file %s, code=%d (%s)",
					lockfile, errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	/* Change l'utilisateur si exécuté par root */
	if (getuid() == 0 || geteuid() == 0) {
		struct passwd *pw = getpwnam(RUN_AS_USER);
		if (pw) {
			syslog(LOG_NOTICE, "setting user to " RUN_AS_USER);
			setuid(pw->pw_uid);
		}
	}

	/* Signaux susceptibles d'arriver */
	signal(SIGCHLD, child_handler);
	signal(SIGUSR1, child_handler);
	signal(SIGALRM, child_handler);

	/* Création d'un processus */
	pid = fork();
	if (pid < 0) {
		syslog( LOG_ERR, "unable to fork daemon, code=%d (%s)",
				errno, strerror(errno));
		unlink(LOCKFILE);
		exit(EXIT_FAILURE);
	}

	/* Processus parent */
	if (pid > 0) {
		/* Attente de la confirmation du fils par un signal, ou 2 secondes */
		alarm(2);
		pause();

		exit(EXIT_FAILURE);
	}

	/* Processus fils */
	parent = getppid();

	/* Masque certain signaux */
	signal(SIGCHLD, SIG_DFL);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGHUP,  SIG_IGN);
	signal(SIGTERM, SIG_DFL);

	/* Change le masque des permissions */
	umask(0);

	/* Nouvelle session */
	sid = setsid();
	if (sid < 0) {
		syslog(LOG_ERR, "unable to create a new session, code %d (%s)",
				errno, strerror(errno));
		unlink(LOCKFILE);
		exit(EXIT_FAILURE);
	}

	/* Change le répertoire courant */
	if ((chdir("/")) < 0) {
		syslog(LOG_ERR, "unable to change directory to %s, code %d (%s)",
				"/", errno, strerror(errno));
		unlink(LOCKFILE);
		exit(EXIT_FAILURE);
	}

	/* Redirige les E/S standards vers /dev/null */
	if (freopen("/dev/null", "r", stdin) != stdin || \
		freopen("/dev/null", "w", stdout) != stdout || \
		freopen("/dev/null", "w", stderr) != stderr) {
		syslog(LOG_ERR, "unable to change reopen standard IOs");
		unlink(LOCKFILE);
		exit(EXIT_FAILURE);
	}

	/* Annonce au processus parent que tout est OK */
	kill(parent, SIGUSR1);
}

int main(void)
{
	/* Initialise le journal */
	openlog(DAEMON_NAME, LOG_PID, LOG_LOCAL5);
	syslog(LOG_INFO, "starting");

	/* Daemonize */
	daemonize(LOCKFILE);

	/* Processus fils : maintenant un démon */
	for (int ii = 0; ii < 3; ii++) {
		syslog(LOG_INFO, "Daemon loop %d", ii);
		sleep(5);
	}

	/* Cleanup */
	unlink(LOCKFILE);
	syslog(LOG_NOTICE, "terminated");
	closelog();
	return 0;
}
