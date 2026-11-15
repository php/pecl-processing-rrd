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

#define PHP_RRD_VERSION "2.0.4"

#ifdef ZTS
#include "TSRM.h"
#endif

#ifndef zend_parse_parameters_none
# define zend_parse_parameters_none() zend_parse_parameters(ZEND_NUM_ARGS(), "")
#endif 

/* librrd took char ** for argv before 1.9 and const char ** from 1.9 on */
#ifdef HAVE_RRD_CONST_ARGV
# define RRD_ARGV(argv) ((const char **)(argv))
#else
# define RRD_ARGV(argv) (argv)
#endif

/* PHP 7.0 remains the minimum version declared by package.xml. */
static zend_always_inline zend_string *rrd_zval_try_get_string(zval *value)
{
#if PHP_VERSION_ID >= 70400
	return zval_try_get_string(value);
#else
	zend_string *result = zval_get_string(value);
	if (EG(exception)) {
		zend_string_release(result);
		return NULL;
	}
	return result;
#endif
}

typedef struct _rrd_args {
	int count;
	char **args;
} rrd_args;

extern rrd_args *rrd_args_init_by_phparray(const char *command_name, const char *filename,
	const zval *options);
extern void rrd_args_free(rrd_args *args);

#endif  /* PHP_RRD_H */
