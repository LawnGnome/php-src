/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2013 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Thies C. Arntzen <thies@thieso.net>                          |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

/* {{{ includes */
#include "zend.h"
#include "zend_API.h"
#include "zend_constants.h"
#include "zend_ini.h"
#include "zend_exceptions.h"
#include "zend_extensions.h"
#include "zend_closures.h"
#include "zend_assert.h"
/* }}} */

static ZEND_FUNCTION(assert);
static ZEND_FUNCTION(assert_options);

#define SAFE_STRING(s) ((s)?(s):"")

static ZEND_INI_MH(OnChangeCallback) /* {{{ */
{
	if (EG(in_execution)) {
		if (ASSERTG(callback)) {
			zval_ptr_dtor(&ASSERTG(callback));
			ASSERTG(callback) = NULL;
		}
		if (new_value && (ASSERTG(callback) || new_value_length)) {
			MAKE_STD_ZVAL(ASSERTG(callback));
			ZVAL_STRINGL(ASSERTG(callback), new_value, new_value_length, 1);
		}
	} else {
		if (ASSERTG(cb)) {
			pefree(ASSERTG(cb), 1);
		}
		if (new_value && new_value_length) {
			ASSERTG(cb) = pemalloc(new_value_length + 1, 1);
			memcpy(ASSERTG(cb), new_value, new_value_length);
			ASSERTG(cb)[new_value_length] = '\0';
		} else {
			ASSERTG(cb) = NULL;
		}
	}
	return SUCCESS;
}
/* }}} */

ZEND_INI_BEGIN()
	 STD_ZEND_INI_ENTRY("assert.active",		"1",	ZEND_INI_ALL,	OnUpdateLong,		active,	 			zend_assert_globals,		assert_globals)
	 STD_ZEND_INI_ENTRY("assert.bail",		"0",	ZEND_INI_ALL,	OnUpdateLong,		bail,	 			zend_assert_globals,		assert_globals)
	 STD_ZEND_INI_ENTRY("assert.warning",	"1",	ZEND_INI_ALL,	OnUpdateLong,		warning, 			zend_assert_globals,		assert_globals)
	 ZEND_INI_ENTRY("assert.callback",		NULL,	ZEND_INI_ALL,	OnChangeCallback)
	 STD_ZEND_INI_ENTRY("assert.exception",		NULL,	ZEND_INI_ALL,	OnUpdateString,				exception_class_name, zend_assert_globals, assert_globals)
	 STD_ZEND_INI_ENTRY("assert.quiet_eval", "0",	ZEND_INI_ALL,	OnUpdateLong,		quiet_eval,		 	zend_assert_globals,		assert_globals)
ZEND_INI_END()

ZEND_BEGIN_ARG_INFO(arginfo_assert, 0)
       ZEND_ARG_INFO(0, assertion)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_assert_options, 0, 0, 1)
       ZEND_ARG_INFO(0, what)
       ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

static const zend_function_entry assert_functions[] = {
  ZEND_FE(assert, arginfo_assert)
  ZEND_FE(assert_options, arginfo_assert_options)
  ZEND_FE_END
};

static void php_assert_init_globals(zend_assert_globals *assert_globals_p TSRMLS_DC) /* {{{ */
{
	assert_globals_p->callback = NULL;
	assert_globals_p->cb = NULL;
	assert_globals_p->exception_class_name = NULL;
}
/* }}} */

ZEND_MINIT_FUNCTION(assert) /* {{{ */
{
	ZEND_INIT_MODULE_GLOBALS(assert, php_assert_init_globals, NULL);

	REGISTER_INI_ENTRIES();

	REGISTER_LONG_CONSTANT("ASSERT_ACTIVE", ASSERT_ACTIVE, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ASSERT_CALLBACK", ASSERT_CALLBACK, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ASSERT_BAIL", ASSERT_BAIL, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ASSERT_WARNING", ASSERT_WARNING, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ASSERT_QUIET_EVAL", ASSERT_QUIET_EVAL, CONST_CS|CONST_PERSISTENT);

	return SUCCESS;
}
/* }}} */

ZEND_MSHUTDOWN_FUNCTION(assert) /* {{{ */
{
	if (ASSERTG(cb)) {
		pefree(ASSERTG(cb), 1);
		ASSERTG(cb) = NULL;
	}
	return SUCCESS;
}
/* }}} */

ZEND_RSHUTDOWN_FUNCTION(assert) /* {{{ */
{
	if (ASSERTG(callback)) {
		zval_ptr_dtor(&ASSERTG(callback));
		ASSERTG(callback) = NULL;
	}

	return SUCCESS;
}
/* }}} */

