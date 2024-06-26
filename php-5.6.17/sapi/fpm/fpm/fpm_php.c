
	/* $Id: fpm_php.c,v 1.22.2.4 2008/12/13 03:21:18 anight Exp $ */
	/* (c) 2007,2008 Andrei Nigmatulin */

#include "fpm_config.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "php.h"
#include "php_main.h"
#include "php_ini.h"
#include "ext/standard/dl.h"

#include "fastcgi.h"

#include "fpm.h"
#include "fpm_php.h"
#include "fpm_cleanup.h"
#include "fpm_worker_pool.h"
#include "zlog.h"

static char **limit_extensions = NULL;

static int fpm_php_zend_ini_alter_master(char *name, int name_length, char *new_value, int new_value_length, int mode, int stage TSRMLS_DC) /* {{{ */
{
	zend_ini_entry *ini_entry;
	char *duplicate;

	if (zend_hash_find(EG(ini_directives), name, name_length, (void **) &ini_entry) == FAILURE) {
		return FAILURE;
	}

	duplicate = strdup(new_value);

	if (!ini_entry->on_modify
			|| ini_entry->on_modify(ini_entry, duplicate, new_value_length,
				ini_entry->mh_arg1, ini_entry->mh_arg2, ini_entry->mh_arg3, stage TSRMLS_CC) == SUCCESS) {
		ini_entry->value = duplicate;
		ini_entry->value_length = new_value_length;
		ini_entry->modifiable = mode;
	} else {
		free(duplicate);
	}

	return SUCCESS;
}
/* }}} */

static void fpm_php_disable(char *value, int (*zend_disable)(char *, uint TSRMLS_DC) TSRMLS_DC) /* {{{ */
{
	char *s = 0, *e = value;

	while (*e) {
		switch (*e) {
			case ' ':
			case ',':
				if (s) {
					*e = '\0';
					zend_disable(s, e - s TSRMLS_CC);
					s = 0;
				}
				break;
			default:
				if (!s) {
					s = e;
				}
				break;
		}
		e++;
	}

	if (s) {
		zend_disable(s, e - s TSRMLS_CC);
	}
}
/* }}} */

int fpm_php_apply_defines_ex(struct key_value_s *kv, int mode) /* {{{ */
{
	TSRMLS_FETCH();

	char *name = kv->key;
	char *value = kv->value;
	int name_len = strlen(name);
	int value_len = strlen(value);

	if (!strcmp(name, "extension") && *value) {
		zval zv;
		php_dl(value, MODULE_PERSISTENT, &zv, 1 TSRMLS_CC);
		return Z_BVAL(zv) ? 1 : -1;
	}

	if (fpm_php_zend_ini_alter_master(name, name_len+1, value, value_len, mode, PHP_INI_STAGE_ACTIVATE TSRMLS_CC) == FAILURE) {
		return -1;
	}

	if (!strcmp(name, "disable_functions") && *value) {
		char *v = strdup(value);
		PG(disable_functions) = v;
		fpm_php_disable(v, zend_disable_function TSRMLS_CC);
		return 1;
	}

	if (!strcmp(name, "disable_classes") && *value) {
		char *v = strdup(value);
		PG(disable_classes) = v;
		fpm_php_disable(v, zend_disable_class TSRMLS_CC);
		return 1;
	}

	return 1;
}
/* }}} */

static int fpm_php_apply_defines(struct fpm_worker_pool_s *wp) /* {{{ */
{
	struct key_value_s *kv;

	for (kv = wp->config->php_values; kv; kv = kv->next) {
		if (fpm_php_apply_defines_ex(kv, ZEND_INI_USER) == -1) {
			zlog(ZLOG_ERROR, "Unable to set php_value '%s'", kv->key);
		}
	}

	for (kv = wp->config->php_admin_values; kv; kv = kv->next) {
		if (fpm_php_apply_defines_ex(kv, ZEND_INI_SYSTEM) == -1) {
			zlog(ZLOG_ERROR, "Unable to set php_admin_value '%s'", kv->key);
		}
	}

	return 0;
}

static int fpm_php_set_allowed_clients(struct fpm_worker_pool_s *wp) /* {{{ */
{
	if (wp->listen_address_domain == FPM_AF_INET) {
		fcgi_set_allowed_clients(wp->config->listen_allowed_clients);
	}
	return 0;
}
/* }}} */

