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

#include <rrd.h>
#include "php.h"
#include "rrd_create.h"
#include "php_rrd.h"
#include "zend_exceptions.h"
#include "ext/standard/php_array.h"

/* declare class entry */
static zend_class_entry *ce_rrd_create;
/* declare class handlers */
static zend_object_handlers rrd_create_handlers;

/* overloading the standard zend object structure (std property) in the need
of having dedicated creating/cloning/destruction functions
*/
typedef struct _rrd_create_object {
	zend_object std;
	/** path to newly created rrd file */
	char *file_path;
	/* "--start" parameters in rrd create */
	char *start_time;
	/* "--step" parameters in rrd create */
	zval *zv_step;
	/* "DS" parameters in rrd create */
	zval *zv_arr_data_sources;
	/* "RRA" parameters in rrd create */
	zval *zv_arr_archives;
} rrd_create_object;

/* {{{ rrd_create_object_dtor
close all resources and the memory allocated for our internal object
*/
static void rrd_create_object_dtor(void *object TSRMLS_DC)
{
	rrd_create_object *intern_obj = (rrd_create_object *)object;

	if (intern_obj->file_path)
		efree(intern_obj->file_path);
	if (intern_obj->start_time)
		efree(intern_obj->start_time);
	if (intern_obj->zv_step)
		zval_dtor(intern_obj->zv_step);
	if (intern_obj->zv_arr_data_sources)
		zval_dtor(intern_obj->zv_arr_data_sources);
	if (intern_obj->zv_arr_archives)
		zval_dtor(intern_obj->zv_arr_archives);

	zend_object_std_dtor(&intern_obj->std TSRMLS_CC);
	efree(intern_obj);
}
/* }}} */

/* {{{ rrd_create_object_new
creates new rrd create object
*/
static zend_object_value rrd_create_object_new(zend_class_entry *ce TSRMLS_DC)
{
	rrd_create_object *intern_obj;
	zend_object_value retval;
	zval *tmp;

	intern_obj = ecalloc(1, sizeof(*intern_obj));
	zend_object_std_init(&intern_obj->std, ce TSRMLS_CC);

	intern_obj->file_path = NULL;
	intern_obj->start_time = NULL;
	intern_obj->zv_step = NULL;
	intern_obj->zv_arr_data_sources = NULL;
	intern_obj->zv_arr_archives = NULL;

	zend_hash_copy(intern_obj->std.properties, &ce->default_properties,
		(copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval*)
	);

	retval.handle = zend_objects_store_put(intern_obj,
		(zend_objects_store_dtor_t)zend_objects_destroy_object,
		rrd_create_object_dtor, NULL TSRMLS_CC
	);

	retval.handlers = &rrd_create_handlers;

	return retval;
}
/* }}} */

/* {{{ proto void RRDCreator::__construct(string path [,string startTime]
 [,int step])
creates new object for creating rrd database
 */
PHP_METHOD(RRDCreator, __construct)
{
	rrd_create_object *intern_obj;
	char *path; int path_length;
	/* better to set defaults for optional parameters */
	char *start_time = NULL; int start_time_length = 0;
	long step = 0;
	int argc = ZEND_NUM_ARGS();

	if (zend_parse_parameters(argc TSRMLS_CC, "s|sl", &path, &path_length,
		&start_time, &start_time_length, &step) == FAILURE) {
		return;
	}

	if (path_length == 0) {
		zend_throw_exception(zend_exception_get_default(TSRMLS_C),
			"path for rrd file cannot be empty string", 0 TSRMLS_CC);
		return;
	}

	if (argc > 1 && start_time_length == 0) {
		zend_throw_exception(zend_exception_get_default(TSRMLS_C),
			"startTime cannot be empty string", 0 TSRMLS_CC);
		return;
	}
	if (argc > 2 && step <= 0) {
		zend_throw_exception(zend_exception_get_default(TSRMLS_C),
			"step parameter must be greater then 0", 0 TSRMLS_CC);
		return;
	}

	intern_obj = (rrd_create_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
	intern_obj->file_path = estrdup(path);
	if (start_time) intern_obj->start_time = estrdup(start_time);
	if (step) {
		MAKE_STD_ZVAL(intern_obj->zv_step);
		ZVAL_LONG(intern_obj->zv_step,step);
	}
}
/* }}} */

/* {{{ proto RRDCreator::addDataSource(string description)
 Add information about data source
 */
PHP_METHOD(RRDCreator, addDataSource)
{
	rrd_create_object *intern_obj;
	char *desc, *rrd_source_desc;
	int desc_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &desc, &desc_length) == FAILURE) {
		return;
	}

	if (desc_length == 0) {
		zend_throw_exception(zend_exception_get_default(TSRMLS_C),
			"description parameter cannot be empty string", 0 TSRMLS_CC);
		return;
	}

	intern_obj = (rrd_create_object*)zend_object_store_get_object(getThis() TSRMLS_CC);

	if (!intern_obj->zv_arr_data_sources) {
		MAKE_STD_ZVAL(intern_obj->zv_arr_data_sources);
		array_init(intern_obj->zv_arr_data_sources);
	}

	rrd_source_desc = emalloc(desc_length + 4);
	strcpy(rrd_source_desc, "DS:");
	strcat(rrd_source_desc, desc);

	add_next_index_string(intern_obj->zv_arr_data_sources, rrd_source_desc, 1);
	efree(rrd_source_desc);
}
/* }}} */

