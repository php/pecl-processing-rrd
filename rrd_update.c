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

#include <rrd.h>
#include "php.h"
#include "php_rrd.h"
#include "rrd_update.h"
#include "ext/standard/php_smart_str.h"

/* declare class entry */
static zend_class_entry *ce_rrd_update;
/* declare class handlers */
static zend_object_handlers rrd_update_handlers;

/* overloading the standard zend object structure (std property) in the need
of having dedicated creating/cloning/destruction functions
*/
typedef struct _rrd_update_object {
	zend_object std;
	/** path to newly created rrd file */
	char *file_path;
} rrd_update_object;

/* {{{ rrd_update_object_dtor
close all resources and the memory allocated for our internal object
*/
static void rrd_update_object_dtor(void *object TSRMLS_DC)
{
	rrd_update_object *intern_obj = (rrd_update_object *)object;

	if (intern_obj->file_path)
		efree(intern_obj->file_path);

	zend_object_std_dtor(&intern_obj->std TSRMLS_CC);
	efree(intern_obj);
}
/* }}} */


/* {{{ rrd_update_object_new
creates new rrd update object
*/
static zend_object_value rrd_update_object_new(zend_class_entry *ce TSRMLS_DC)
{
	rrd_update_object *intern_obj;
	zend_object_value retval;
	zval *tmp;

	intern_obj = ecalloc(1, sizeof(*intern_obj));
	zend_object_std_init(&intern_obj->std, ce TSRMLS_CC);

	intern_obj->file_path = NULL;

	zend_hash_copy(intern_obj->std.properties, &ce->default_properties,
		(copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval*)
	);

	retval.handle = zend_objects_store_put(intern_obj,
		(zend_objects_store_dtor_t)zend_objects_destroy_object,
		rrd_update_object_dtor, NULL TSRMLS_CC
	);

	retval.handlers = &rrd_update_handlers;

	return retval;
}
/* }}} */

 /* {{{ proto void RRDUpdater::__construct(string path)
creates new object for rrd update function
 */
PHP_METHOD(RRDUpdater, __construct)
{
	rrd_update_object *intern_obj;
	char *path;
	int path_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &path_length) == FAILURE) {
		return;
	}

	intern_obj = (rrd_update_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
	intern_obj->file_path = estrdup(path);
}
/* }}} */

/* {{{ proto array RRDUpdater::update(array $values, [string time=time()])
 Updates data sources in RRD database
 */
