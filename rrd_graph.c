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
#include "rrd_graph.h"
#include "ext/standard/php_smart_str.h"

/* declare class entry */
static zend_class_entry *ce_rrd_graph;
/* declare class handlers */
static zend_object_handlers rrd_graph_handlers;

/* overloading the standard zend object structure (std property) in the need
of having dedicated creating/cloning/destruction functions
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

    return;
}
/* }}} */

/* {{{ proto array RRDGraph::save()
Saves graph according to current properties.
 */
PHP_METHOD(RRDGraph, save)
{
    rrd_graph_object *intern_obj = (rrd_graph_object *)zend_object_store_get_object(getThis() TSRMLS_CC);

    int xsize, ysize;
    double ymin,ymax;
    char **calcpr;
    /* array of arguments for rrd_graph call */
    char **argv;
    unsigned int argc,i;

    if (!intern_obj->zv_arr_options || Z_TYPE_P(intern_obj->zv_arr_options) != IS_ARRAY) {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C),
            "options aren't correctly set", 0 TSRMLS_CC);
        return;
    }

    if (rrd_test_error()) rrd_clear_error();

    argc = zend_hash_num_elements(Z_ARRVAL_P(intern_obj->zv_arr_options)) + 3;
    argv = (char **) safe_emalloc(argc, sizeof(char *), 0);
    /* "dummy" and "graph" arguments are needed */
    argv[0] = "dummy";
    argv[1] = estrdup("graph");
    argv[2] = estrdup(intern_obj->file_path);

    /* makes array of arguments for rrd_graph call from options array */
    zend_hash_internal_pointer_reset(Z_ARRVAL_P(intern_obj->zv_arr_options));
    for(i=3; i<argc; i++) {
        char *str_key;
        ulong num_key;
        zval **item;
        smart_str option = {0}; /* one argument option */

        /* option with string key means long option, hence they are used as
         "key=value" e.g. "--start=920804400"
         */
        if (zend_hash_get_current_key(Z_ARRVAL_P(intern_obj->zv_arr_options), &str_key, &num_key, 0) == HASH_KEY_IS_STRING) {
            smart_str_appends(&option, str_key);
            smart_str_appendc(&option, '=');
        };

        zend_hash_get_current_data(Z_ARRVAL_P(intern_obj->zv_arr_options), (void**) &item);

        if (Z_TYPE_PP(item) != IS_STRING)
            convert_to_string(*item);

        smart_str_appendl(&option, Z_STRVAL_PP(item), Z_STRLEN_PP(item));
        smart_str_0(&option);

        argv[i] = estrdup(option.c);

        smart_str_free(&option);

        if (i<argc) zend_hash_move_forward(Z_ARRVAL_P(intern_obj->zv_arr_options));
    }

    /* call rrd graph and test if fails */
    if (rrd_graph(argc-1, &argv[1], &calcpr, &xsize, &ysize, NULL, &ymin, &ymax) == -1) {

        for (i=1; i<argc; i++)
            efree(argv[i]);

        /* throw exception with rrd error string */
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), rrd_get_error(), 0 TSRMLS_CC);
        rrd_clear_error();
        return;
    }

    /* rrd_graph call was OK, so copy rrd_graph return values which are stored
     in calcpr to php array
    */
    zval zv_calcpr_array;

    INIT_PZVAL(&zv_calcpr_array)
    array_init(&zv_calcpr_array);
    if (calcpr) {
        for (i=0; calcpr[i]; i++) {
            add_next_index_string(&zv_calcpr_array, calcpr[i], 1);
            free(calcpr[i]);
        }
        free(calcpr);
    }

    /* made return value */
    array_init(return_value);
    add_assoc_long(return_value, "xsize", xsize);
    add_assoc_long(return_value, "ysize", ysize);
    /* add calcpr return values under "calcpr" key */
    zend_hash_update(return_value->value.ht, "calcpr", sizeof("calcpr"),
    (void *)&zv_calcpr_array, sizeof(zval *), NULL);

   	for (i=1; i<argc; i++)
	   efree(argv[i]);

    return;
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
