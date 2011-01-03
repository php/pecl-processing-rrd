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

#ifndef RRD_INFO_H
#define RRD_INFO_H

extern PHP_FUNCTION(rrd_info);

ZEND_BEGIN_ARG_INFO(arginfo_rrd_info, 0)
	ZEND_ARG_INFO(0, file)
ZEND_END_ARG_INFO()

/* necessary, because rrd_info_t definition is needed for function definition */
#include <rrd.h>
extern uint rrd_info_toarray(const rrd_info_t *rrd_info_data, zval *array TSRMLS_DC);

#endif  /* RRD_INFO_H */
