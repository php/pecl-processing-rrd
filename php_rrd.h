/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2011 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt.                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Miroslav Kubelik <koubel@php.net>                            |
  +----------------------------------------------------------------------+
*/

#ifndef PHP_RRD_H
#define PHP_RRD_H

extern zend_module_entry rrd_module_entry;
#define phpext_rrd_ptr &rrd_module_entry

#define PHP_RRD_VERSION "1.0.3"

#ifdef ZTS
#include "TSRM.h"
#endif

typedef struct _rrd_args {
	int count;
	char **args;
} rrd_args;

extern rrd_args *rrd_args_init_by_phparray(const char *command_name, const char *filename,
	const zval *options TSRMLS_DC);
extern void rrd_args_free(rrd_args *args);

#endif  /* PHP_RRD_H */
