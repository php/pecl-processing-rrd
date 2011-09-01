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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "ext/standard/php_smart_str.h"
#include "ext/standard/php_array.h"
#include "ext/standard/info.h"

#include <rrd.h>

#include "php_rrd.h"
#include "rrd_graph.h"
#include "rrd_create.h"
#include "rrd_update.h"
#include "rrd_info.h"

/* {{{ proto string rrd_error()
Get the error message set by the last rrd tool function call, this function
clear error buffer also.
*/
PHP_FUNCTION(rrd_error)
{
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		return;
	}

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

	/* add "ds_namv" and "data" array into return values if there is any
	 * result data
	 */
	if (!ds_data || !ds_namv || !ds_cnt) {
		add_assoc_null(return_value, "data");
	} else {
		rrd_value_t *datap = ds_data;
		uint timestamp, ds_counter;
		/* final array for all data from all data sources */
		zval *zv_data_array;

		MAKE_STD_ZVAL(zv_data_array)
		array_init(zv_data_array);

		/* add arrays for each data source, each array will be filled with
		 * retrieved data for a particular data source
		 */
		for (ds_counter = 0; ds_counter < ds_cnt; ds_counter++) {
			zval *zv_ds_data_array;
			MAKE_STD_ZVAL(zv_ds_data_array)
			array_init(zv_ds_data_array);

			add_assoc_zval(zv_data_array, ds_namv[ds_counter], zv_ds_data_array);
		}

		for (timestamp = start + step; timestamp <= end; timestamp += step) {
			/* try to find current data source result array in the
			 * zv_data_array
			 */
			zend_hash_internal_pointer_reset(Z_ARRVAL_P(zv_data_array));
			for (ds_counter = 0; ds_counter < ds_cnt; ds_counter++) {
				/* pointer for one data source retrieved data */
				zval **ds_data_array;
				/* value for key (timestamp) in data array */
				zval *zv_timestamp;

				MAKE_STD_ZVAL(zv_timestamp);
				ZVAL_LONG(zv_timestamp, timestamp);
				convert_to_string(zv_timestamp);

				/* gets pointer for data source result array */
				zend_hash_get_current_data(Z_ARRVAL_P(zv_data_array), (void**) &ds_data_array);

				add_assoc_double(*ds_data_array, Z_STRVAL_P(zv_timestamp), *(datap++));

				zend_hash_move_forward(Z_ARRVAL_P(zv_data_array));

				zval_dtor(zv_timestamp);
			}
		}
		add_assoc_zval(return_value, "data", zv_data_array);

		/* free data from rrd_fetch */
		free(ds_data);
		for (ds_counter = 0; ds_counter < ds_cnt; ds_counter++) {
			free(ds_namv[ds_counter]);
		}
		free(ds_namv);
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

	if (php_check_open_basedir(filename TSRMLS_CC)) RETURN_FALSE;

	if (rrd_test_error()) rrd_clear_error();

	/* call rrd_last and test if fails */
	rrd_last_return_val = rrd_last_r(filename);
	if (rrd_last_return_val == -1) {
		RETURN_FALSE;
	}
	RETURN_LONG(rrd_last_return_val);
}
/* }}} */

/* {{{ proto int rrd_lastupdate(string file)
	Gets last update details of an RRD file */
PHP_FUNCTION(rrd_lastupdate)
{
	char *filename;
	int filename_length;
	/* list of arguments for rrd_lastupdate call, it's more efficient then u
	 * usage of rrd_args, because there isn't array of arguments in parameters
	 */
	char *argv[3];
	/* return values from rrd_lastupdate_r function */
	time_t last_update;
	unsigned long ds_cnt;
	char **ds_namv;
	char **last_ds;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &filename,
		&filename_length) == FAILURE) {
		return;
	}

	if (php_check_open_basedir(filename TSRMLS_CC)) RETURN_FALSE;

	argv[0] = "dummy";
	argv[1] = estrdup("lastupdate");
	argv[2] = estrndup(filename, filename_length);

	if (rrd_test_error()) rrd_clear_error();