PHP_METHOD(RRDUpdater, update)
{
	rrd_update_object *intern_obj;
	zval *zv_values_array;

	/* help structures for preparing arguments for rrd_update call */
	zval *zv_update_argv;
	rrd_args *update_argv;

	char *time = NULL;
	int time_str_length = 0;

	int argc = ZEND_NUM_ARGS();
	uint ds_count,i;
	/* string for all data source names formated for rrd_update call */
	smart_str ds_names = {0};
	/* string for all data source values for rrd_update call */
	smart_str ds_vals = {0};

	if (zend_parse_parameters(argc TSRMLS_CC, "a|s", &zv_values_array, &time,
		&time_str_length) == FAILURE) {
		return;
	}

	ds_count = zend_hash_num_elements(Z_ARRVAL_P(zv_values_array));
	if (ds_count<=0) {
		RETURN_TRUE;
	}

	intern_obj = (rrd_update_object *)zend_object_store_get_object(getThis() TSRMLS_CC);

	if (php_check_open_basedir(intern_obj->file_path TSRMLS_CC)) {
		RETURN_FALSE;
	}

	if (time_str_length == 0) {
		if (argc > 1) {
			zend_throw_exception(zend_exception_get_default(TSRMLS_C),
				"time cannot be empty string", 0 TSRMLS_CC);
			return;
		}

		/* default time string, see rrdtool update man page, it's need to be
		 freed
		*/
		time = estrdup("N");
	}

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(zv_values_array));
	for (i=0; i<ds_count; i++) {
		char *ds_name;
		zval **ds_val;

		if (ds_names.len) smart_str_appendc(&ds_names, ':');
		else smart_str_appends(&ds_names,"--template=");

		zend_hash_get_current_key(Z_ARRVAL_P(zv_values_array), &ds_name, NULL, 0);
		smart_str_appends(&ds_names, ds_name);

		/* "timestamp:value" string */
		smart_str_appends(&ds_vals, time);
		smart_str_appendc(&ds_vals, ':');
		zend_hash_get_current_data(Z_ARRVAL_P(zv_values_array), (void**) &ds_val);
		if (Z_TYPE_PP(ds_val) != IS_STRING)
			convert_to_string(*ds_val);
		smart_str_appendl(&ds_vals, Z_STRVAL_PP(ds_val), Z_STRLEN_PP(ds_val));
	};

	smart_str_0(&ds_names);
	smart_str_0(&ds_vals);

	/* add copy of names and values strings into arguments array and free
	 * original strings
	 */
	MAKE_STD_ZVAL(zv_update_argv);
	array_init(zv_update_argv);
	add_next_index_string(zv_update_argv, ds_names.c, 1);
	add_next_index_string(zv_update_argv, ds_vals.c, 1);
	smart_str_free(&ds_names);
	smart_str_free(&ds_vals);

	update_argv = rrd_args_init_by_phparray("update", intern_obj->file_path, zv_update_argv TSRMLS_CC);
	if (!update_argv) {
		zend_error(E_WARNING, "cannot allocate arguments options");
		zval_dtor(zv_update_argv);
		if (time_str_length == 0) efree(time);
		RETURN_FALSE;
	}

	if (rrd_test_error()) rrd_clear_error();

	/* call rrd_update and test if fails */
	if (rrd_update(update_argv->count - 1, &update_argv->args[1]) == -1) {
		zval_dtor(zv_update_argv);
		rrd_args_free(update_argv);
		if (time_str_length == 0) efree(time);

		/* throw exception with rrd error string */
		zend_throw_exception(zend_exception_get_default(TSRMLS_C), rrd_get_error(), 0 TSRMLS_CC);
		rrd_clear_error();
		return;
	}

	/* parameter isn't presented and we alloced default one, so we need to
	 * free this one
	 */
	if (time_str_length == 0) efree(time);
	zval_dtor(zv_update_argv);
	rrd_args_free(update_argv);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int rrd_update(string file, array options)
	Updates the RRD file with a particular options and values.
*/
PHP_FUNCTION(rrd_update)
{
	char *filename;
	int filename_length;
	zval *zv_arr_options;
	rrd_args *argv;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa", &filename,
		&filename_length, &zv_arr_options) == FAILURE) {
		return;
	}

	if (php_check_open_basedir(filename TSRMLS_CC)) RETURN_FALSE;

	argv = rrd_args_init_by_phparray("update", filename, zv_arr_options TSRMLS_CC);
	if (!argv) {
		zend_error(E_WARNING, "cannot allocate arguments options");
		RETURN_FALSE;
	}

	if (rrd_test_error()) rrd_clear_error();

	if (rrd_update(argv->count - 1, &argv->args[1]) == -1 ) {
		RETVAL_FALSE;
	} else {
		RETVAL_TRUE;
	}

	rrd_args_free(argv);
}
/* }}} */

/* arguments */
ZEND_BEGIN_ARG_INFO(arginfo_construct, 0)
	ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_update, 0, 0, 1)
	ZEND_ARG_INFO(0, values)
	ZEND_ARG_INFO(0, time)
ZEND_END_ARG_INFO()

/* class method table */
static zend_function_entry rrd_update_methods[] = {
	PHP_ME(RRDUpdater, __construct, arginfo_construct, ZEND_ACC_PUBLIC)
	PHP_ME(RRDUpdater, update, arginfo_update, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

/* minit hook, called from main module minit */
void rrd_update_minit(TSRMLS_DC)
{
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "RRDUpdater", rrd_update_methods);
	ce.create_object = rrd_update_object_new;
	ce_rrd_update = zend_register_internal_class(&ce TSRMLS_CC);

	memcpy(&rrd_update_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	rrd_update_handlers.clone_obj = NULL;
}