#if 0 /* Comment out this non used function. It could be used later. */
static int fpm_php_set_fcgi_mgmt_vars(struct fpm_worker_pool_s *wp) /* {{{ */
{
	char max_workers[10 + 1]; /* 4294967295 */
	int len;

	len = sprintf(max_workers, "%u", (unsigned int) wp->config->pm_max_children);

	fcgi_set_mgmt_var("FCGI_MAX_CONNS", sizeof("FCGI_MAX_CONNS")-1, max_workers, len);
	fcgi_set_mgmt_var("FCGI_MAX_REQS",  sizeof("FCGI_MAX_REQS")-1,  max_workers, len);
	return 0;
}
/* }}} */
#endif

char *fpm_php_script_filename(TSRMLS_D) /* {{{ */
{
	return SG(request_info).path_translated;
}
/* }}} */

char *fpm_php_request_uri(TSRMLS_D) /* {{{ */
{
	return (char *) SG(request_info).request_uri;
}
/* }}} */

char *fpm_php_request_method(TSRMLS_D) /* {{{ */
{
	return (char *) SG(request_info).request_method;
}
/* }}} */

char *fpm_php_query_string(TSRMLS_D) /* {{{ */
{
	return SG(request_info).query_string;
}
/* }}} */

char *fpm_php_auth_user(TSRMLS_D) /* {{{ */
{
	return SG(request_info).auth_user;
}
/* }}} */

size_t fpm_php_content_length(TSRMLS_D) /* {{{ */
{
	return SG(request_info).content_length;
}
/* }}} */

static void fpm_php_cleanup(int which, void *arg) /* {{{ */
{
	TSRMLS_FETCH();
	php_module_shutdown(TSRMLS_C);
	sapi_shutdown();
}
/* }}} */

void fpm_php_soft_quit() /* {{{ */
{
	fcgi_set_in_shutdown(1);
}
/* }}} */

int fpm_php_init_main() /* {{{ */
{
	if (0 > fpm_cleanup_add(FPM_CLEANUP_PARENT, fpm_php_cleanup, 0)) {
		return -1;
	}
	return 0;
}
/* }}} */

int fpm_php_init_child(struct fpm_worker_pool_s *wp) /* {{{ */
{
	if (0 > fpm_php_apply_defines(wp) ||
		0 > fpm_php_set_allowed_clients(wp)) {
		return -1;
	}

	if (wp->limit_extensions) {
		limit_extensions = wp->limit_extensions;
	}
	return 0;
}
/* }}} */

int fpm_php_limit_extensions(char *path) /* {{{ */
{/* lux:文件后缀限制 */
	char **p;
	size_t path_len;

	if (!path || !limit_extensions) {
		return 0; /* allowed by default */
	}

	p = limit_extensions;
	path_len = strlen(path);
	while (p && *p) {
		size_t ext_len = strlen(*p);/* lux: 注册的扩展名长度，如.php，为4 */
		if (path_len > ext_len) {
			char *path_ext = path + path_len - ext_len; /* lux: 按照注册的扩展名长度去截取。如.php为4，则对请求的url，截取最后4位 */
			if (strcmp(*p, path_ext) == 0) { /* lux: 如果一样则合法，否则不合法，继续尝试下一个 */
				return 0; /* allow as the extension has been found */
			}
		}
		p++;
	}


	zlog(ZLOG_NOTICE, "Access to the script '%s' has been denied (see security.limit_extensions)", path);
	return 1; /* extension not found: not allowed  */
}
/* }}} */

char* fpm_php_get_string_from_table(char *table, char *key TSRMLS_DC) /* {{{ */
{
	zval **data, **tmp;
	char *string_key;
	uint string_len;
	ulong num_key;
	if (!table || !key) {
		return NULL;
	}

	/* inspired from ext/standard/info.c */

	zend_is_auto_global(table, strlen(table) TSRMLS_CC);

	/* find the table and ensure it's an array */
	if (zend_hash_find(&EG(symbol_table), table, strlen(table) + 1, (void **) &data) == SUCCESS && Z_TYPE_PP(data) == IS_ARRAY) {

		/* reset the internal pointer */
		zend_hash_internal_pointer_reset(Z_ARRVAL_PP(data));

		/* parse the array to look for our key */
		while (zend_hash_get_current_data(Z_ARRVAL_PP(data), (void **) &tmp) == SUCCESS) {
			/* ensure the key is a string */
			if (zend_hash_get_current_key_ex(Z_ARRVAL_PP(data), &string_key, &string_len, &num_key, 0, NULL) == HASH_KEY_IS_STRING) {
				/* compare to our key */
				if (!strncmp(string_key, key, string_len)) {
					return Z_STRVAL_PP(tmp);
				}
			}
			zend_hash_move_forward(Z_ARRVAL_PP(data));
		}
	}

	return NULL;
}
/* }}} */

