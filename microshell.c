#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int tmp;

int err(char *s)
{
	while (*s)
		write(2, s++, 1);
	return 1;
}

int cd(char **av, int i)
{
	if (i != 2)
		return err("error: cd: bad arguments\n");
	if (chdir(av[1]))
		return err("error: cd: cannot change directory to ") & err(av[1]) & err("\n");
	return 0;
}

int exe(char **av, char **ep, int i)
{
	int tm2[2], r;
	int pip = (av[i] && !strcmp(av[i], "|"));
	if (pip && (pipe(tm2)))
		return err("error: fatal\n");
	int pid = fork();
	if (!pid)
	{
		av[i] = 0;
		if (dup2(tmp, 0) == -1 | close(tmp) == -1 | (pip && (dup2(tm2[1], 1)
			== -1 | close(tm2[0]) == -1 | close(tm2[1]) == -1)))
			return err("error: fatal\n");
		execve(*av, av, ep);
		return err("error: cannot execute ") & err(*av) & err("\n");
	}
	if ((pip && (dup2(tm2[0], tmp) == -1 | close(tm2[0]) == -1
		| close(tm2[1]) == -1)) | (!pip && dup2(0, tmp) == -1) |
		waitpid(pid, &r, 0) == -1)
		return err("error: fatal\n");
	return WIFEXITED(r) && WEXITSTATUS(r);
}

int main(int ac, char **av, char **ep)
{
	(void)ac;
	int i = 0, r = 0;
	tmp = dup(0);
	while (av[i] && av[++i])
	{
		av = av + i;
		i = 0;
		while (av[i] && strcmp(av[i], "|") && strcmp(av[i], ";"))
			i++;
		if (!strcmp(*av, "cd"))
			r = cd(av, i);
		else if (i)
			r = exe(av, ep, i);
	}
	return ((dup2(0, tmp) == -1) && err("error: fatal\n")) | r;
}

// leaks -atExit -- ./microshell /bin/ls "|" /usr/bin/grep microshell ";" /bin/echo i love my microshell 