/* {{{ proto RRDCreator::addArchive(string description)
 Add information about new round robin archive
 */
PHP_METHOD(RRDCreator, addArchive)
{
	rrd_create_object *intern_obj;
	char *desc, *rrd_archive_desc;
	int desc_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &desc, &desc_length) == FAILURE) {
		return;
	}

	if (desc_length == 0) {
		zend_throw_exception(zend_exception_get_default(TSRMLS_C),
			"description parameter cannot be empty string", 0 TSRMLS_CC);
		return;
	}

	intern_obj = (rrd_create_object*)zend_object_store_get_object(getThis() TSRMLS_CC);

	if (!intern_obj->zv_arr_archives) {
		MAKE_STD_ZVAL(intern_obj->zv_arr_archives);
		array_init(intern_obj->zv_arr_archives);
	}

	rrd_archive_desc = emalloc(desc_length + 5);
	strcpy(rrd_archive_desc, "RRA:");
	strcat(rrd_archive_desc, desc);

	add_next_index_string(intern_obj->zv_arr_archives, rrd_archive_desc, 1);
	efree(rrd_archive_desc);
}
/* }}} */

/* {{{ proto bool RRDCreator::save()
 Saves new rrd database according to current properties.
 */
PHP_METHOD(RRDCreator, save)
{
	rrd_create_object *intern_obj = (rrd_create_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	/* help structures for preparing arguments for rrd_create call */
	zval *zv_create_argv;
	rrd_args *create_argv;

	MAKE_STD_ZVAL(zv_create_argv);
	array_init(zv_create_argv);

	if (intern_obj->start_time) {
		const char *prefix = "--start=";
		char *start_time_str = emalloc(strlen(intern_obj->start_time)
			+ strlen(prefix) + 1);

		strcpy(start_time_str, prefix);
		strcat(start_time_str, intern_obj->start_time);
		add_next_index_string(zv_create_argv, start_time_str, 1);

		efree(start_time_str);
	}

	if (intern_obj->zv_step) {
		const char *prefix = "--step=";
		char *start_time_str;

		convert_to_string(intern_obj->zv_step);
		start_time_str = emalloc(strlen(prefix) + Z_STRLEN_P(intern_obj->zv_step) + 1);

		strcpy(start_time_str, prefix);
		strcat(start_time_str, Z_STRVAL_P(intern_obj->zv_step));
		add_next_index_string(zv_create_argv, start_time_str, 1);

		/* back to long, doesn't needed, but we are consistent */
		convert_to_long(intern_obj->zv_step);
		efree(start_time_str);
	}

	/* add array of archive and data source strings into argument list */
	php_array_merge(Z_ARRVAL_P(zv_create_argv), Z_ARRVAL_P(intern_obj->zv_arr_data_sources),
		0 TSRMLS_CC);
	php_array_merge(Z_ARRVAL_P(zv_create_argv), Z_ARRVAL_P(intern_obj->zv_arr_archives),
		0 TSRMLS_CC);

	create_argv = rrd_args_init_by_phparray("create", intern_obj->file_path,
		zv_create_argv TSRMLS_CC);
	if (!create_argv) {
		zend_error(E_WARNING, "cannot allocate arguments options");
		zval_dtor(zv_create_argv);
		RETURN_FALSE;
	}

	if (rrd_test_error()) rrd_clear_error();

	/* call rrd_create and test if fails */
	if (rrd_create(create_argv->count - 1, &create_argv->args[1]) == -1) {
		zval_dtor(zv_create_argv);
		rrd_args_free(create_argv);

		/* throw exception with rrd error string */
		zend_throw_exception(zend_exception_get_default(TSRMLS_C), rrd_get_error(), 0 TSRMLS_CC);
		rrd_clear_error();
		return;
	}

	zval_dtor(zv_create_argv);
	rrd_args_free(create_argv);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool rrd_create(string file, array options)
	Create an RRD file with the options passed
 */
PHP_FUNCTION(rrd_create)
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

	argv = rrd_args_init_by_phparray("create", filename, zv_arr_options TSRMLS_CC);
	if (!argv) {
		zend_error(E_WARNING, "cannot allocate arguments options");
		RETURN_FALSE;
	}

	if (rrd_test_error()) rrd_clear_error();

	if (rrd_create(argv->count - 1, &argv->args[1]) == -1 ) {
		RETVAL_FALSE;
	} else {
		RETVAL_TRUE;
	}

	rrd_args_free(argv);
}
/* }}} */

/* arguments */
ZEND_BEGIN_ARG_INFO_EX(arginfo_construct, 0, 0, 1)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, startTime)
	ZEND_ARG_INFO(0, step)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_description, 0, 0, 1)
	ZEND_ARG_INFO(0, description)
ZEND_END_ARG_INFO()

/* class method table */
static zend_function_entry rrd_create_methods[] = {
	PHP_ME(RRDCreator, __construct, arginfo_construct, ZEND_ACC_PUBLIC)
	PHP_ME(RRDCreator, save, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(RRDCreator, addDataSource, arginfo_description, ZEND_ACC_PUBLIC)
	PHP_ME(RRDCreator, addArchive, arginfo_description, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

/* minit hook, called from main module minit */
void rrd_create_minit(TSRMLS_DC)
{
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "RRDCreator", rrd_create_methods);
	ce.create_object = rrd_create_object_new;
	ce_rrd_create = zend_register_internal_class(&ce TSRMLS_CC);

	memcpy(&rrd_create_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	rrd_create_handlers.clone_obj = NULL;
}
