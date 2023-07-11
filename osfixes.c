//shared between bash and gawk
#include "osfixes.h"
#ifdef _WIN32
#include <config.h>
#ifndef GAWK
#include "wlb_debug.h"
#endif
#define __WIN32__

#ifndef GAWK
#include "term.h"
#include "general.h"
#endif
#define ENV_ALLOC_CNT 1024
#define FAKE_EXIT_STATUS_UNDEFINED 2424242 //exit codes should be 1 byte only
#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif // !FALSE

static volatile long EXEC_FAKE_CUR_PID = FAKE_EXIT_STATUS_UNDEFINED;
#ifndef GAWK



HandleExec* ExecGetNew() {
	HandleExec* exec = malloc(sizeof(HandleExec));
	exec->actions = malloc(sizeof(posix_spawn_file_actions_t));
	exec->attrp = malloc(sizeof(posix_spawnattr_t));
	posix_spawn_file_actions_init(exec->actions);
	posix_spawnattr_init(exec->attrp);
	exec->__has_run = FALSE;
	exec->__has_forked = FALSE;
	exec->child_pid = 0;
	exec->envp = NULL;
	exec->cmd = 0;
	exec->args = 0;
	exec->___env_max = 0;
	exec->___env_cur = 0;
	exec->__had_fake_exit = FALSE;
	exec->fake_exit_status = FAKE_EXIT_STATUS_UNDEFINED;
	return exec;
}
BOOL ExecIsFakePid(pid_t pid) {
	return (pid >= FAKE_EXIT_STATUS_UNDEFINED && pid <= EXEC_FAKE_CUR_PID);
}
BOOL ExecHasFakeExit(HandleExec* exec) {
	return exec->__had_fake_exit;
}
void ExecSetCmdAndArgs(HandleExec* exec, const char* cmd, const char** args) {
	exec->cmd = savestring(cmd);
	if (args) {
		int argCnt;
		for (argCnt = 0; args[argCnt]; argCnt++) {
		}
		exec->args = malloc(sizeof(char*) * argCnt + 1);
		for (argCnt = 0; args[argCnt]; argCnt++) {
			exec->args[argCnt] = savestring(args[argCnt]);
		}
		exec->args[argCnt] = 0;
	}
}
void ExecSetEnvVar(HandleExec* exec, const char** envp) {
	int argCnt;
	if (!envp)
		return;
	for (argCnt = 0; envp[argCnt]; argCnt++)
		ExecDupeEnvVar(exec, envp[argCnt]);
}
void ExecSetFakeExit(HandleExec* exec, int ExitCode) {
	if (exec->__has_run)
		dlogfatal("ExecDoSpawn called but has already been run");

	dlog("Set Fake Exit called for cmd: %s", exec->cmd);
	exec->__has_run = TRUE;
	exec->__had_fake_exit = TRUE;
	exec->fake_exit_status = ExitCode;
	exec->child_pid = InterlockedIncrement(&EXEC_FAKE_CUR_PID);

}
int ExecDoSpawn(HandleExec* exec) {
	if (exec->__has_run)
		dlogfatal("ExecDoSpawn called but has already been run");

	exec->__has_run = TRUE;
	if (ExecHasFakeExit(exec))
		return 0;
	dlog("Running spawn cmd: %s", exec->cmd);
	return posix_spawnp(&exec->child_pid, exec->cmd, exec->actions, exec->attrp, exec->args, exec->envp);
}
void ExecDupeEnvVar(HandleExec* exec, const char* varLine) {
	if (exec->___env_cur == exec->___env_max) {
		dlog("IN DUMPENVVAR");
		int newMax = exec->___env_max + ENV_ALLOC_CNT;
		char** old = exec->envp;
		exec->envp = malloc(sizeof(char*) * (newMax + 1));
		if (old)
			memcpy(exec->envp, old, sizeof(char*) * exec->___env_cur);
		exec->___env_max = newMax;
	}
	exec->envp[exec->___env_cur++] = savestring(varLine);
	exec->envp[exec->___env_cur] = 0;
}

static void __DupeOrUpdateEnvVarPair(HandleExec* exec, BOOL CHECK_EXISTING, const char* varName, const char* value) {
	int sz = 1 + strlen(varName) + strlen(value) + 1;
	char* tmp = strcpy(malloc(sz), varName);
	strcat_s(tmp, sz, "=");

	if (CHECK_EXISTING) {
		char* findStr = tmp;
		for (int x = 0; x < exec->___env_cur; x++) {
			if (strcmp(exec->envp[x], findStr) == 0) {
				strcat_s(tmp, sz, value);
				free(exec->envp[x]);
				exec->envp[x] = tmp;
				return;
			}
		}

	}
	strcat_s(tmp, sz, value);
	ExecDupeEnvVar(exec, tmp);
	free(tmp);
}
void ExecDupeEnvVarPair(HandleExec* exec, const char* varName, const char* value) {
	__DupeOrUpdateEnvVarPair(exec, FALSE, varName, value);
}
void ExecDupeOrUpdateEnvVarPair(HandleExec* exec, const char* varName, const char* value) {
	__DupeOrUpdateEnvVarPair(exec, TRUE, varName, value);
}
//the original code would have already forked by now, aka this should be spanwed before the destroy
void ExecWouldHaveForked(HandleExec* exec) {
	exec->__has_forked = true;
}
void ExecDestroy(HandleExec** exec_ptr) {
	HandleExec* exec = *exec_ptr;
	if (exec->__has_forked && ! exec->__has_run) {
		dlogfatal("originally we would have forked by now but we have not yet run so shouldn't be destroying");
	}
	posix_spawnattr_destroy(exec->attrp);
	posix_spawn_file_actions_destroy(exec->actions);
	for (int x = 0; x < exec->___env_cur; x++)
		free(exec->envp[x]);
	if (exec->envp)
		free(exec->envp);
	if (exec->cmd)
		free(exec->cmd);
	if (exec->args) {
		for (int argCnt = 0; exec->args[argCnt]; argCnt++)
			free(exec->args[argCnt]);
		free(exec->args);
	}

	free(exec);
	exec_ptr = -1;
}
#endif // !GAWK]6
int kill(pid_t pid, int sig) {
	if (sig == SIGINT)
		return GenerateConsoleCtrlEvent(CTRL_C_EVENT, pid);//as long as we have created the process with CREATE_NEW_PROCESS_GROUP  it should
	else {
		printf("kill for pid: %i and sig: %i", pid, sig);
		return -1;
	}
}

int fchmod(int fd, mode_t mode) {
	return 0;
}
unsigned int alarm(unsigned int seconds) {
	return -1;
}
pid_t getpgrp(pid_t pid) {
	return -1;
}
pid_t getppid() {
	return -1;
}
pid_t wait(int* wstatus) {
	return -1;
}



#endif
