#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <errno.h>

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v, const char* l, int t) {
    int i = 0;
    while (l[i] != '\0') {
        if (c->json[i] != l[i])
            return LEPT_PARSE_INVALID_VALUE;
        i++;
    }
    c->json += i;
    v->type = t;
	return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
    const char* num = c->json;
    if (*num == '-')
        num++;
    if (*num == '0')
        num++;
    else if (ISDIGIT1TO9(*num)) {
        num++;
        while (ISDIGIT(*num))
            num++;
    }
    else {
        num++;
        goto INVALID;
    }

    if (*num == '.') {
        num++;
        if (ISDIGIT(*num)) {
            num++;
            while (ISDIGIT(*num))
                num++;
        }
        else
            goto INVALID;
    }
    if (*num == 'e' || *num == 'E') {
        num++;
        if (*num == '+' || *num == '-' || ISDIGIT(*num)) {
            num++;
            while (ISDIGIT(*num))
                num++;
        }
    }

//NUMBER:
    if (*num == '\0') {
        v->n = strtod(c->json, NULL);
        //if (c->json == end)
        //    return LEPT_PARSE_INVALID_VALUE;
        if (errno == ERANGE)
        {
            errno = 0;
			//c->json = num;
			v->type = LEPT_NULL;
			return LEPT_PARSE_NUMBER_TOO_BIG;
        }
        c->json = num;
        v->type = LEPT_NUMBER;
        return LEPT_PARSE_OK;
    }

	v->n = 0;
	v->type = LEPT_NULL;
	return LEPT_PARSE_ROOT_NOT_SINGULAR;

INVALID:
    v->n = 0;
    v->type = LEPT_NULL;
    return LEPT_PARSE_INVALID_VALUE;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':  return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 'n':  return lept_parse_literal(c, v, "null", LEPT_NULL);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
