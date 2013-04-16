/*
 * yaepghd.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * Community Edition
 */

#pragma once


// To avoid problems with the old MainMenuHooks-v1.0 patch uncomment next line.
// #undef MAINMENUHOOKSVERSNUM


/* Logging functions */
void yaepg_error(const char *func, const char *fmt, ...);

void yaepg_info(const char *func, const char *fmt, ...);



/**
 * Macros
 */
//#define DEBUG
#ifdef DEBUG
#define ASSERT                   assert
#define YAEPG_ERROR(...)         yaepg_error(__PRETTY_FUNCTION__, __VA_ARGS__)
#define YAEPG_INFO(...)          yaepg_info(__PRETTY_FUNCTION__, __VA_ARGS__)
#else /* !DEBUG */
#define ASSERT(_a)
#define YAEPG_ERROR(...)
#define YAEPG_INFO(...)
#endif /* DEBUG */

#ifndef MIN
#define MIN(_a, _b)              ((_a) < (_b) ? (_a) : (_b))
#endif
#ifndef MAX
#define MAX(_a, _b)              ((_a) < (_b) ? (_b) : (_a))
#endif
#define ROUND(_f)                (int)((_f) + 0.5f)

