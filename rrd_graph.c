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

#include <rrd.h>

#include "php_rrd.h"
#include "rrd_info.h"
#include "rrd_graph.h"

/* declare class entry */
static zend_class_entry *ce_rrd_graph;
/* declare class handlers */
static zend_object_handlers rrd_graph_handlers;

/**
 * overloading the standard zend object structure (std property) in the need
 * of having dedicated creating/cloning/destruction functions
 */
typedef struct _rrd_graph_object {
	zend_object std;
	char *file_path;
	zval *zv_arr_options;
} rrd_graph_object;

/* {{{ rrd_graph_object_dtor
close all resources and the memory allocated for our internal object
*/
static void rrd_graph_object_dtor(void *object TSRMLS_DC)
{
	rrd_graph_object *intern_obj = (rrd_graph_object *)object;

	if (intern_obj->file_path)
		efree(intern_obj->file_path);
	if (intern_obj->zv_arr_options) {
		zval_dtor(intern_obj->zv_arr_options);
	}

	zend_object_std_dtor(&intern_obj->std TSRMLS_CC);
	efree(intern_obj);
}
/* }}} */

/* {{{ rrd_graph_object_new
creates new rrd graph object
*/
static zend_object_value rrd_graph_object_new(zend_class_entry *ce TSRMLS_DC)
{
	rrd_graph_object *intern_obj;
	zend_object_value retval;
	zval *tmp;

	intern_obj = ecalloc(1, sizeof(*intern_obj));
	zend_object_std_init(&intern_obj->std, ce TSRMLS_CC);
	intern_obj->file_path = NULL;
	intern_obj->zv_arr_options = NULL;

	zend_hash_copy(intern_obj->std.properties, &ce->default_properties,
		(copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval*)
	);

	retval.handle = zend_objects_store_put(intern_obj,
		(zend_objects_store_dtor_t)zend_objects_destroy_object,
		rrd_graph_object_dtor, NULL TSRMLS_CC
	);

	retval.handlers = &rrd_graph_handlers;

	return retval;
}
/* }}} */

/* {{{ proto void RRDGraph::__construct(string path)
creates new object for rrd graph function
 */
PHP_METHOD(RRDGraph, __construct)
{
	rrd_graph_object *intern_obj;
	char *path;
	int path_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &path_length) == FAILURE) {
		return;
	}

	intern_obj = (rrd_graph_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
	intern_obj->file_path = estrdup(path);
}
/* }}} */

/* {{{ proto void RRDGraph::setOptions(array options)
set command options for rrd graph call
 */
PHP_METHOD(RRDGraph, setOptions)
{
	rrd_graph_object *intern_obj;
	zval *zv_arr_options;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &zv_arr_options) == FAILURE) {
		return;
	}

	intern_obj = (rrd_graph_object*)zend_object_store_get_object(getThis() TSRMLS_CC);

	/* if our array is initialized, so delete it first */
	if (intern_obj->zv_arr_options) {
		zval_dtor(intern_obj->zv_arr_options);
	}

	/* copy array from parameter */
	MAKE_STD_ZVAL(intern_obj->zv_arr_options);
	*intern_obj->zv_arr_options = *zv_arr_options;
	zval_copy_ctor(intern_obj->zv_arr_options);
}
/* }}} */

