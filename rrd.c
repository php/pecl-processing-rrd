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
  | Author: Miroslav Kubelik <koubel@php.net>                           |
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

#include <rrd.h>

/* {{{ proto string rrd_error()
Get the error message set by the last rrd tool function call, this function
clear error buffer also.
*/
PHP_FUNCTION(rrd_error)
{
	if (!rrd_test_error()) RETURN_FALSE;

	RETVAL_STRING(rrd_get_error(), 1);
	rrd_clear_error();
}
/* }}} */

/* {{{ proto array rrd_fetch(string file, array options)
Fetch data from RRD in requested resolution.
*/
PHP_FUNCTION(rrd_fetch)
{
	char *filename;
	int filename_length;
	zval *zv_arr_options;
	rrd_args *argv;
	/* returned values if rrd_fetch doesn't fail */
	time_t start, end;
	ulong step,
	ds_cnt; /* count of data sources */
	char **ds_namv; /* list of data source names */
	rrd_value_t *ds_data; /* all data from all sources */

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa", &filename,
		&filename_length, &zv_arr_options) == FAILURE) {
		return;
	}

	if (php_check_open_basedir(filename TSRMLS_CC)) RETURN_FALSE;

	argv = rrd_args_init_by_phparray("fetch", filename, zv_arr_options TSRMLS_CC);
	if (!argv) {
		zend_error(E_WARNING, "cannot allocate arguments options");
		RETURN_FALSE;
	}

	if (rrd_test_error()) rrd_clear_error();

	/* call rrd_fetch and test if fails */
	if (rrd_fetch(argv->count-1, &argv->args[1], &start, &end, &step, &ds_cnt,
		&ds_namv, &ds_data) == -1 ) {

		rrd_args_free(argv);
		RETURN_FALSE;
	}

	/* making return array*/
	array_init(return_value);
	add_assoc_long(return_value, "start", start);
	add_assoc_long(return_value, "end", end);
	add_assoc_long(return_value, "step", step);
	add_assoc_long(return_value, "ds_cnt", ds_cnt);

	/* add "ds_namv" return values to return array under "ds_namv" key
	 * if no data sources are presented add PHP NULL value
	 */
	if (!ds_namv || !ds_cnt) {
		add_assoc_null(return_value, "ds_namv");
	} else {
		/* count of intems in ds_namv array, total count is stored in
		 * ds_cnt value
		 */
		uint i;
		/* "ds_namv" is presented, hence create array for it, and add it to
		 * return array
		 */
		zval *zv_ds_namv_array;
		MAKE_STD_ZVAL(zv_ds_namv_array)
		array_init(zv_ds_namv_array);
		for	(i=0; i < ds_cnt; i++) {
			add_next_index_string(zv_ds_namv_array, ds_namv[i], 1);
			free(ds_namv[i]);
		}
		free(ds_namv);
		add_assoc_zval(return_value, "ds_navm", zv_ds_namv_array);
	}

	/* Add all data from all sources as array under "data" key in return array.
	 * If no data are presented add PHP NULL value
	 */
	if (!ds_data) {
		add_assoc_null(return_value, "data");
	} else {
		rrd_value_t *datap = ds_data;
		uint timestamp, ds_counter;
		zval *zv_data_array;

		MAKE_STD_ZVAL(zv_data_array)
		array_init(zv_data_array);
		for (timestamp = start + step; timestamp <= end; timestamp += step) {
			for (ds_counter = 0; ds_counter < ds_cnt; ds_counter++) {
				/* value for key (timestamp) in data array */
				zval *zv_timestamp;
				MAKE_STD_ZVAL(zv_timestamp);
				ZVAL_LONG(zv_timestamp, timestamp);
				convert_to_string(zv_timestamp);

				add_assoc_double(zv_data_array, Z_STRVAL_P(zv_timestamp), *(datap++));
				zval_dtor(zv_timestamp);
			}
		}
		free(ds_data);

		add_assoc_zval(return_value, "data", zv_data_array);
	}

	rrd_args_free(argv);
}
/* }}} */

