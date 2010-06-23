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

#include <rrd.h>
#include "php.h"
#include "rrd_create.h"
#include "php_rrd.h"
#include "ext/standard/php_smart_str.h"

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
    uint step = 0;
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
    return;
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

    return;
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

    return;
}
/* }}} */

/* {{{ rrd_create_add_string_arg
 Adds strings form php array into array (in C sense) of arguments for rrd_create
 call
 @param argv "C" array of arguments for rrd_create, is already allocated
  with "counter param" filled elements
 @param arr php array of strings
 @param counter count of filled elements in array, is increased for each newly
  added string
*/
static void rrd_create_add_string_arg(char **argv, zval *arr, uint *counter TSRMLS_DC)
{
    uint elements = zend_hash_num_elements(Z_ARRVAL_P(arr));
    uint i;

    zend_hash_internal_pointer_reset(Z_ARRVAL_P(arr));
    for(i=0; i<elements; i++) {
        char *str_key;
        zval **item;
        smart_str option = {0}; /* one argument option */

        /* Data are always string, because they are added via setters, which
         * forces using strings as arguments.
         */
        zend_hash_get_current_data(Z_ARRVAL_P(arr), (void**) &item);
        smart_str_appendl(&option, Z_STRVAL_PP(item), Z_STRLEN_PP(item));
        smart_str_0(&option);

        argv[(*counter)++] = estrdup(option.c);
        smart_str_free(&option);

        if (i<elements) zend_hash_move_forward(Z_ARRVAL_P(arr));
    }
}
/* }}} */

/* {{{ proto array RRDCreator::save()
 Saves new rrd database according to current properties.
 */
PHP_METHOD(RRDCreator, save)
{
    rrd_create_object *intern_obj = (rrd_create_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
    /* array of arguments for rrd_create call */
    char **argv;
    uint argc,arg_counter,i;
    uint archive_count = zend_hash_num_elements(Z_ARRVAL_P(intern_obj->zv_arr_archives));
    uint data_source_count = zend_hash_num_elements(Z_ARRVAL_P(intern_obj->zv_arr_data_sources));

    /* 0-2 are defined below + start_time + step = 5 options */
    argc = archive_count + data_source_count + 5;

    argv = (char **) safe_emalloc(argc, sizeof(char *), 0);
    /* "dummy" and "create" arguments are needed */
    argv[0] = "dummy";
    argv[1] = estrdup("create");
    argv[2] = estrdup(intern_obj->file_path);
    arg_counter = 3;

    if (intern_obj->start_time) {
        const char *prefix = "--start=";
        char *start_time_str = emalloc(strlen(intern_obj->start_time)
            + strlen(prefix) + 1);

        strcpy(start_time_str, prefix);
        strcat(start_time_str, intern_obj->start_time);

        argv[arg_counter++] = estrdup(start_time_str);
        efree(start_time_str);
    } else {
        argc--;
    }

    if (intern_obj->zv_step) {
        const char *prefix = "--step=";
        char *start_time_str;

        convert_to_string(intern_obj->zv_step);
        start_time_str = emalloc(strlen(prefix) + Z_STRLEN_P(intern_obj->zv_step) + 1);

        strcpy(start_time_str, prefix);
        strcat(start_time_str, Z_STRVAL_P(intern_obj->zv_step));

        argv[arg_counter++] = estrdup(start_time_str);
        /* back to long, doesn't needed, but we are consistent */
        convert_to_long(intern_obj->zv_step);
        efree(start_time_str);
    } else {
        argc--;
    }

    /* add array of archive, data source strings into argument list for rrd_create */
    rrd_create_add_string_arg(argv, intern_obj->zv_arr_data_sources, &arg_counter TSRMLS_CC);
    rrd_create_add_string_arg(argv, intern_obj->zv_arr_archives, &arg_counter TSRMLS_CC);

    if (rrd_test_error()) rrd_clear_error();

    /* call rrd_create and test if fails */
    if (rrd_create(argc-1, &argv[1]) == -1) {

        for (i=1; i<argc; i++)
            efree(argv[i]);

        /* throw exception with rrd error string */
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), rrd_get_error(), 0 TSRMLS_CC);
        rrd_clear_error();
        return;
    }

    for (i=1; i<argc; i++)
        efree(argv[i]);

    return;
}
/* }}} */

/* {{{ proto int rrd_create(string file, array options)
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

