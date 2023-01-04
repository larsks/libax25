#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <limits.h>

#include <netax25/ttyutils.h>
#include "pathnames.h"

static struct speed_struct {
	int     user_speed;
	speed_t termios_speed;
} speed_table[] = {
	{300,		B300},
	{600,		B600},
	{1200,		B1200},
	{2400,		B2400},
	{4800,		B4800},
	{9600,		B9600},
	{19200,		B19200},
	{38400,		B38400},
#ifdef  B57600
	{57600,		B57600},
#endif
#ifdef  B115200
	{115200,	B115200},
#endif
#ifdef  B230400
	{230400,	B230400},
#endif
#ifdef  B460800
	{460800,	B460800},
#endif
#ifdef  B500000
	{500000,	B500000},
#endif
#ifdef  B576000
	{576000,	B576000},
#endif
#ifdef  B921600
	{921600,	B921600},
#endif
#ifdef  B1000000
	{1000000,	B1000000},
#endif
#ifdef  B1152000
	{1152000,	B1152000},
#endif
#ifdef  B1500000
	{1500000,	B1500000},
#endif
#ifdef  B2000000
	{2000000,	B2000000},
#endif
#ifdef  B2500000
	{2500000,	B2500000},
#endif
#ifdef  B3000000
	{3000000,	B3000000},
#endif
#ifdef  B3500000
	{3500000,	B3500000},
#endif
#ifdef  B4000000
	{4000000,	B4000000},
#endif
	{-1,		B0}
};

int tty_raw(int fd, int hwflag)
{
	struct termios term;

	if (tcgetattr(fd, &term) == -1) {
		perror("tty_raw: tcgetattr");
		return FALSE;
	}

	term.c_cc[VMIN]  = 1;
	term.c_cc[VTIME] = 0;

	term.c_iflag = IGNBRK | IGNPAR;
	term.c_oflag = 0;
	term.c_lflag = 0;

#ifdef CIBAUD
	term.c_cflag = (term.c_cflag & (CBAUD | CIBAUD)) | CREAD | CS8 | CLOCAL;
#else
	term.c_cflag = (term.c_cflag & CBAUD) | CREAD | CS8 | CLOCAL;
#endif

	if (hwflag)
		term.c_cflag |= CRTSCTS;

	if (tcsetattr(fd, TCSANOW, &term) == -1) {
		perror("tty_raw: tcsetattr");
		return FALSE;
	}

	return TRUE;
}

int tty_speed(int fd, int speed)
{
	struct termios term;
	struct speed_struct *s;

	for (s = speed_table; s->user_speed != -1; s++)
		if (s->user_speed == speed)
			break;

	if (s->user_speed == -1) {
		fprintf(stderr, "tty_speed: invalid speed %d\n", speed);
		return FALSE;
	}

	if (tcgetattr(fd, &term) == -1) {
		perror("tty_speed: tcgetattr");
		return FALSE;
	}

	cfsetispeed(&term, s->termios_speed);
	cfsetospeed(&term, s->termios_speed);

	if (tcsetattr(fd, TCSANOW, &term) == -1) {
		perror("tty_speed: tcsetattr");
		return FALSE;
	}

	return TRUE;
}

int tty_is_locked(char *tty)
{
	char buffer[PATH_MAX], *s;
	FILE *fp;
	int pid = 0;

	if ((s = strrchr(tty, '/')) != NULL)
		s++;
	else
		s = tty;

	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "%s/LCK..", LOCK_SERIAL_DIR);
	strncat(buffer+strlen(buffer), s, sizeof(buffer)-strlen(buffer)-1);

	if ((fp = fopen(buffer, "r")) == NULL)
		return FALSE;

	if (fscanf(fp, "%d", &pid) != 1) {
		fclose(fp);
		return FALSE;
	}

	fclose(fp);

	if (kill(pid, 0) == 0)
		return TRUE;

	return FALSE;
}

int tty_lock(char *tty)
{
	char buffer[PATH_MAX], *s;
	FILE *fp;

	if ((s = strrchr(tty, '/')) != NULL)
		s++;
	else
		s = tty;

	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "%s/LCK..", LOCK_SERIAL_DIR);
	strncat(buffer+strlen(buffer), s, sizeof(buffer)-strlen(buffer)-1);

	if ((fp = fopen(buffer, "w")) == NULL)
		return FALSE;

	fprintf(fp, "%10d\n", getpid());

	fclose(fp);

	return TRUE;
}

int tty_unlock(char *tty)
{
	char buffer[PATH_MAX], *s;

	if ((s = strrchr(tty, '/')) != NULL)
		s++;
	else
		s = tty;

	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "%s/LCK..", LOCK_SERIAL_DIR);
	strncat(buffer+strlen(buffer), s, sizeof(buffer)-strlen(buffer)-1);

	return unlink(buffer) == 0;
}
