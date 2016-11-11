/**
 * markupsafe._speedups
 * ~~~~~~~~~~~~~~~~~~~~
 *
 * This module implements functions for automatic escaping in C for better
 * performance.
 *
 * :copyright: (c) 2010 by Armin Ronacher.
 * :license: BSD.
 *
 * :adapted to Genshi needs by: Lele Gaifax <lele@metapensiero.it>
 */

#include <Python.h>

#define ESCAPED_CHARS_TABLE_SIZE 63
#define UNICHR(x) (PyUnicode_AS_UNICODE((PyUnicodeObject*)PyUnicode_DecodeASCII(x, strlen(x), NULL)));


static PyObject* markup;
static Py_ssize_t escaped_chars_delta_len[ESCAPED_CHARS_TABLE_SIZE];
static Py_UNICODE *escaped_chars_repl[ESCAPED_CHARS_TABLE_SIZE];

static int
init_constants(void)
{
    PyObject *module;
    /* mapping of characters to replace */
    escaped_chars_repl['"'] = UNICHR("&#34;");
    escaped_chars_repl['\''] = UNICHR("&#39;");
    escaped_chars_repl['&'] = UNICHR("&amp;");
    escaped_chars_repl['<'] = UNICHR("&lt;");
    escaped_chars_repl['>'] = UNICHR("&gt;");

    /* lengths of those characters when replaced - 1 */
    memset(escaped_chars_delta_len, 0, sizeof (escaped_chars_delta_len));
    escaped_chars_delta_len['"'] = escaped_chars_delta_len['\''] = \
        escaped_chars_delta_len['&'] = 4;
    escaped_chars_delta_len['<'] = escaped_chars_delta_len['>'] = 3;

    /* import markup type so that we can mark the return value */
    module = PyImport_ImportModule("genshi.core");
    if (!module)
        return 0;
    markup = PyObject_GetAttrString(module, "Markup");
    Py_DECREF(module);

    return 1;
}

static PyObject*
escape_unicode(PyUnicodeObject *in, int quotes)
{
    PyUnicodeObject *out;
    Py_UNICODE *inp = PyUnicode_AS_UNICODE(in);
    const Py_UNICODE *inp_end = PyUnicode_AS_UNICODE(in) + PyUnicode_GET_SIZE(in);
    Py_UNICODE *next_escp;
    Py_UNICODE *outp;
    Py_ssize_t delta=0, erepl=0, delta_len=0;

    /* First we need to figure out how long the escaped string will be */
    while (*(inp) || inp < inp_end) {
        if (*inp < ESCAPED_CHARS_TABLE_SIZE && (quotes || (*inp != '"' && *inp != '\''))) {
            delta += escaped_chars_delta_len[*inp];
            erepl += !!escaped_chars_delta_len[*inp];
        }
        ++inp;
    }

    /* Do we need to escape anything at all? */
    if (!erepl) {
        Py_INCREF(in);
        return (PyObject*)in;
    }

    out = (PyUnicodeObject*)PyUnicode_FromUnicode(NULL, PyUnicode_GET_SIZE(in) + delta);
    if (!out)
        return NULL;

    outp = PyUnicode_AS_UNICODE(out);
    inp = PyUnicode_AS_UNICODE(in);
    while (erepl-- > 0) {
        /* look for the next substitution */
        next_escp = inp;
        while (next_escp < inp_end) {
            if (*next_escp < ESCAPED_CHARS_TABLE_SIZE &&
                (quotes || (*next_escp != '"' && *next_escp != '\'')) &&
                (delta_len = escaped_chars_delta_len[*next_escp])) {
                ++delta_len;
                break;
            }
            ++next_escp;
        }

        if (next_escp > inp) {
            /* copy unescaped chars between inp and next_escp */
            Py_UNICODE_COPY(outp, inp, next_escp-inp);
            outp += next_escp - inp;
        }

        /* escape 'next_escp' */
        Py_UNICODE_COPY(outp, escaped_chars_repl[*next_escp], delta_len);
        outp += delta_len;

        inp = next_escp + 1;
    }
    if (inp < inp_end)
        Py_UNICODE_COPY(outp, inp, PyUnicode_GET_SIZE(in) - (inp - PyUnicode_AS_UNICODE(in)));

    return (PyObject*)out;
}


static PyObject*
escape(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"text", "quotes", NULL};
    PyObject *text;
    int quotes = 1;
    PyObject *s = NULL, *rv = NULL, *html;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i", kwlist,
                                     &text, &quotes))
        return NULL;

    if (PyObject_TypeCheck(text, (PyTypeObject *)markup)) {
        Py_INCREF(text);
        return text;
    }

    /* we don't have to escape integers, bools or floats */
    if (PyLong_CheckExact(text) ||
        PyFloat_CheckExact(text) || PyBool_Check(text) ||
        text == Py_None)
        return PyObject_CallFunctionObjArgs(markup, text, NULL);

    /* if the object has an __html__ method that performs the escaping */
    html = PyObject_GetAttrString(text, "__html__");
    if (html) {
        rv = PyObject_CallObject(html, NULL);
        Py_DECREF(html);
        return rv;
    }

    /* otherwise make the object unicode if it isn't, then escape */
    PyErr_Clear();
    if (!PyUnicode_Check(text)) {
        PyObject *unicode = PyObject_Str(text);
        if (!unicode)
            return NULL;
        s = escape_unicode((PyUnicodeObject*)unicode, quotes);
        Py_DECREF(unicode);
    }
    else
        s = escape_unicode((PyUnicodeObject*)text, quotes);

    /* convert the unicode string into a markup object. */
    rv = PyObject_CallFunctionObjArgs(markup, (PyObject*)s, NULL);
    Py_DECREF(s);
    return rv;
}

static PyMethodDef module_methods[] = {
    {"escape", (PyCFunction)escape, METH_VARARGS | METH_KEYWORDS,
     "escape(text, quotes=True) -> markup\n\n"
     "Convert the characters &, <, >, ', and \" (latter two only when quotes\n"
     "is true) in string text to HTML-safe sequences.  Use this if you need\n"
     "to display text that might contain such characters in HTML.\n"
     "Marks return value as markup string."},
    {NULL, NULL, 0, NULL}		/* Sentinel */
};


static struct PyModuleDef module_definition = {
    PyModuleDef_HEAD_INIT,
    "genshi._speedups",
    NULL,
    -1,
    module_methods,
    NULL,
    NULL,
    NULL,
    NULL
};

PyMODINIT_FUNC
PyInit__speedups(void)
{
    if (!init_constants())
        return NULL;

    return PyModule_Create(&module_definition);
}
