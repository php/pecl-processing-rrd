/**
 * PHP bindings to the rrdtool
 *
 * This source file is subject to the BSD license that is bundled
 * with this package in the file LICENSE.
 * ---------------------------------------------------------------
 *  Author: Miroslav Kubelik <koubel@php.net>
 * ---------------------------------------------------------------
 */

#ifndef RRD_INFO_H
#define RRD_INFO_H

extern PHP_FUNCTION(rrd_info);

/* necessary, because rrd_info_t definition is needed for function definition */
#include <rrd.h>
extern unsigned rrd_info_toarray(const rrd_info_t *rrd_info_data, zval *array);

#endif  /* RRD_INFO_H */