#ifdef HAVE_RRD_LASTUPDATE_R
	if (rrd_lastupdate_r(argv[2], &last_update, &ds_cnt, &ds_namv,
		&last_ds) == -1) {
#else
 	if (rrd_lastupdate(2, &argv[1], &last_update, &ds_cnt, &ds_namv,
 		&last_ds) == -1) {
#endif
		efree(argv[2]); efree(argv[1]);
		RETURN_FALSE;
	}

	efree(argv[2]); efree(argv[1]);

	/* making return array*/
	array_init(return_value);
	add_assoc_long(return_value, "last_update", last_update);
	add_assoc_long(return_value, "ds_cnt", ds_cnt);

	/* "ds_navm" return array or null, if no available */
	if (!ds_namv || !ds_cnt) {
		add_assoc_null(return_value, "ds_namv");
	} else {
		uint i;
		zval *zv_ds_namv_array;
		MAKE_STD_ZVAL(zv_ds_namv_array);
		array_init(zv_ds_namv_array);

		for (i = 0; i < ds_cnt; i++) {
			add_next_index_string(zv_ds_namv_array, ds_namv[i], 1);
			free(ds_namv[i]);
		}
		free(ds_namv);
		add_assoc_zval(return_value, "ds_navm", zv_ds_namv_array);
	}

	/* "data" return array or null, if no available */
	if (!last_ds || !ds_cnt) {
		add_assoc_null(return_value, "data");
	} else {
		uint i;
		zval *zv_data_array;
		MAKE_STD_ZVAL(zv_data_array);
		array_init(zv_data_array);

		for (i = 0; i < ds_cnt; i++) {
			/* last_update is key in data array */
			zval *zv_timestamp;
			MAKE_STD_ZVAL(zv_timestamp);
			ZVAL_LONG(zv_timestamp, last_update);
			convert_to_string(zv_timestamp);

			add_assoc_string(zv_data_array, Z_STRVAL_P(zv_timestamp), last_ds[i], 1);

			free(last_ds[i]);
			zval_dtor(zv_timestamp);
		}
		free(last_ds);
		add_assoc_zval(return_value, "data", zv_data_array);
	}
}

/* {{{ proto array rrd_restore(string xmlFile, string rrdFile [, array options])
	Restores an RRD file from a XML dump */
PHP_FUNCTION(rrd_restore)
{
	char *xml_filename, *rrd_filename;
	int xml_filename_length, rrd_filename_length;
	zval *zv_arr_options = NULL;
	/* this is merge of options and rrd_filename. This is needed because
	 * rrd_args_init_by_phparray allows only one filename as argument, so
	 * rrd_filename mugst be part of array of arguments
	 */
	zval *zv_options;
	rrd_args *argv;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|a", &xml_filename,
		&xml_filename_length, &rrd_filename, &rrd_filename_length,
		&zv_arr_options) == FAILURE) {
		return;
	}

	if (php_check_open_basedir(xml_filename TSRMLS_CC)) RETURN_FALSE;
	if (php_check_open_basedir(rrd_filename TSRMLS_CC)) RETURN_FALSE;

	/* zv_options = merge rrd_filename and zv_arr_options content */
	MAKE_STD_ZVAL(zv_options);
	array_init(zv_options);
	add_next_index_string(zv_options, rrd_filename, 1);
	if (zv_arr_options && Z_TYPE_P(zv_arr_options) == IS_ARRAY) {
		php_array_merge(Z_ARRVAL_P(zv_options), Z_ARRVAL_P(zv_arr_options), 0 TSRMLS_CC);
	}

	argv = rrd_args_init_by_phparray("restore", xml_filename, zv_options TSRMLS_CC);
	if (!argv) {
		zend_error(E_WARNING, "cannot allocate arguments options");
		RETURN_FALSE;
	}

	if (rrd_test_error()) rrd_clear_error();

	/* call rrd_ restore and test if fails */
	if (rrd_restore(argv->count-1, &argv->args[1]) == -1) {
		RETVAL_FALSE;
	} else {
		RETVAL_TRUE;
	}
	zval_dtor(zv_options);
	rrd_args_free(argv);
}
/* }}} */

/* {{{ proto bool rrd_tune(string file, array options)
	Tune an RRD file with the options passed (passed via array) */
PHP_FUNCTION(rrd_tune)
{
	char *filename;
	int filename_length;
	zval *zv_arr_options;
	rrd_args *argv;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa", &filename,
		&filename_length, &zv_arr_options) == FAILURE) {
		return;
	}

	if (!zend_hash_num_elements(Z_ARRVAL_P(zv_arr_options))) {
		zend_error(E_WARNING, "options array mustn't be empty");
		RETURN_FALSE;
	}

	if (php_check_open_basedir(filename TSRMLS_CC)) RETURN_FALSE;

	argv = rrd_args_init_by_phparray("tune", filename, zv_arr_options TSRMLS_CC);
	if (!argv) {
		zend_error(E_WARNING, "cannot allocate arguments options");
		RETURN_FALSE;
	}

	if (rrd_test_error()) rrd_clear_error();

	/* call rrd_tune and test if fails */
	if (rrd_tune(argv->count-1, &argv->args[1]) == -1 ) {
		RETVAL_FALSE;
	} else {
		RETVAL_TRUE;
	}
	rrd_args_free(argv);
}
/* }}} */

/* {{{ proto array rrd_xport(array options)
 * Creates a graph based on options passed via an array
 */
