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

#include <rrd.h>

#include "php_rrd.h"
#include "rrd_info.h"

/* {{{ proto array rrd_info(string file)
	Gets the header information from an RRD.
*/
PHP_FUNCTION(rrd_info)
{
	char *filename;
	size_t filename_length;
	/* list of arguments for rrd_info call, it's more efficient then u
	 * usage of rrd_args, because there isn't array of arguments in parameters
	 */
	char *argv[3];
	/* return value from rrd_info_r() */
	rrd_info_t *rrd_info_data;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "p", &filename,
		&filename_length) == FAILURE) {
		return;
	}

	if (php_check_open_basedir(filename)) RETURN_FALSE;

	argv[0] = "dummy";
	argv[1] = estrdup("info");
	argv[2] = estrndup(filename, filename_length);

	rrd_info_data = rrd_info(2, &argv[1]);

	efree(argv[2]); efree(argv[1]);

	if (!rrd_info_data) RETURN_FALSE;

	/* making return array*/
	array_init(return_value);
	rrd_info_toarray(rrd_info_data, return_value);
	rrd_info_free(rrd_info_data);
}
/* }}} */

/* {{{ converts rrd_info_t struct into php array
  @return int 1 OK, 0 conversion failed
 */
uint rrd_info_toarray(const rrd_info_t *rrd_info_data, zval *array)
{
	const rrd_info_t *data_p;

	if (!rrd_info_data || Z_TYPE_P(array) != IS_ARRAY) return 0;

	data_p = rrd_info_data;
	while (data_p) {
		switch (data_p->type) {
		case RD_I_VAL:
			add_assoc_double(array, data_p->key, data_p->value.u_val);
			break;
		case RD_I_CNT:
			add_assoc_long(array, data_p->key, data_p->value.u_cnt);
			break;
		case RD_I_INT:
			add_assoc_long(array, data_p->key, data_p->value.u_int);
			break;
		case RD_I_STR:
			add_assoc_string(array, data_p->key, data_p->value.u_str);
			break;
		case RD_I_BLO:
			add_assoc_stringl(array, data_p->key, (char *)data_p->value.u_blo.ptr,
				 data_p->value.u_blo.size);
			break;
		}
		data_p = data_p->next;
	}

	return 1;
}
/* }}} */
