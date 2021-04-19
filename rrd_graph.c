/**
 * PHP bindings to the rrdtool
 *
 * This source file is subject to the BSD license that is bundled
 * with this package in the file LICENSE.
 * ---------------------------------------------------------------
 *  Author: Miroslav Kubelik <koubel@php.net>
 * ---------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "zend_exceptions.h"
#include "ext/standard/php_smart_string.h"

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
	char *file_path;
	zval zv_arr_options;
	zend_object std;
} rrd_graph_object;

/**
 * fetch our custom object from user space object
 */
static inline rrd_graph_object *php_rrd_graph_fetch_object(zend_object *obj) {
	return (rrd_graph_object *)((char*)(obj) - XtOffsetOf(rrd_graph_object, std));
} 

/* {{{ rrd_graph_object_dtor
close all resources and the memory allocated for our internal object
*/
static void rrd_graph_object_dtor(zend_object *object)
{
	rrd_graph_object *intern_obj = php_rrd_graph_fetch_object(object);
	if (!intern_obj) return;

	if (intern_obj->file_path){
		efree(intern_obj->file_path);
	}
	if (!Z_ISUNDEF(intern_obj->zv_arr_options)) {
		zval_dtor(&intern_obj->zv_arr_options);
	}

	zend_object_std_dtor(&intern_obj->std);
}
/* }}} */

/* {{{ rrd_graph_object_new
creates new rrd graph object
*/
static zend_object *rrd_graph_object_new(zend_class_entry *ce)
{
	rrd_graph_object *intern_obj = ecalloc(1, sizeof(rrd_graph_object) + 
		zend_object_properties_size(ce));
	intern_obj->file_path = NULL;
	ZVAL_UNDEF(&intern_obj->zv_arr_options);

	zend_object_std_init(&intern_obj->std, ce);
	object_properties_init(&intern_obj->std, ce);
	
	intern_obj->std.handlers = &rrd_graph_handlers;

	return &intern_obj->std;
}
/* }}} */

/* {{{ proto void RRDGraph::__construct(string path)
creates new object for rrd graph function
 */
PHP_METHOD(RRDGraph, __construct)
{
	rrd_graph_object *intern_obj;
	char *path;
	size_t path_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "p", &path, &path_length) == FAILURE) {
		return;
	}

	intern_obj = php_rrd_graph_fetch_object(Z_OBJ_P(getThis()));
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

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "a", &zv_arr_options) == FAILURE) {
		return;
	}

	intern_obj = php_rrd_graph_fetch_object(Z_OBJ_P(getThis()));

	/* if our array is initialized, so delete it first */
	if (!Z_ISUNDEF(intern_obj->zv_arr_options)) {
		zval_dtor(&intern_obj->zv_arr_options);
	}

	/* copy array from parameter */
	ZVAL_DUP(&intern_obj->zv_arr_options, zv_arr_options);
}
/* }}} */

/* {{{
 creates arguments for rrd_graph call for RRDGraph instance options
*/
static rrd_args *rrd_graph_obj_create_argv(const char *command_name, const rrd_graph_object *obj)
{
	/* iterated item and keys*/
	zval *zv_option_val;
	zend_ulong num_key;
	zend_string *zs_key;
	/* arguments for rrd_graph call as php array - temporary storage */
	zval zv_argv;
	rrd_args *result;

	array_init(&zv_argv);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(obj->zv_arr_options), num_key, zs_key, zv_option_val) {
		(void)num_key; /* to avoid -Wunused-but-set-variable */
		smart_string option = {0}; /* one argument option */

		/* option with string key means long option, hence they are used as
		 * "key=value" e.g. "--start=920804400"
		 */
		if (zs_key) {
			smart_string_appends(&option, ZSTR_VAL(zs_key));
			smart_string_appendc(&option, '=');
		}

		/* use always string for option value */
		if (Z_TYPE_P(zv_option_val) != IS_STRING) {
			convert_to_string(zv_option_val);
		}

		smart_string_appendl(&option, Z_STRVAL_P(zv_option_val), Z_STRLEN_P(zv_option_val));
		smart_string_0(&option);

		add_next_index_string(&zv_argv, option.c);

		smart_string_free(&option);
	} ZEND_HASH_FOREACH_END();

	result = rrd_args_init_by_phparray(command_name, obj->file_path, &zv_argv);
	zval_dtor(&zv_argv);

	return result;
}
/* }}} */

/* {{{ proto array RRDGraph::save()
Saves graph according to current option
 */
