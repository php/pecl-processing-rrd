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
#include "ext/standard/php_array.h"

#include <rrd.h>

#include "php_rrd.h"
#include "rrd_create.h"

/* declare class entry */
static zend_class_entry *ce_rrd_create;
/* declare class handlers */
static zend_object_handlers rrd_create_handlers;

/* overloading the standard zend object structure (std property) in the need
of having dedicated creating/cloning/destruction functions
*/
typedef struct _rrd_create_object {
	/** path to newly created rrd file */
	char *file_path;
	/* "--start" parameters in rrd create */
	char *start_time;
	/* "--step" parameters in rrd create */
	zval zv_step;
	/* "DS" parameters in rrd create */
	zval zv_arr_data_sources;
	/* "RRA" parameters in rrd create */
	zval zv_arr_archives;
	zend_object std;
} rrd_create_object;

/**
 * fetch our custom object from user space object
 */
static inline rrd_create_object *php_rrd_create_fetch_object(zend_object *obj) {
	return (rrd_create_object *)((char*)(obj) - XtOffsetOf(rrd_create_object, std));
} 

/* {{{ rrd_create_object_dtor
close all resources and the memory allocated for our internal object
*/
static void rrd_create_object_dtor(zend_object *object)
{
	rrd_create_object *intern_obj = php_rrd_create_fetch_object(object);
	if (!intern_obj) return;

	if (intern_obj->file_path)
		efree(intern_obj->file_path);
	if (intern_obj->start_time)
		efree(intern_obj->start_time);
	if (!Z_ISUNDEF(intern_obj->zv_step))
		zval_dtor(&intern_obj->zv_step);
	if (!Z_ISUNDEF(intern_obj->zv_arr_data_sources))
		zval_dtor(&intern_obj->zv_arr_data_sources);
	if (!Z_ISUNDEF(intern_obj->zv_arr_archives))
		zval_dtor(&intern_obj->zv_arr_archives);

	zend_object_std_dtor(&intern_obj->std);
}
/* }}} */

/* {{{ rrd_create_object_new
creates new rrd create object
*/
static zend_object *rrd_create_object_new(zend_class_entry *ce)
{
	rrd_create_object *intern_obj = ecalloc(1, sizeof(rrd_create_object) + 
		zend_object_properties_size(ce));
	intern_obj->file_path = NULL;
	intern_obj->start_time = NULL;
	ZVAL_UNDEF(&intern_obj->zv_step);
	ZVAL_UNDEF(&intern_obj->zv_arr_data_sources);
	ZVAL_UNDEF(&intern_obj->zv_arr_archives);

	zend_object_std_init(&intern_obj->std, ce);
	object_properties_init(&intern_obj->std, ce);

	intern_obj->std.handlers = &rrd_create_handlers;

	return &intern_obj->std;
}
/* }}} */

/* {{{ proto void RRDCreator::__construct(string path [,string startTime]
 [,int step])
creates new object for creating rrd database
 */
PHP_METHOD(RRDCreator, __construct)
{
	rrd_create_object *intern_obj;
	char *path; size_t path_length;
	/* better to set defaults for optional parameters */
	zend_string *start_time;
	long step = 0;
	int argc = ZEND_NUM_ARGS();

	if (zend_parse_parameters(argc, "p|Sl", &path, &path_length,
		&start_time, &step) == FAILURE) {
		return;
	}

	if (path_length == 0) {
		zend_throw_exception(NULL,
			"path for rrd file cannot be empty string", 0);
		return;
	}

	if (argc > 1 && ZSTR_LEN(start_time) == 0) {
		zend_throw_exception(NULL,
			"startTime cannot be empty string", 0);
		return;
	}
	if (argc > 2 && step <= 0) {
		zend_throw_exception(NULL,
			"step parameter must be greater then 0", 0);
		return;
	}

	intern_obj = php_rrd_create_fetch_object(Z_OBJ_P(getThis()));
	intern_obj->file_path = estrdup(path);
	if (start_time) intern_obj->start_time = estrdup(ZSTR_VAL(start_time));
	if (step) {
		ZVAL_LONG(&intern_obj->zv_step, step);
	}
}
/* }}} */

/* {{{ proto RRDCreator::addDataSource(string description)
 Add information about data source
 */
PHP_METHOD(RRDCreator, addDataSource)
{
	rrd_create_object *intern_obj;
	zend_string *description;
	char *rrd_source_desc;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &description) == FAILURE) {
		return;
	}

	if (ZSTR_LEN(description) == 0) {
		zend_throw_exception(NULL,
			"description parameter cannot be empty string", 0);
		return;
	}


	intern_obj = php_rrd_create_fetch_object(Z_OBJ_P(getThis()));

	if (Z_ISUNDEF(intern_obj->zv_arr_data_sources)) {
		array_init(&intern_obj->zv_arr_data_sources);
	}

	rrd_source_desc = emalloc(ZSTR_LEN(description) + 4);
	strcpy(rrd_source_desc, "DS:");
	strcat(rrd_source_desc, ZSTR_VAL(description));

	add_next_index_string(&intern_obj->zv_arr_data_sources, rrd_source_desc);
	efree(rrd_source_desc);
}
/* }}} */

