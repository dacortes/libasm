/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tests.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: codespace <codespace@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/05 11:50:33 by dacortes          #+#    #+#             */
/*   Updated: 2026/03/23 15:12:45 by codespace        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file tests.h
 * @brief Shared declarations, color macros, and test entry points for the libasm test suite.
 *
 * This header exposes the assembly functions under test together with the helper
 * callbacks and suite runners used by the C-based validation executable.
 */

#pragma once

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>

/******************************************************************************/
/*                            COLORS                                          */
/******************************************************************************/

#define END	"\033[m"
#define RED	"\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW	"\033[1;33m"
#define BLUE	"\033[1;34m"
#define TUR	"\033[1;35m"
#define CYAN	"\033[1;36m"
#define ORANGE	"\033[38;5;208m"
#define PURPLE	"\033[38;5;128m"

#define ERROR -1

/* error messages	*/
#define STR_ERR RED"[ERROR]"END
#define STR_SCC GREEN"[SUCCESS]"END
#define STR_WAR YELLOW"[WARNING]"END
#define STR_INF BLUE"[INFO]"END

#define ERR_SIG STR_ERR" sigaction"

extern sigjmp_buf env;

/** Returns the number of bytes in a null-terminated string. */
size_t	ft_strlen(const char *);
/** Copies a null-terminated string into the destination buffer and returns dest. */
char	*ft_strcpy(char *, const char *);
/** Compares two strings and returns the signed byte difference at the first mismatch. */
int		ft_strcmp(const char *, const char *);
/** Writes up to count bytes to the given file descriptor. */
ssize_t	ft_write(int, const void *, size_t);
/** Reads up to count bytes from the given file descriptor into the provided buffer. */
ssize_t ft_read(int, void *, size_t);
/** Allocates and returns a heap-allocated duplicate of the input string. */
char	*ft_strdup(const char *);

/*  General tools   */
/** Signal handler that turns a segmentation fault into a recoverable test event. */
void	handle_sigsegv(int sig, siginfo_t *info, void *context);
/** Signal handler that turns an abort into a recoverable test event. */
void	handle_sigabrt(int sig, siginfo_t *info, void *context);

/*	Test			*/
/** Runs every strlen validation scenario. */
void	inject_data_strlen(void);
/** Runs every strcpy validation scenario. */
void	inject_data_strcpy(void);
/** Runs every strcmp validation scenario. */
void	inject_data_strcmp(void);
/** Runs every write validation scenario. */
void	inject_data_write(void);
/** Runs every read validation scenario. */
void	inject_data_read(void);
/** Runs every strdup validation scenario. */
void	inject_data_strdup(void);