PHP_METHOD(RRDGraph, save)
{
	rrd_graph_object *intern_obj = php_rrd_graph_fetch_object(Z_OBJ_P(getThis()));

	/* returned values if rrd_graph doesn't fail */
	int xsize, ysize;
	double ymin,ymax;
	char **calcpr;

	/* arguments for rrd_graph call */
	rrd_args *graph_argv;

	if (Z_TYPE(intern_obj->zv_arr_options) != IS_ARRAY) {
		zend_throw_exception(NULL, "options aren't correctly set", 0);
		return;
	}

	if (php_check_open_basedir(intern_obj->file_path)) {
		RETURN_FALSE;
	}

	graph_argv = rrd_graph_obj_create_argv("graph", intern_obj);
	if (!graph_argv) {
		zend_error(E_WARNING, "cannot allocate arguments options");
		RETURN_FALSE;
	}

	if (rrd_test_error()) rrd_clear_error();

	/* call rrd graph and test if fails */
	if (rrd_graph(graph_argv->count - 1, &graph_argv->args[1], &calcpr, &xsize,
		&ysize, NULL, &ymin, &ymax) == -1) {

		/* throw exception with rrd error string */
		zend_throw_exception(NULL, rrd_get_error(), 0);
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
		zval zv_calcpr_array;
		array_init(&zv_calcpr_array);
		if (calcpr) {
			unsigned i;
			for (i = 0; calcpr[i]; i++) {
				add_next_index_string(&zv_calcpr_array, calcpr[i]);
				free(calcpr[i]);
			}
			free(calcpr);
		}
		add_assoc_zval(return_value, "calcpr", &zv_calcpr_array);
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
	rrd_graph_object *intern_obj = php_rrd_graph_fetch_object(Z_OBJ_P(getThis()));
	/* return value from rrd_graphv */
	rrd_info_t *rrd_info_data;

	/* arguments for rrd_graph call */
	rrd_args *graph_argv;

	if (Z_TYPE(intern_obj->zv_arr_options) != IS_ARRAY) {
		zend_throw_exception(NULL, "options aren't correctly set", 0); 
		return;
	}

	graph_argv = rrd_graph_obj_create_argv("graphv", intern_obj);
	if (!graph_argv) {
		zend_error(E_WARNING, "cannot allocate arguments options");
		RETURN_FALSE;
	}

	if (rrd_test_error()) rrd_clear_error();

	/* call rrd graphv and test if fails */
	rrd_info_data = rrd_graph_v(graph_argv->count - 1, &graph_argv->args[1]);
	if (!rrd_info_data) {
		/* throw exception with rrd error string */
		zend_throw_exception(NULL, rrd_get_error(), 0);
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
	size_t filename_length;
	zval *zv_arr_options;
	rrd_args *argv;
	/* returned values if rrd_graph doesn't fail */
	int xsize, ysize;
	double ymin,ymax;
	char **calcpr;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "pa", &filename,
		&filename_length, &zv_arr_options) == FAILURE) {
		return;
	}

	if (php_check_open_basedir(filename)) RETURN_FALSE;

	argv = rrd_args_init_by_phparray("graph", filename, zv_arr_options);
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
		zval zv_calcpr_array;
		array_init(&zv_calcpr_array);
		if (calcpr) {
			unsigned i;
			for (i = 0; calcpr[i]; i++) {
				add_next_index_string(&zv_calcpr_array, calcpr[i]);
				free(calcpr[i]);
			}
			free(calcpr);
		}
		add_assoc_zval(return_value, "calcpr", &zv_calcpr_array);
	}

	rrd_args_free(argv);
}
/* }}} */

/* arguments */
ZEND_BEGIN_ARG_INFO_EX(arginfo_rrd_void, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_rrd_path, 0, 0, 1)
	ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_rrd_options, 0, 0, 1)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

/* class method table */
static zend_function_entry rrd_graph_methods[] = {
	PHP_ME(RRDGraph, __construct, arginfo_rrd_path, ZEND_ACC_PUBLIC)
	PHP_ME(RRDGraph, save, arginfo_rrd_void, ZEND_ACC_PUBLIC)
	PHP_ME(RRDGraph, saveVerbose, arginfo_rrd_void, ZEND_ACC_PUBLIC)
	PHP_ME(RRDGraph, setOptions, arginfo_rrd_options, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/* minit hook, called from main module minit */
void rrd_graph_minit()
{
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "RRDGraph", rrd_graph_methods);
	ce.create_object = rrd_graph_object_new;
	ce_rrd_graph = zend_register_internal_class(&ce);

	memcpy(&rrd_graph_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	rrd_graph_handlers.clone_obj = NULL;
	rrd_graph_handlers.offset = XtOffsetOf(rrd_graph_object, std);
	rrd_graph_handlers.free_obj = rrd_graph_object_dtor; 
}
