#ifndef ZEND_ASSERT_H
#define ZEND_ASSERT_H

ZEND_BEGIN_MODULE_GLOBALS(assert)
	long active;
	long bail;
	long warning;
	long quiet_eval;
	zval *callback;
	char *cb;
  char *exception_class_name;
ZEND_END_MODULE_GLOBALS(assert)

ZEND_DECLARE_MODULE_GLOBALS(assert)

#ifdef ZTS
#define ASSERTG(v) TSRMG(assert_globals_id, zend_assert_globals *, v)
#else
#define ASSERTG(v) (assert_globals.v)
#endif

enum {
	ASSERT_ACTIVE=1,
	ASSERT_CALLBACK,
	ASSERT_BAIL,
	ASSERT_WARNING,
	ASSERT_QUIET_EVAL
};

int zend_startup_assert(TSRMLS_D);
void zend_assert_handle_failure(const char *description, int description_len, const char *eval, zend_bool *threw TSRMLS_DC);

#endif
