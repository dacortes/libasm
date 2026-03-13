/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tests.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: codespace <codespace@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/05 11:50:33 by dacortes          #+#    #+#             */
/*   Updated: 2026/03/13 12:55:38 by codespace        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>

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

size_t	ft_strlen(const char *);
char    *ft_strcpy(char *, const char *);


/*  General tools   */
void	segfault_handler(int sig, siginfo_t *info, void *context);
bool	check_null(size_t (*func)(const char *), const char *param);

/*	Test            */
void	inject_data_strlen(void);
void    inject_data_strcpy(void);