/* {{{
 creates arguments for rrd_graph call for RRDGraph instance options
*/
static rrd_args *rrd_graph_obj_create_argv(const char *command_name, const rrd_graph_object *obj TSRMLS_DC)
{
	/* iterated item */
	zval **zv_option_val;
	/* arguments for rrd_graph call as php array - temporary storage */
	zval *zv_argv;
	rrd_args *result;

	MAKE_STD_ZVAL(zv_argv);
	array_init(zv_argv);

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(obj->zv_arr_options));
	while (zend_hash_get_current_data(Z_ARRVAL_P(obj->zv_arr_options), (void**)&zv_option_val) == SUCCESS) {
		/* copy for converted non-string value, because we don't want to modify
		 * original array item
		 */
		zval option_str_copy;
		/* pointer for current value, either on zv_option_val (current array item)
		 * or on option_str_copy (string converted item)
		 */
		zval *option_ptr = *zv_option_val;
		char *str_key;
		ulong num_key;
		smart_str option = {0}; /* one argument option */

		/* option with string key means long option, hence they are used as
		 * "key=value" e.g. "--start=920804400"
		 */
		if (zend_hash_get_current_key(Z_ARRVAL_P(obj->zv_arr_options), &str_key, &num_key, 0) == HASH_KEY_IS_STRING) {
			smart_str_appends(&option, str_key);
			smart_str_appendc(&option, '=');
		};

		/* if option isn't string, use new value as old one casted to string */
		if (Z_TYPE_PP(zv_option_val) != IS_STRING) {
			option_str_copy = **zv_option_val;
			zval_copy_ctor(&option_str_copy);
			convert_to_string(&option_str_copy);
			option_ptr = &option_str_copy;
		}

		smart_str_appendl(&option, Z_STRVAL_P(option_ptr), Z_STRLEN_P(option_ptr));
		smart_str_0(&option);

		add_next_index_string(zv_argv, option.c, 1);

		smart_str_free(&option);
		if (option_ptr != *zv_option_val) {
			zval_dtor(&option_str_copy);
		}

		zend_hash_move_forward(Z_ARRVAL_P(obj->zv_arr_options));
	}

	result = rrd_args_init_by_phparray(command_name, obj->file_path, zv_argv TSRMLS_CC);
	zval_dtor(zv_argv);

	return result;
}
/* }}} */

/* {{{ proto array RRDGraph::save()
Saves graph according to current option
 */
PHP_METHOD(RRDGraph, save)
{
	rrd_graph_object *intern_obj = (rrd_graph_object *)zend_object_store_get_object(getThis() TSRMLS_CC);

	/* returned values if rrd_graph doesn't fail */
	int xsize, ysize;
	double ymin,ymax;
	char **calcpr;

	/* arguments for rrd_graph call */
	rrd_args *graph_argv;

	if (!intern_obj->zv_arr_options || Z_TYPE_P(intern_obj->zv_arr_options) != IS_ARRAY) {
		zend_throw_exception(zend_exception_get_default(TSRMLS_C),
			"options aren't correctly set", 0 TSRMLS_CC);
		return;
	}

	if (php_check_open_basedir(intern_obj->file_path TSRMLS_CC)) {
		RETURN_FALSE;
	}

	graph_argv = rrd_graph_obj_create_argv("graph", intern_obj TSRMLS_CC);
	if (!graph_argv) {
		zend_error(E_WARNING, "cannot allocate arguments options");
		RETURN_FALSE;
	}

	if (rrd_test_error()) rrd_clear_error();

	/* call rrd graph and test if fails */
	if (rrd_graph(graph_argv->count - 1, &graph_argv->args[1], &calcpr, &xsize,
		&ysize, NULL, &ymin, &ymax) == -1) {

		/* throw exception with rrd error string */
		zend_throw_exception(zend_exception_get_default(TSRMLS_C), rrd_get_error(), 0 TSRMLS_CC);
		rrd_clear_error();
		rrd_args_free(graph_argv);
		return;
	}

	/* making return array */
	array_init(return_value);
	add_assoc_long(return_value, "xsize", xsize);
	add_assoc_long(return_value, "ysize", ysize);

	/* add calcpr return values under "calcpr" key
	 *
	 * if calcpr isn't presented add PHP NULL value
	 */
	if (!calcpr) {
		add_assoc_null(return_value, "calcpr");
	} else {
		/* calcpr is presented, hence create array for it, and add it to return array */
		zval *zv_calcpr_array;
		MAKE_STD_ZVAL(zv_calcpr_array)
		array_init(zv_calcpr_array);
		if (calcpr) {
			uint i;
			for (i = 0; calcpr[i]; i++) {
				add_next_index_string(zv_calcpr_array, calcpr[i], 1);
				free(calcpr[i]);
			}
			free(calcpr);
		}
		add_assoc_zval(return_value, "calcpr", zv_calcpr_array);
	}

	rrd_args_free(graph_argv);
}
/* }}} */

