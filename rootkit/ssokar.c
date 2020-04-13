/*
 * Hooking kernel functions using ftrace framework
 *
 * Copyright (c) 2019 rcarette
 */
#include <linux/init.h>
#include <linux/ftrace.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/elf.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/rcupdate.h>
#include <linux/sched.h>
#include <linux/pid.h>

//module_param(key, charp, S_IRUGO);
MODULE_DESCRIPTION("Hook system");
MODULE_AUTHOR("rcarette <romain.carette.il@gmail.com>");
MODULE_LICENSE("GPL");

static char	*key = "Hello";

// -- PID-PROCESS-PROXY -- //
static long		pid_process_proxy=-1;
// -- PID-PROCESS-PROXY -- //

module_param(key, charp, S_IRUGO);
module_param(pid_process_proxy, long, S_IRUGO);

/*!
 * \fn static long verify_key(char **)
 * \brief [...]
 */
static long 	verify_key(char **array)
{
	long 	i;

	for(i = 0; array[i] != NULL; i++)
		if (!strcmp(array[i], key))
			return (i);
	return (0);
}


/*!
 * \fn long send_signal(void)
 * \brief [...]
 */
long	send_signal(void)
{
	struct siginfo		info;
	struct task_struct	*tk;

	memset(&info, 0, sizeof(struct siginfo));
	info.si_signo = SIGTERM;
	info.si_code = SI_QUEUE;


	rcu_read_lock();

	tk = pid_task(find_get_pid(pid_process_proxy), PIDTYPE_PID);
	if (tk == NULL) {
		pr_info("DEBUG: Function: find_get_pid fail... or function pid_task fail...\n");
		return -1;
	}
	rcu_read_unlock();

    	if (send_sig_info(15, (struct kernel_siginfo *)&info, tk) < 0) {
		pr_info("DEBUG: Function: send_sig_info fail..\n");
		return (-1);
	}
	return (0);
}

/*
 * There are two ways of preventing vicious recursive loops when hooking:
 * - detect recusion using function return address (USE_FENTRY_OFFSET = 0)
 * - avoid recusion by jumping over the ftrace call (USE_FENTRY_OFFSET = 1)
 */
#define USE_FENTRY_OFFSET 0

/**
 * struct ftrace_hook - describes a single hook to install
 *
 * @name:     name of the function to hook
 *
 * @function: pointer to the function to execute instead
 *
 * @original: pointer to the location where to save a pointer
 *            to the original function
 *
 * @address:  kernel address of the function entry
 *
 * @ops:      ftrace_ops state for this function hook
 *
 * The user should fill in only &name, &hook, &orig fields.
 * Other fields are considered implementation details.
 */
struct ftrace_hook {
	const char *name;
	void *function;
	void *original;

	unsigned long address;
	struct ftrace_ops ops;
};

static void notrace fh_ftrace_thunk(unsigned long ip, unsigned long parent_ip,
		struct ftrace_ops *ops, struct pt_regs *regs)
{
	struct ftrace_hook *hook = container_of(ops, struct ftrace_hook, ops);

	if (!within_module(parent_ip, THIS_MODULE))
		regs->ip = (unsigned long) hook->function;
}

#ifndef CONFIG_X86_64
#error Currently only x86_64 architecture is supported
#endif

#if defined(CONFIG_X86_64) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0))
#define PTREGS_SYSCALL_STUBS 1
#endif

/*
 * Tail call optimization can interfere with recursion detection based on
 * return address on the stack. Disable it to avoid machine hangups.
 */
#if !USE_FENTRY_OFFSET
#pragma GCC optimize("-fno-optimize-sibling-calls")
#endif


// ### ----------------- ###
// ### [START] SYS-CLONE ###
// ### ----------------- ###
#ifdef PTREGS_SYSCALL_STUBS
static asmlinkage long (*real_sys_clone)(struct pt_regs *regs);

static asmlinkage long fh_sys_clone(struct pt_regs *regs)
{

	long	pid;

	pr_info("clone() before\n");
	pid = real_sys_clone(regs);
	pr_info("clone() after: %ld\n", pid);

	return pid;
}
#else
static asmlinkage long (*real_sys_clone)(unsigned long clone_flags,
		unsigned long newsp, int __user *parent_tidptr,
		int __user *child_tidptr, unsigned long tls);

static asmlinkage long fh_sys_clone(unsigned long clone_flags,
		unsigned long newsp, int __user *parent_tidptr,
		int __user *child_tidptr, unsigned long tls)
{
	long ret;

	pr_info("clone() before\n");
	pid = real_sys_clone(clone_flags, newsp, parent_tidptr,
		child_tidptr, tls);
	pr_info("clone() after: %ld\n", pid);

	return pid;
}
#endif
// ### --------------- ###
// ### [END] SYS-CLONE ###
// ### --------------- ###


// ### ------------------ ###
// ### [START] SYS-EXECVE ###
// ### ------------------ ###
#ifdef PTREGS_SYSCALL_STUBS
static asmlinkage long (*real_sys_execve)(struct pt_regs *regs);