ZEND_MINFO_FUNCTION(assert) /* {{{ */
{
	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ proto int assert(string|bool assertion[, string description])
   Checks if assertion is false */
ZEND_FUNCTION(assert)
{
	zval **assertion;
	int val, description_len = 0;
	char *myeval = NULL;
	char *compiled_string_description, *description = NULL;

	if (! ASSERTG(active)) {
		RETURN_TRUE;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Z|s", &assertion, &description, &description_len) == FAILURE) {
		return;
	}

	if (Z_TYPE_PP(assertion) == IS_STRING) {
		zval retval;
		int old_error_reporting = 0; /* shut up gcc! */

		myeval = Z_STRVAL_PP(assertion);

		if (ASSERTG(quiet_eval)) {
			old_error_reporting = EG(error_reporting);
			EG(error_reporting) = 0;
		}

		compiled_string_description = zend_make_compiled_string_description("assert code" TSRMLS_CC);
		if (zend_eval_stringl(myeval, Z_STRLEN_PP(assertion), &retval, compiled_string_description TSRMLS_CC) == FAILURE) {
			efree(compiled_string_description);
			if (description_len == 0) {
				zend_error(E_RECOVERABLE_ERROR, "Failure evaluating code: %s%s", "\n", myeval);
			} else {
				zend_error(E_RECOVERABLE_ERROR, "Failure evaluating code: %s%s:\"%s\"", "\n", description, myeval);
			}
			if (ASSERTG(bail)) {
				zend_bailout();
			}
			RETURN_FALSE;
		}
		efree(compiled_string_description);

		if (ASSERTG(quiet_eval)) {
			EG(error_reporting) = old_error_reporting;
		}

		convert_to_boolean(&retval);
		val = Z_LVAL(retval);
	} else {
		convert_to_boolean_ex(assertion);
		val = Z_LVAL_PP(assertion);
	}

	if (val) {
		RETURN_TRUE;
	}

  zend_assert_handle_failure(description, description_len, myeval, NULL TSRMLS_CC);
}
/* }}} */

/* {{{ proto mixed assert_options(int what [, mixed value])
   Set/get the various assert flags */
ZEND_FUNCTION(assert_options)
{
	zval **value = NULL;
	long what;
	int oldint;
	int ac = ZEND_NUM_ARGS();

	if (zend_parse_parameters(ac TSRMLS_CC, "l|Z", &what, &value) == FAILURE) {
		return;
	}

	switch (what) {
	case ASSERT_ACTIVE:
		oldint = ASSERTG(active);
		if (ac == 2) {
			convert_to_string_ex(value);
			zend_alter_ini_entry_ex("assert.active", sizeof("assert.active"), Z_STRVAL_PP(value), Z_STRLEN_PP(value), ZEND_INI_USER, ZEND_INI_STAGE_RUNTIME, 0 TSRMLS_CC);
		}
		RETURN_LONG(oldint);
		break;

	case ASSERT_BAIL:
		oldint = ASSERTG(bail);
		if (ac == 2) {
			convert_to_string_ex(value);
			zend_alter_ini_entry_ex("assert.bail", sizeof("assert.bail"), Z_STRVAL_PP(value), Z_STRLEN_PP(value), ZEND_INI_USER, ZEND_INI_STAGE_RUNTIME, 0 TSRMLS_CC);
		}
		RETURN_LONG(oldint);
		break;

	case ASSERT_QUIET_EVAL:
		oldint = ASSERTG(quiet_eval);
		if (ac == 2) {
			convert_to_string_ex(value);
			zend_alter_ini_entry_ex("assert.quiet_eval", sizeof("assert.quiet_eval"), Z_STRVAL_PP(value), Z_STRLEN_PP(value), ZEND_INI_USER, ZEND_INI_STAGE_RUNTIME, 0 TSRMLS_CC);
		}
		RETURN_LONG(oldint);
		break;

	case ASSERT_WARNING:
		oldint = ASSERTG(warning);
		if (ac == 2) {
			convert_to_string_ex(value);
			zend_alter_ini_entry_ex("assert.warning", sizeof("assert.warning"), Z_STRVAL_PP(value), Z_STRLEN_PP(value), ZEND_INI_USER, ZEND_INI_STAGE_RUNTIME, 0 TSRMLS_CC);
		}
		RETURN_LONG(oldint);
		break;

	case ASSERT_CALLBACK:
		if (ASSERTG(callback) != NULL) {
			RETVAL_ZVAL(ASSERTG(callback), 1, 0);
		} else if (ASSERTG(cb)) {
			RETVAL_STRING(ASSERTG(cb), 1);
		} else {
			RETVAL_NULL();
		}
		if (ac == 2) {
			if (ASSERTG(callback)) {
				zval_ptr_dtor(&ASSERTG(callback));
			}
			ASSERTG(callback) = *value;
			zval_add_ref(value);
		}
		return;
		break;

	default:
		zend_error(E_WARNING, "Unknown value %ld", what);
		break;
	}

	RETURN_FALSE;
}
/* }}} */

zend_module_entry zend_assert_module = { /* {{{ */
  STANDARD_MODULE_HEADER,
	"Assert",
	assert_functions,
	ZEND_MINIT(assert),
  ZEND_MSHUTDOWN(assert),
	NULL,
	ZEND_RSHUTDOWN(assert),
	NULL,
	ZEND_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

void zend_assert_handle_failure(const char *description, int description_len, const char *eval, zend_bool *threw TSRMLS_DC)
{
    if (threw) {
        *threw = 0;
    }

	if (!ASSERTG(callback) && ASSERTG(cb)) {
		MAKE_STD_ZVAL(ASSERTG(callback));
		ZVAL_STRING(ASSERTG(callback), ASSERTG(cb), 1);
	}

	if (ASSERTG(callback)) {
		zval **args = safe_emalloc(description_len == 0 ? 3 : 4, sizeof(zval *), 0);
		zval *retval;
		int i;
		uint lineno = zend_get_executed_lineno(TSRMLS_C);
		const char *filename = zend_get_executed_filename(TSRMLS_C);

		MAKE_STD_ZVAL(args[0]);
		MAKE_STD_ZVAL(args[1]);
		MAKE_STD_ZVAL(args[2]);

		ZVAL_STRING(args[0], SAFE_STRING(filename), 1);
		ZVAL_LONG (args[1], lineno);

        if (eval) {
            ZVAL_STRING(args[2], SAFE_STRING(eval), 1);
        } else {
            ZVAL_NULL(args[2]);
        }

		MAKE_STD_ZVAL(retval);
		ZVAL_FALSE(retval);

		/* XXX do we want to check for error here? */
		if (description_len == 0) {
			call_user_function(CG(function_table), NULL, ASSERTG(callback), retval, 3, args TSRMLS_CC);
			for (i = 0; i <= 2; i++) {
				zval_ptr_dtor(&(args[i]));
			}
		} else {
			MAKE_STD_ZVAL(args[3]);
			ZVAL_STRINGL(args[3], SAFE_STRING(description), description_len, 1);

			call_user_function(CG(function_table), NULL, ASSERTG(callback), retval, 4, args TSRMLS_CC);
			for (i = 0; i <= 3; i++) {
				zval_ptr_dtor(&(args[i]));
			}
		}

		efree(args);
		zval_ptr_dtor(&retval);
	}

	if (ASSERTG(warning)) {
		if (description_len == 0) {
			if (eval) {
				zend_error(E_WARNING, "Assertion \"%s\" failed", eval);
			} else {
				zend_error(E_WARNING, "Assertion failed");
			}
		} else {
			if (eval) {
				zend_error(E_WARNING, "%s: \"%s\" failed", description, eval);
			} else {
				zend_error(E_WARNING, "%s failed", description);
			}
		}
	}

	if (ASSERTG(exception_class_name)) {
		zend_class_entry **ce;
		const char *exception_class_name = ASSERTG(exception_class_name);

		if (zend_lookup_class(exception_class_name, strlen(exception_class_name), &ce TSRMLS_CC) == FAILURE) {
			zend_error(E_WARNING, "Cannot throw exception for failed assertion: class \"%s\" is not defined", exception_class_name);
		} else {
			zend_class_entry *expectation_ce = zend_get_expectation_exception(TSRMLS_C);

			if (!instanceof_function(*ce, expectation_ce)) {
				zend_error(E_WARNING, "Cannot throw exception for failed assertion: class \"%s\" does not inherit from %s", exception_class_name, expectation_ce->name);
			} else {
				zend_throw_exception(*ce, description, 0 TSRMLS_CC);
				if (threw) {
					*threw = 1;
				}
			}
		}
	}

	if (ASSERTG(bail)) {
		zend_bailout();
	}
}

int zend_startup_assert(TSRMLS_D) /* {{{ */
{
	zend_assert_module.module_number = zend_next_free_module();
	zend_assert_module.type = MODULE_PERSISTENT;
	return (EG(current_module) = zend_register_module_ex(&zend_assert_module TSRMLS_CC)) == NULL ? FAILURE : SUCCESS;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker noet
 * vim<600: sw=4 ts=4
 */