/* {{{ proto array RRDGraph::saveVerbose()
Saves graph according to current option with return an extra information about
saved image.
*/
PHP_METHOD(RRDGraph, saveVerbose)
{
	rrd_graph_object *intern_obj = (rrd_graph_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	/* return value from rrd_graphv */
	rrd_info_t *rrd_info_data, *data_p;

	/* arguments for rrd_graph call */
	rrd_args *graph_argv;

	if (!intern_obj->zv_arr_options || Z_TYPE_P(intern_obj->zv_arr_options) != IS_ARRAY) {
		zend_throw_exception(zend_exception_get_default(TSRMLS_C),
			"options aren't correctly set", 0 TSRMLS_CC);
		return;
	}

	graph_argv = rrd_graph_obj_create_argv("graphv", intern_obj TSRMLS_CC);
	if (!graph_argv) {
		zend_error(E_WARNING, "cannot allocate arguments options");
		RETURN_FALSE;
	}

	if (rrd_test_error()) rrd_clear_error();

	/* call rrd graphv and test if fails */
	rrd_info_data = rrd_graph_v(graph_argv->count - 1, &graph_argv->args[1]);
	if (!rrd_info_data) {
		/* throw exception with rrd error string */
		zend_throw_exception(zend_exception_get_default(TSRMLS_C), rrd_get_error(), 0 TSRMLS_CC);
		rrd_clear_error();
		rrd_args_free(graph_argv);
		return;
	}

	/* making return array */
	array_init(return_value);
	rrd_info_toarray(rrd_info_data, return_value);

	rrd_info_free(rrd_info_data);
	rrd_args_free(graph_argv);
}
/* }}} */

/* {{{ proto array rrd_graph(string file, array options)
	Ceates a graph based on options passed via an array.
*/
PHP_FUNCTION(rrd_graph)
{
	char *filename;
	int filename_length;
	zval *zv_arr_options;
	rrd_args *argv;
	/* returned values if rrd_graph doesn't fail */
	int xsize, ysize;
	double ymin,ymax;
	char **calcpr;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa", &filename,
		&filename_length, &zv_arr_options) == FAILURE) {
		return;
	}

	if (php_check_open_basedir(filename TSRMLS_CC)) RETURN_FALSE;

	argv = rrd_args_init_by_phparray("graph", filename, zv_arr_options TSRMLS_CC);
	if (!argv) {
		zend_error(E_WARNING, "cannot allocate arguments options");
		RETURN_FALSE;
	}

	if (rrd_test_error()) rrd_clear_error();

	/* call rrd graph and test if fails */
	if (rrd_graph(argv->count - 1, &argv->args[1], &calcpr, &xsize, &ysize,
		NULL, &ymin, &ymax) == -1) {

		rrd_args_free(argv);
		RETURN_FALSE;
	}

	/* making return array*/
	array_init(return_value);
	add_assoc_long(return_value, "xsize", xsize);
	add_assoc_long(return_value, "ysize", ysize);

	/* add calcpr return values under "calcpr" key
	 *
	 * if calcpr isn't presented add PHP NULL value
	 */
	if (!calcpr) {
		add_assoc_null(return_value, "calcpr");
	} else {
		/* calcpr is presented, hence create array for it, and add it to return array */
		zval *zv_calcpr_array;
		MAKE_STD_ZVAL(zv_calcpr_array)
		array_init(zv_calcpr_array);
		if (calcpr)	{
			uint i;
			for	(i=0; calcpr[i]; i++) {
				add_next_index_string(zv_calcpr_array, calcpr[i], 1);
				free(calcpr[i]);
			}
			free(calcpr);
		}
		add_assoc_zval(return_value, "calcpr", zv_calcpr_array);
	}

	rrd_args_free(argv);
}
/* }}} */

/* arguments */
ZEND_BEGIN_ARG_INFO_EX(arginfo_rrd_path, 0, 0, 1)
	ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_rrd_options, 0, 0, 1)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

/* class method table */
static zend_function_entry rrd_graph_methods[] = {
	PHP_ME(RRDGraph, __construct, arginfo_rrd_path, ZEND_ACC_PUBLIC)
	PHP_ME(RRDGraph, save, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(RRDGraph, saveVerbose, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(RRDGraph, setOptions, arginfo_rrd_options, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

/* minit hook, called from main module minit */
void rrd_graph_minit(TSRMLS_DC)
{
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "RRDGraph", rrd_graph_methods);
	ce.create_object = rrd_graph_object_new;
	ce_rrd_graph = zend_register_internal_class(&ce TSRMLS_CC);

	memcpy(&rrd_graph_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	rrd_graph_handlers.clone_obj = NULL;
}