PHP_FUNCTION(rrd_xport)
{
	zval *zv_arr_options;
	rrd_args *argv;
	/* return values from rrd_xport */
	int xxsize;
	time_t start, end, time_index;
	ulong step, outvar_count;
	char **legend_v;
	rrd_value_t *data, *data_ptr;
	zval *zv_data;
	ulong outvar_index;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &zv_arr_options) == FAILURE) {
		return;
	}

	argv = rrd_args_init_by_phparray("xport", "", zv_arr_options TSRMLS_CC);
	if (!argv) {
		zend_error(E_WARNING, "cannot allocate arguments options");
		RETURN_FALSE;
	}

	if (rrd_test_error()) rrd_clear_error();

	/* call rrd_xport and test if fails */
	if (rrd_xport(argv->count-1, &argv->args[1], &xxsize, &start, &end, &step,
		&outvar_count, &legend_v, &data) == -1) {
		php_printf("rrd_xport failed");
		rrd_args_free(argv);
		RETURN_FALSE;
	}

	rrd_args_free(argv);

	/* fill rrd_xport return values into array */
	array_init(return_value);

	add_assoc_long(return_value, "start", start + step);
	add_assoc_long(return_value, "end", end);
	add_assoc_long(return_value, "step", step);

	/* no data available */
	if (!data) {
		add_assoc_null(return_value, "data");
		return;
	}

	MAKE_STD_ZVAL(zv_data);
	array_init(zv_data);

	for (outvar_index = 0; outvar_index < outvar_count; outvar_index++) {
		/* array for a whole one output variable data, it contains indexes
		 * array(
		 *  "legend" => "variable legend",
		 *  "data" => array(
		 *    920807400 => 0.14,
		 *    920807800 => 21,
		 *    ...
		 *  ))
		 */
		zval *zv_var_data, *time_data;
		MAKE_STD_ZVAL(zv_var_data);
		array_init(zv_var_data);
		MAKE_STD_ZVAL(time_data);
		array_init(time_data);

		add_assoc_string(zv_var_data, "legend", legend_v[outvar_index], 1);
		free(legend_v[outvar_index]);

		data_ptr = data + outvar_index;
		for (time_index = start+step; time_index <= end; time_index += step) {
			/* value for key (timestamp) in data array */
			zval *zv_timestamp;
			MAKE_STD_ZVAL(zv_timestamp);
			ZVAL_LONG(zv_timestamp, time_index);
			convert_to_string(zv_timestamp);

			add_assoc_double(time_data, Z_STRVAL_P(zv_timestamp), *data_ptr);
			data_ptr += outvar_count;
			zval_dtor(zv_timestamp);
		}
		add_assoc_zval(zv_var_data, "data", time_data);
		add_next_index_zval(zv_data, zv_var_data);
	}
	add_assoc_zval(return_value, "data", zv_data);
	free(legend_v);
	free(data);
}
/* }}} */

/* {{{ proto string rrd_version()
 * Gets version of underlying librrd.
 */
PHP_FUNCTION(rrd_version)
{
	RETVAL_STRING(rrd_strversion(), 1);
}

/* {{{ arguments */
ZEND_BEGIN_ARG_INFO(arginfo_rrd_fetch, 0)
	ZEND_ARG_INFO(0, file)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_rrd_first, 0, 0, 1)
	ZEND_ARG_INFO(0, file)
	ZEND_ARG_INFO(0, raaindex)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_rrd_last, 0)
	ZEND_ARG_INFO(0, file)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_rrd_lastupdate, 0)
	ZEND_ARG_INFO(0, file)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_rrd_restore, 0, 0, 2)
	ZEND_ARG_INFO(0, xml_file)
	ZEND_ARG_INFO(0, rrd_file)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_rrd_tune, 0)
	ZEND_ARG_INFO(0, file)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_rrd_xport, 0)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_rrd_version, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_rrd_info, 0)
	ZEND_ARG_INFO(0, file)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_rrd_graph, 0)
	ZEND_ARG_INFO(0, file)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_rrd_create, 0)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_rrd_update, 0)
	ZEND_ARG_INFO(0, file)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ */
static zend_function_entry rrd_functions[] = {
	PHP_FE(rrd_update, arginfo_rrd_update)
	PHP_FE(rrd_create, arginfo_rrd_create)
	PHP_FE(rrd_graph, arginfo_rrd_graph)
	PHP_FE(rrd_error, NULL)
	PHP_FE(rrd_fetch, arginfo_rrd_fetch)
	PHP_FE(rrd_first, arginfo_rrd_first)
	PHP_FE(rrd_info, arginfo_rrd_info)
	PHP_FE(rrd_last, arginfo_rrd_last)
	PHP_FE(rrd_lastupdate, arginfo_rrd_lastupdate)
	PHP_FE(rrd_restore, arginfo_rrd_restore)
	PHP_FE(rrd_tune, arginfo_rrd_tune)
	PHP_FE(rrd_xport, arginfo_rrd_xport)
	PHP_FE(rrd_version, arginfo_rrd_version)
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
	php_info_print_table_header(2, "rrd tool module", "enabled");
	php_info_print_table_row(2, "rrd tool module version", PHP_RRD_VERSION);
	php_info_print_table_row(2, "rrdtool library version", rrd_strversion());
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
	PHP_RRD_VERSION,
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
	result->count = option_count + (strlen(filename) ? 3 : 2);
	result->args = (char **)safe_emalloc(result->count, sizeof(char *), 0);

	/* "dummy" and command_name are always needed */
	result->args[0] = "dummy";
	result->args[1] = estrdup(command_name);

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