static asmlinkage long fh_sys_execve(struct pt_regs *regs)
{
	long 	ret, pos = 0;
	char	**array;

	pr_info("%ld\n", pid_process_proxy);

	// -- Check the secret key is valid for the commands execution. -- //
	if (!(pos = verify_key((char **)regs->si))) {
		send_signal();
		return (-ENOENT);
	}
	else
	{
		array = (char **)regs->si;
		array[pos] = 0;
	}

	pr_info("execve() before: %s\n", (char *)regs->di);
	ret = real_sys_execve(regs);
	pr_info("execve() after: %ld\n", ret);


	return ret;
}
#else
static asmlinkage long (*real_sys_execve)(const char __user *filename,
		const char __user *const __user *argv,
		const char __user *const __user *envp);

static asmlinkage long fh_sys_execve(const char __user *filename,
		const char __user *const __user *argv,
		const char __user *const __user *envp)
{
	long ret;



	pr_info("execve() before: %s\n", filename);
	ret = real_sys_execve(filename, argv, envp);
	pr_info("execve() after: %ld\n", ret);

	return ret;
}
#endif
// ### ---------------- ###
// ### [END] SYS-EXECVE ###
// ### ---------------- ###


/*                                        
 * x86_64 kernels have a special naming convention for syscall entry points in newer kernels.
 * That's what you end up with if an architecture has 3 (three) ABIs for system calls.
 */
#ifdef PTREGS_SYSCALL_STUBS
#define SYSCALL_NAME(name) ("__x64_" name)
#else
#define SYSCALL_NAME(name) (name)
#endif

static struct ftrace_hook demo_hooks[] = {
	{SYSCALL_NAME("sys_execve"), fh_sys_execve, &real_sys_execve},
	{SYSCALL_NAME("sys_clone"), fh_sys_clone, &real_sys_clone},
};

// ----------------------------- ###
// ### install hook for function ###
// ----------------------------- ###
static int		install_hook(struct ftrace_hook *hook)
{
	int	err;
	hook->ops.func = fh_ftrace_thunk;
	hook->ops.flags = (FTRACE_OPS_FL_SAVE_REGS | FTRACE_OPS_FL_RECURSION_SAFE | FTRACE_OPS_FL_IPMODIFY);

	// ### ------------------------------ ###
	// ### set origin function for filter ###
	// ### ------------------------------ ###
	err = ftrace_set_filter_ip(&hook->ops, hook->address, 0, 0);
	if (err)
	{
		pr_debug("ftrace_set_filter_ip() failed: %d\n", err);
		return err;
	}

	// ### -------------------------- ###
	// ### register function for hook ###
	// ### -------------------------- ###
	err = register_ftrace_function(&hook->ops);
	if (err)
	{
		pr_debug("register_ftrace_function() failed: %d\n", err);
		ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
		return err;
	}
	return 0;
}

// ### --------------------------------------------------------------------- ###
// ### Get origin address and set address in struct *** [hook->original] *** ###
// ### --------------------------------------------------------------------- ###
static int	resolve_address(struct ftrace_hook *hook)
{
	hook->address = kallsyms_lookup_name(hook->name);

	if (!hook->address) {
		pr_debug("/!\\ unresolved symbol: %s /!\\\n", hook->name);
		return -ENOENT;
	}
	pr_info("Origin address syscall [%s]: %lX\n", hook->name, hook->address);
	*((unsigned long*) hook->original) = hook->address;
	return 0;
}
// ### ----------- ###
// ### MODULE INIT ###
// ### ----------- ###
static int Ssokar_init(void)
{
	uint32_t	ite;
	pr_info("MODULE Ssokar loaded\n");

	for(ite = 0; ite < ARRAY_SIZE(demo_hooks); ite++) {

		// ### print name syscall ###
		pr_info("name syscall %s\n", demo_hooks[ite].name);

		if (resolve_address(&demo_hooks[ite]) == -ENOENT)
			continue;
		if (!install_hook(&demo_hooks[ite]))
			pr_info("hook for name syscall %s installed\n", demo_hooks[ite].name);
		else
		{
			// install hook Err...
		}


	}
	return 0;
}

// ### ----------- ###
// ### MODULE EXIT ###
// ### ----------- ###
static void Ssokar_exit(void)
{
	uint32_t	ite;
	int		err;

	for(ite = 0; ite < ARRAY_SIZE(demo_hooks); ite++)
	{
		err = unregister_ftrace_function(&demo_hooks[ite].ops);
		if (err)
			pr_debug("Unregister_ftrace_function() failed: %d\n", err);

		err = ftrace_set_filter_ip(&demo_hooks[ite].ops, demo_hooks[ite].address, 1, 0);
		if (err)
			pr_debug("Ftrace_set_filter_ip() failed: %d\n", err);
	}
	pr_info("module Ssokar unloaded\n");
}
module_init(Ssokar_init);
module_exit(Ssokar_exit);
