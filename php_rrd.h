/**
 * PHP bindings to the rrdtool
 *
 * This source file is subject to the BSD license that is bundled
 * with this package in the file LICENSE.
 * ---------------------------------------------------------------
 *  Author: Miroslav Kubelik <koubel@php.net>
 * ---------------------------------------------------------------
 */

#ifndef PHP_RRD_H
#define PHP_RRD_H

extern zend_module_entry rrd_module_entry;
#define phpext_rrd_ptr &rrd_module_entry

#define PHP_RRD_VERSION "1.0.5"

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