/* {{{ proto RRDCreator::addArchive(string description)
 Add information about new round robin archive
 */
PHP_METHOD(RRDCreator, addArchive)
{
	rrd_create_object *intern_obj;
	zend_string *description;
	char *rrd_archive_desc;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &description) == FAILURE) {
		return;
	}

	if (ZSTR_LEN(description) == 0) {
		zend_throw_exception(NULL,
			"description parameter cannot be empty string", 0);
		return;
	}

	intern_obj = php_rrd_create_fetch_object(Z_OBJ_P(getThis()));

	if (Z_ISUNDEF(intern_obj->zv_arr_archives)) {
		array_init(&intern_obj->zv_arr_archives);
	}

	rrd_archive_desc = emalloc(ZSTR_LEN(description) + 5);
	strcpy(rrd_archive_desc, "RRA:");
	strcat(rrd_archive_desc, ZSTR_VAL(description));

	add_next_index_string(&intern_obj->zv_arr_archives, rrd_archive_desc);
	efree(rrd_archive_desc);
}
/* }}} */

/* {{{ proto bool RRDCreator::save()
 Saves new rrd database according to current properties.
 */
PHP_METHOD(RRDCreator, save)
{
	rrd_create_object *intern_obj = php_rrd_create_fetch_object(Z_OBJ_P(getThis()));
	/* help structures for preparing arguments for rrd_create call */
	zval zv_create_argv;
	rrd_args *create_argv;

	array_init(&zv_create_argv);

	if (intern_obj->start_time) {
		const char *prefix = "--start=";
		char *start_time_str = emalloc(strlen(intern_obj->start_time)
			+ strlen(prefix) + 1);

		strcpy(start_time_str, prefix);
		strcat(start_time_str, intern_obj->start_time);
		add_next_index_string(&zv_create_argv, start_time_str);

		efree(start_time_str);
	}

	if (!Z_ISUNDEF(intern_obj->zv_step)) {
		const char *prefix = "--step=";
		char *start_time_str;

		convert_to_string(&intern_obj->zv_step);
		start_time_str = emalloc(strlen(prefix) + Z_STRLEN(intern_obj->zv_step) + 1);

		strcpy(start_time_str, prefix);
		strcat(start_time_str, Z_STRVAL(intern_obj->zv_step));
		add_next_index_string(&zv_create_argv, start_time_str);

		/* back to long, doesn't needed, but we are consistent */
		convert_to_long(&intern_obj->zv_step);
		efree(start_time_str);
	}

	/* add array of archive and data source strings into argument list */
	php_array_merge(Z_ARRVAL(zv_create_argv), Z_ARRVAL(intern_obj->zv_arr_data_sources));
	php_array_merge(Z_ARRVAL(zv_create_argv), Z_ARRVAL(intern_obj->zv_arr_archives));

	create_argv = rrd_args_init_by_phparray("create", intern_obj->file_path, &zv_create_argv);
	if (!create_argv) {
		zend_error(E_WARNING, "cannot allocate arguments options");
		zval_dtor(&zv_create_argv);
		RETURN_FALSE;
	}

	if (rrd_test_error()) rrd_clear_error();

	/* call rrd_create and test if fails */
	if (rrd_create(create_argv->count - 1, &create_argv->args[1]) == -1) {
		zval_dtor(&zv_create_argv);
		rrd_args_free(create_argv);

		/* throw exception with rrd error string */
		zend_throw_exception(NULL, rrd_get_error(), 0);
		rrd_clear_error();
		return;
	}

	zval_dtor(&zv_create_argv);
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
	size_t filename_length;
	zval *zv_arr_options;
	rrd_args *argv;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "pa", &filename,
		&filename_length, &zv_arr_options) == FAILURE) {
		return;
	}

	if (php_check_open_basedir(filename)) RETURN_FALSE;

	argv = rrd_args_init_by_phparray("create", filename, zv_arr_options);
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_rrdcreator_construct, 0, 0, 1)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, startTime)
	ZEND_ARG_INFO(0, step)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_rrdcreator_description, 0, 0, 1)
	ZEND_ARG_INFO(0, description)
ZEND_END_ARG_INFO()

/* class method table */
static zend_function_entry rrd_create_methods[] = {
	PHP_ME(RRDCreator, __construct, arginfo_rrdcreator_construct, ZEND_ACC_PUBLIC)
	PHP_ME(RRDCreator, save, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(RRDCreator, addDataSource, arginfo_rrdcreator_description, ZEND_ACC_PUBLIC)
	PHP_ME(RRDCreator, addArchive, arginfo_rrdcreator_description, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/* minit hook, called from main module minit */
void rrd_create_minit()
{
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "RRDCreator", rrd_create_methods);
	ce.create_object = rrd_create_object_new;
	ce_rrd_create = zend_register_internal_class(&ce);

	memcpy(&rrd_create_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	rrd_create_handlers.clone_obj = NULL;
	rrd_create_handlers.offset = XtOffsetOf(rrd_create_object, std);
	rrd_create_handlers.free_obj = rrd_create_object_dtor; 
}
