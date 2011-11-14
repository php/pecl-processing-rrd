/**
 * PHP bindings to the rrdtool
 *
 * This source file is subject to the BSD license that is bundled
 * with this package in the file LICENSE.
 * ---------------------------------------------------------------
 *  Author: Miroslav Kubelik <koubel@php.net>
 * ---------------------------------------------------------------
 */

#ifndef RRD_UPDATE_H
#define RRD_UPDATE_H

void rrd_update_minit(TSRMLS_D);
PHP_FUNCTION(rrd_update);

#endif  /* RRD_UPDATE_H */
