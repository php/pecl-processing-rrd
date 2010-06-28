/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2010 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt.                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Miroslav Kubelik <koubel@volny.cz>                           |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "ext/standard/php_smart_str.h"
#include "php_rrd.h"
#include "rrd_graph.h"
#include "rrd_create.h"
#include "rrd_update.h"

/* {{{ */
static function_entry rrd_functions[] = {
	PHP_FE(rrd_update, arginfo_rrd_update)
	PHP_FE(rrd_create, arginfo_rrd_create)
	PHP_FE(rrd_graph, arginfo_rrd_graph)
	{NULL, NULL, NULL}
};
/* }}} */

#ifdef COMPILE_DL_RRD
	ZEND_GET_MODULE(rrd)
#endif

/* {{{ PHP_MINIT_FUNCTION */
static PHP_MINIT_FUNCTION(rrd)
{
	rrd_graph_minit(TSRMLS_CC);
	rrd_create_minit(TSRMLS_CC);
	rrd_update_minit(TSRMLS_CC);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
static PHP_MINFO_FUNCTION(rrd)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "rrd wrapper", "enabled");
	php_info_print_table_row(2, "Version", "0.1");
	php_info_print_table_end();
}
/* }}} */

/* {{{ rrd module_entry */
zend_module_entry rrd_module_entry = {
	STANDARD_MODULE_HEADER,
	"rrd",
	rrd_functions,
	PHP_MINIT(rrd),
	NULL, /* PHP_MSHUTDOWN(rrd) */
	NULL, /* PHP_RINIT(rrd) */
	NULL, /* PHP_RSHUTDOWN(rrd) */
	PHP_MINFO(rrd),
	"0.1",
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

/* {{{ Inits rrd arguments object for a particular rrd command by php array
 * filename paremeter is optional.
 */
rrd_args *rrd_args_init_by_phparray(const char *command_name, const char *filename,
	const zval *options TSRMLS_DC)
{
	uint i, option_count, args_counter = 2;

	if (Z_TYPE_P(options) != IS_ARRAY) return NULL;
	option_count = zend_hash_num_elements(Z_ARRVAL_P(options));
	if (!option_count) return NULL;
	if (!strlen(command_name)) return NULL;

	rrd_args *result = (rrd_args *)emalloc(sizeof(rrd_args));
	/* "dummy" + command_name + filename if presented */
	result->count = option_count + (strlen(command_name) ? 3 : 2);
	result->args = (char **)safe_emalloc(result->count, sizeof(char *), 0);

	/* "dummy" and command_name are needed always needed */
	result->args[0] = "dummy";
	result->args[1] = estrdup(filename);

	/* append filename if it's presented */
	if (strlen(filename)) result->args[args_counter++] = estrdup(filename);

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(options));
	for(i=0; i<option_count; i++) {
		zval **item;
		smart_str option = {0}; /* one argument option */

		/* force using strings as array items */
		zend_hash_get_current_data(Z_ARRVAL_P(options), (void**) &item);
		if (Z_TYPE_PP(item) != IS_STRING) convert_to_string(*item);
		smart_str_appendl(&option, Z_STRVAL_PP(item), Z_STRLEN_PP(item));
		smart_str_0(&option);

		result->args[args_counter++] = estrdup(option.c);
		smart_str_free(&option);

		zend_hash_move_forward(Z_ARRVAL_P(options));
	}

	return result;
}
/* }}} */

/* {{{ Frees all memory for arguments object
 */
void rrd_args_free(rrd_args *args)
{
	int i;
	if (!args || !args->args) return;

	for (i=1; i<args->count; i++)
		efree(args->args[i]);

	efree(args->args);
	efree(args);
}
/* }}} */