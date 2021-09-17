/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Gustavo Lopes <cataphract@php.net>                          |
   +----------------------------------------------------------------------+
*/

#include "../intl_cppshims.h"

#include <unicode/calendar.h>

extern "C" {
#include "../php_intl.h"
#define USE_CALENDAR_POINTER 1
#include "../calendar/calendar_class.h"
#include <ext/date/php_date.h>
}

#ifndef INFINITY
#define INFINITY (DBL_MAX+DBL_MAX)
#endif

#ifndef NAN
#define NAN (INFINITY-INFINITY)
#endif

/* {{{ timezone_convert_datetimezone
 *      The timezone in DateTime and DateTimeZone is not unified. */
U_CFUNC TimeZone *timezone_convert_datetimezone(int type,
												void *object,
												int is_datetime,
												intl_error *outside_error,
												const char *func TSRMLS_DC)
{
	char		*id = NULL,
				offset_id[] = "GMT+00:00";
	int			id_len = 0;
	char		*message;
	TimeZone	*timeZone;

	switch (type) {
		case TIMELIB_ZONETYPE_ID:
			id = is_datetime
				? ((php_date_obj*)object)->time->tz_info->name
				: ((php_timezone_obj*)object)->tzi.tz->name;
			id_len = strlen(id);
			break;
		case TIMELIB_ZONETYPE_OFFSET: {
			int offset_mins = is_datetime
				? -((php_date_obj*)object)->time->z
				: -(int)((php_timezone_obj*)object)->tzi.utc_offset,
				hours = offset_mins / 60,
				minutes = offset_mins - hours * 60;
			minutes *= minutes > 0 ? 1 : -1;

			if (offset_mins <= -24 * 60 || offset_mins >= 24 * 60) {
				spprintf(&message, 0, "%s: object has an time zone offset "
					"that's too large", func);
				intl_errors_set(outside_error, U_ILLEGAL_ARGUMENT_ERROR,
					message, 1 TSRMLS_CC);
				efree(message);
				return NULL;
			}

			id = offset_id;
			id_len = slprintf(id, sizeof(offset_id), "GMT%+03d:%02d",
				hours, minutes);
			break;
		}
		case TIMELIB_ZONETYPE_ABBR:
			id = is_datetime
				? ((php_date_obj*)object)->time->tz_abbr
				: ((php_timezone_obj*)object)->tzi.z.abbr;
			id_len = strlen(id);
			break;
	}

	UnicodeString s = UnicodeString(id, id_len, US_INV);
	timeZone = TimeZone::createTimeZone(s);
#if U_ICU_VERSION_MAJOR_NUM >= 49
	if (*timeZone == TimeZone::getUnknown()) {
#else
	UnicodeString resultingId;
	timeZone->getID(resultingId);
	if (resultingId == UnicodeString("Etc/Unknown", -1, US_INV)
			|| resultingId == UnicodeString("GMT", -1, US_INV)) {
#endif
		spprintf(&message, 0, "%s: time zone id '%s' "
			"extracted from ext/date DateTimeZone not recognized", func, id);
		intl_errors_set(outside_error, U_ILLEGAL_ARGUMENT_ERROR,
			message, 1 TSRMLS_CC);
		efree(message);
		delete timeZone;
		return NULL;
	}
	return timeZone;
}
/* }}} */

U_CFUNC int intl_datetime_decompose(zval *z, double *millis, TimeZone **tz,
		intl_error *err, const char *func TSRMLS_DC)
{
	zval	retval;
	zval	*zfuncname;
	char	*message;

	if (err && U_FAILURE(err->code)) {
		return FAILURE;
	}

	if (millis) {
		*millis = NAN;
	}
	if (tz) {
		*tz = NULL;
	}

	if (millis) {
		INIT_ZVAL(retval);
		MAKE_STD_ZVAL(zfuncname);
		ZVAL_STRING(zfuncname, "getTimestamp", 1);
		if (call_user_function(NULL, &(z), zfuncname, &retval, 0, NULL TSRMLS_CC)
				!= SUCCESS || Z_TYPE(retval) != IS_LONG) {
			spprintf(&message, 0, "%s: error calling ::getTimeStamp() on the "
					"object", func);
			intl_errors_set(err, U_INTERNAL_PROGRAM_ERROR,
				message, 1 TSRMLS_CC);
			efree(message);
			zval_ptr_dtor(&zfuncname);
			return FAILURE;
		}

		*millis = U_MILLIS_PER_SECOND * (double)Z_LVAL(retval);
		zval_ptr_dtor(&zfuncname);
	}

	if (tz) {
		php_date_obj *datetime;
		datetime = (php_date_obj*)zend_object_store_get_object(z TSRMLS_CC);
		if (!datetime->time) {
			spprintf(&message, 0, "%s: the DateTime object is not properly "
					"initialized", func);
			intl_errors_set(err, U_ILLEGAL_ARGUMENT_ERROR,
				message, 1 TSRMLS_CC);
			efree(message);
			return FAILURE;
		}
		if (!datetime->time->is_localtime) {
			*tz = TimeZone::getGMT()->clone();
		} else {
			*tz = timezone_convert_datetimezone(datetime->time->zone_type,
				datetime, 1, NULL, func TSRMLS_CC);
			if (*tz == NULL) {
				spprintf(&message, 0, "%s: could not convert DateTime's "
						"time zone", func);
				intl_errors_set(err, U_ILLEGAL_ARGUMENT_ERROR,
					message, 1 TSRMLS_CC);
				efree(message);
				return FAILURE;
			}
		}
	}

	return SUCCESS;
}

U_CFUNC double intl_zval_to_millis(zval *z, intl_error *err, const char *func TSRMLS_DC)
{
	double	rv = NAN;
	long	lv;
	int		type;
	char	*message;

	if (err && U_FAILURE(err->code)) {
		return NAN;
	}

	switch (Z_TYPE_P(z)) {
	case IS_STRING:
		type = is_numeric_string(Z_STRVAL_P(z), Z_STRLEN_P(z), &lv, &rv, 0);
		if (type == IS_DOUBLE) {
			rv *= U_MILLIS_PER_SECOND;
		} else if (type == IS_LONG) {
			rv = U_MILLIS_PER_SECOND * (double)lv;
		} else {
			spprintf(&message, 0, "%s: string '%s' is not numeric, "
					"which would be required for it to be a valid date", func,
					Z_STRVAL_P(z));
			intl_errors_set(err, U_ILLEGAL_ARGUMENT_ERROR,
				message, 1 TSRMLS_CC);
			efree(message);
		}
		break;
	case IS_LONG:
		rv = U_MILLIS_PER_SECOND * (double)Z_LVAL_P(z);
		break;
	case IS_DOUBLE:
		rv = U_MILLIS_PER_SECOND * Z_DVAL_P(z);
		break;
	case IS_OBJECT:
		if (instanceof_function(Z_OBJCE_P(z), php_date_get_date_ce() TSRMLS_CC)) {
			intl_datetime_decompose(z, &rv, NULL, err, func TSRMLS_CC);
		} else if (instanceof_function(Z_OBJCE_P(z), Calendar_ce_ptr TSRMLS_CC)) {
			Calendar_object *co = (Calendar_object *)
				zend_object_store_get_object(z TSRMLS_CC );
			if (co->ucal == NULL) {
				spprintf(&message, 0, "%s: IntlCalendar object is not properly "
						"constructed", func);
				intl_errors_set(err, U_ILLEGAL_ARGUMENT_ERROR,
					message, 1 TSRMLS_CC);
				efree(message);
			} else {
				UErrorCode status = UErrorCode();
				rv = (double)co->ucal->getTime(status);
				if (U_FAILURE(status)) {
					spprintf(&message, 0, "%s: call to internal "
							"Calendar::getTime() has failed", func);
					intl_errors_set(err, status, message, 1 TSRMLS_CC);
					efree(message);
				}
			}
		} else {
			/* TODO: try with cast(), get() to obtain a number */
			spprintf(&message, 0, "%s: invalid object type for date/time "
					"(only IntlCalendar and DateTime permitted)", func);
			intl_errors_set(err, U_ILLEGAL_ARGUMENT_ERROR,
				message, 1 TSRMLS_CC);
			efree(message);
		}
		break;
	default:
		spprintf(&message, 0, "%s: invalid PHP type for date", func);
		intl_errors_set(err, U_ILLEGAL_ARGUMENT_ERROR,
			message, 1 TSRMLS_CC);
		efree(message);
		break;
	}

	return rv;
}