/* {{{ proto int rrd_first(string file [, int rraindex = 0])
	Gets first update time of an RRD file
*/
PHP_FUNCTION(rrd_first)
{
	char *filename;
	int filename_length;
	long rraindex = 0;
	/* return value from rrd_first_r call */
	time_t rrd_first_return_val;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &filename,
		&filename_length, &rraindex) == FAILURE) {
		return;
	}

	if (rraindex < 0) {
		rrd_set_error("invalid rraindex number, rraindex must be >= 0");
		RETURN_FALSE;
	}

	if (php_check_open_basedir(filename TSRMLS_CC)) RETURN_FALSE;

	if (rrd_test_error()) rrd_clear_error();

	/* call rrd_first and test if fails */
	rrd_first_return_val = rrd_first_r(filename, rraindex);
	if (rrd_first_return_val == -1) {
		RETURN_FALSE;
	}
	RETURN_LONG(rrd_first_return_val);
}
/* }}} */

/* {{{ proto array rrd_info(string file)
	Gets the header information from an RRD.
*/
PHP_FUNCTION(rrd_info)
{
	char *filename;
	int filename_length;
	/* list of arguments for rrd_info call, it's more efficient then u
	 * usage of rrd_args, because there isn't array of arguments in parameters
	 */
	char *argv[3];
	/* return value from rrd_info_r() */
	rrd_info_t *rrd_info_data, *data_p;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &filename,
		&filename_length) == FAILURE) {
		return;
	}

	if (php_check_open_basedir(filename TSRMLS_CC)) RETURN_FALSE;

	argv[0] = "dummy";
	argv[1] = estrdup("info");
	argv[2] = estrndup(filename, filename_length);

	rrd_info_data = rrd_info(2, &argv[1]);

	efree(argv[2]); efree(argv[1]);

	if (!rrd_info_data) RETURN_FALSE;

	/* making return array*/
	array_init(return_value);

	data_p = rrd_info_data;
	while (data_p) {
		switch (data_p->type) {
		case RD_I_VAL:
			add_assoc_double(return_value, data_p->key, data_p->value.u_val);
			break;
		case RD_I_CNT:
			add_assoc_long(return_value, data_p->key, data_p->value.u_cnt);
			break;
		case RD_I_INT:
			add_assoc_long(return_value, data_p->key, data_p->value.u_int);
			break;
		case RD_I_STR:
			add_assoc_string(return_value, data_p->key, data_p->value.u_str, 1);
			break;
		case RD_I_BLO:
			add_assoc_stringl(return_value, data_p->key, data_p->value.u_blo.ptr,
				 data_p->value.u_blo.size, 1);
			break;
		}
		data_p = data_p->next;
	}

	rrd_info_free(rrd_info_data);
}
/* }}} */

/* {{{ proto int rrd_last(string file)
	Gets last update time of an RRD file
*/
PHP_FUNCTION(rrd_last)
{
	char *filename;
	int filename_length;
	/* return value from rrd_first_r call */
	time_t rrd_last_return_val;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &filename,
		&filename_length) == FAILURE) {
		return;
	}

	if (rrd_test_error()) rrd_clear_error();

	/* call rrd_last and test if fails */
	rrd_last_return_val = rrd_last_r(filename);
	if (rrd_last_return_val == -1) {
		RETURN_FALSE;
	}
	RETURN_LONG(rrd_last_return_val);
}
/* }}} */

/* {{{ arguments */
ZEND_BEGIN_ARG_INFO(arginfo_rrd_fetch, 0)
	ZEND_ARG_INFO(0, file)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_rrd_first, 0, 0, 1)
	ZEND_ARG_INFO(0, file)
	ZEND_ARG_INFO(0, raaindex)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_rrd_info, 0)
	ZEND_ARG_INFO(0, file)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_rrd_last, 0)
	ZEND_ARG_INFO(0, file)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ */
static function_entry rrd_functions[] = {
	PHP_FE(rrd_update, arginfo_rrd_update)
	PHP_FE(rrd_create, arginfo_rrd_create)
	PHP_FE(rrd_graph, arginfo_rrd_graph)
	PHP_FE(rrd_error, NULL)
	PHP_FE(rrd_fetch, arginfo_rrd_fetch)
	PHP_FE(rrd_first, arginfo_rrd_first)
	PHP_FE(rrd_info, arginfo_rrd_info)
	PHP_FE(rrd_last, arginfo_rrd_last)
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
	for (i=0; i<option_count; i++) {
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