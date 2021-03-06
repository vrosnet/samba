/* 
   Unix SMB/CIFS implementation.
   Samba utility functions
   Copyright (C) Jelmer Vernooij <jelmer@samba.org> 2007-2008
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>
#include <stdbool.h>

#include "includes.h"
#include "param/param.h"
#include "param/loadparm.h"
#include <Python.h>
#include "pytalloc.h"

/* There's no Py_ssize_t in 2.4, apparently */
#if PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION < 5
typedef int Py_ssize_t;
typedef inquiry lenfunc;
#endif

#ifndef Py_RETURN_NONE
#define Py_RETURN_NONE return Py_INCREF(Py_None), Py_None
#endif

#define PyLoadparmContext_AsLoadparmContext(obj) py_talloc_get_type(obj, struct loadparm_context)

PyAPI_DATA(PyTypeObject) PyLoadparmContext;
PyAPI_DATA(PyTypeObject) PyLoadparmService;

PyObject *PyLoadparmService_FromService(struct loadparm_service *service)
{
	return py_talloc_import(&PyLoadparmService, service);
}

static PyObject *py_lp_ctx_get_helper(struct loadparm_context *lp_ctx, const char *service_name, const char *param_name)
{
    struct parm_struct *parm = NULL;
    void *parm_ptr = NULL;
    int i;

    if (service_name != NULL) {
	struct loadparm_service *service;
	/* its a share parameter */
	service = lp_service(lp_ctx, service_name);
	if (service == NULL) {
	    return NULL;
	}
	if (strchr(param_name, ':')) {
	    /* its a parametric option on a share */
	    const char *type = talloc_strndup(lp_ctx, 
			      param_name, 
			      strcspn(param_name, ":"));
	    const char *option = strchr(param_name, ':') + 1;
	    const char *value;
	    if (type == NULL || option == NULL) {
		return NULL;
	    }
	    value = lp_get_parametric(lp_ctx, service, type, option);
	    if (value == NULL) {
		return NULL;
	    }
	    return PyString_FromString(value);
	}

	parm = lp_parm_struct(param_name);
	if (parm == NULL || parm->pclass == P_GLOBAL) {
	    return NULL;
	}
	parm_ptr = lp_parm_ptr(lp_ctx, service, parm);
    } else if (strchr(param_name, ':')) {
	/* its a global parametric option */
	const char *type = talloc_strndup(lp_ctx, 
			  param_name, strcspn(param_name, ":"));
	const char *option = strchr(param_name, ':') + 1;
	const char *value;
	if (type == NULL || option == NULL) {
	    return NULL;
	}
	value = lp_get_parametric(lp_ctx, NULL, type, option);
	if (value == NULL)
	    return NULL;
	return PyString_FromString(value);
    } else {
	/* its a global parameter */
	parm = lp_parm_struct(param_name);
	if (parm == NULL) {
	    return NULL;
	}
	parm_ptr = lp_parm_ptr(lp_ctx, NULL, parm);
    }

    if (parm == NULL || parm_ptr == NULL) {
	return NULL;
    }

    /* construct and return the right type of python object */
    switch (parm->type) {
    case P_STRING:
    case P_USTRING:
	return PyString_FromString(*(char **)parm_ptr);
    case P_BOOL:
	return PyBool_FromLong(*(bool *)parm_ptr);
    case P_INTEGER:
    case P_OCTAL:
    case P_BYTES:
	return PyLong_FromLong(*(int *)parm_ptr);
    case P_ENUM:
	for (i=0; parm->enum_list[i].name; i++) {
	    if (*(int *)parm_ptr == parm->enum_list[i].value) {
		return PyString_FromString(parm->enum_list[i].name);
	    }
	}
	return NULL;
    case P_LIST: 
	{
	    int j;
	    const char **strlist = *(const char ***)parm_ptr;
	    PyObject *pylist = PyList_New(str_list_length(strlist));
	    for (j = 0; strlist[j]; j++) 
		PyList_SetItem(pylist, j, 
			       PyString_FromString(strlist[j]));
	    return pylist;
	}

	break;
    }
    return NULL;

}

static PyObject *py_lp_ctx_load(py_talloc_Object *self, PyObject *args)
{
	char *filename;
	bool ret;
	if (!PyArg_ParseTuple(args, "s", &filename))
		return NULL;

	ret = lp_load(PyLoadparmContext_AsLoadparmContext(self), filename);

	if (!ret) {
		PyErr_Format(PyExc_RuntimeError, "Unable to load file %s", filename);
		return NULL;
	}
	Py_RETURN_NONE;
}

static PyObject *py_lp_ctx_load_default(py_talloc_Object *self)
{
	bool ret;
        ret = lp_load_default(PyLoadparmContext_AsLoadparmContext(self));

	if (!ret) {
		PyErr_SetString(PyExc_RuntimeError, "Unable to load default file");
		return NULL;
	}
	Py_RETURN_NONE;
}

static PyObject *py_lp_ctx_get(py_talloc_Object *self, PyObject *args)
{
	char *param_name;
	char *section_name = NULL;
	PyObject *ret;
	if (!PyArg_ParseTuple(args, "s|s", &param_name, &section_name))
		return NULL;

	ret = py_lp_ctx_get_helper(PyLoadparmContext_AsLoadparmContext(self), section_name, param_name);
	if (ret == NULL)
		Py_RETURN_NONE;
	return ret;
}

static PyObject *py_lp_ctx_is_myname(py_talloc_Object *self, PyObject *args)
{
	char *name;
	if (!PyArg_ParseTuple(args, "s", &name))
		return NULL;

	return PyBool_FromLong(lp_is_myname(PyLoadparmContext_AsLoadparmContext(self), name));
}

static PyObject *py_lp_ctx_is_mydomain(py_talloc_Object *self, PyObject *args)
{
	char *name;
	if (!PyArg_ParseTuple(args, "s", &name))
		return NULL;

	return PyBool_FromLong(lp_is_mydomain(PyLoadparmContext_AsLoadparmContext(self), name));
}

static PyObject *py_lp_ctx_set(py_talloc_Object *self, PyObject *args)
{
	char *name, *value;
	bool ret;
	if (!PyArg_ParseTuple(args, "ss", &name, &value))
		return NULL;

	ret = lp_set_cmdline(PyLoadparmContext_AsLoadparmContext(self), name, value);
	if (!ret) {
		PyErr_SetString(PyExc_RuntimeError, "Unable to set parameter");
		return NULL;
        }

	Py_RETURN_NONE;
}

static PyObject *py_lp_ctx_private_path(py_talloc_Object *self, PyObject *args)
{
	char *name, *path;
	PyObject *ret;
	if (!PyArg_ParseTuple(args, "s", &name))
		return NULL;

	path = private_path(NULL, PyLoadparmContext_AsLoadparmContext(self), name);
	ret = PyString_FromString(path);
	talloc_free(path);

	return ret;
}

static PyMethodDef py_lp_ctx_methods[] = {
	{ "load", (PyCFunction)py_lp_ctx_load, METH_VARARGS, 
		"S.load(filename) -> None\n"
		"Load specified file." },
	{ "load_default", (PyCFunction)py_lp_ctx_load_default, METH_NOARGS,
        	"S.load_default() -> None\n"
		"Load default smb.conf file." },
	{ "is_myname", (PyCFunction)py_lp_ctx_is_myname, METH_VARARGS,
		"S.is_myname(name) -> bool\n"
		"Check whether the specified name matches one of our netbios names." },
	{ "is_mydomain", (PyCFunction)py_lp_ctx_is_mydomain, METH_VARARGS,
		"S.is_mydomain(name) -> bool\n"
		"Check whether the specified name matches our domain name." },
	{ "get", (PyCFunction)py_lp_ctx_get, METH_VARARGS,
        	"S.get(name, service_name) -> value\n"
		"Find specified parameter." },
	{ "set", (PyCFunction)py_lp_ctx_set, METH_VARARGS,
		"S.set(name, value) -> bool\n"
		"Change a parameter." },
	{ "private_path", (PyCFunction)py_lp_ctx_private_path, METH_VARARGS,
		"S.private_path(name) -> path\n" },
	{ NULL }
};

static PyObject *py_lp_ctx_default_service(py_talloc_Object *self, void *closure)
{
	return PyLoadparmService_FromService(lp_default_service(PyLoadparmContext_AsLoadparmContext(self)));
}

static PyObject *py_lp_ctx_config_file(py_talloc_Object *self, void *closure)
{
	const char *configfile = lp_configfile(PyLoadparmContext_AsLoadparmContext(self));
	if (configfile == NULL)
		Py_RETURN_NONE;
	else
		return PyString_FromString(configfile);
}

static PyGetSetDef py_lp_ctx_getset[] = {
	{ discard_const_p(char, "default_service"), (getter)py_lp_ctx_default_service, NULL, NULL },
	{ discard_const_p(char, "configfile"), (getter)py_lp_ctx_config_file, NULL,
	  discard_const_p(char, "Name of last config file that was loaded.") },
	{ NULL }
};

static PyObject *py_lp_ctx_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
	return py_talloc_import(type, loadparm_init(NULL));
}

static Py_ssize_t py_lp_ctx_len(py_talloc_Object *self)
{
	return lp_numservices(PyLoadparmContext_AsLoadparmContext(self));
}

static PyObject *py_lp_ctx_getitem(py_talloc_Object *self, PyObject *name)
{
	struct loadparm_service *service;
	if (!PyString_Check(name)) {
		PyErr_SetString(PyExc_TypeError, "Only string subscripts are supported");
		return NULL;
	}
	service = lp_service(PyLoadparmContext_AsLoadparmContext(self), PyString_AsString(name));
	if (service == NULL) {
		PyErr_SetString(PyExc_KeyError, "No such section");
		return NULL;
	}
	return PyLoadparmService_FromService(service);
}

static PyMappingMethods py_lp_ctx_mapping = {
	.mp_length = (lenfunc)py_lp_ctx_len,
	.mp_subscript = (binaryfunc)py_lp_ctx_getitem,
};

PyTypeObject PyLoadparmContext = {
	.tp_name = "LoadParm",
	.tp_basicsize = sizeof(py_talloc_Object),
	.tp_dealloc = py_talloc_dealloc,
	.tp_getset = py_lp_ctx_getset,
	.tp_methods = py_lp_ctx_methods,
	.tp_new = py_lp_ctx_new,
	.tp_as_mapping = &py_lp_ctx_mapping,
	.tp_flags = Py_TPFLAGS_DEFAULT,
};

PyTypeObject PyLoadparmService = {
	.tp_name = "LoadparmService",
	.tp_dealloc = py_talloc_dealloc,
	.tp_basicsize = sizeof(py_talloc_Object),
	.tp_flags = Py_TPFLAGS_DEFAULT,
};

_PUBLIC_ struct loadparm_context *lp_from_py_object(PyObject *py_obj)
{
    struct loadparm_context *lp_ctx;
    if (PyString_Check(py_obj)) {
        lp_ctx = loadparm_init(NULL);
        if (!lp_load(lp_ctx, PyString_AsString(py_obj))) {
            talloc_free(lp_ctx);
	    PyErr_Format(PyExc_RuntimeError, 
			 "Unable to load %s", PyString_AsString(py_obj));
            return NULL;
        }
        return lp_ctx;
    }

    if (py_obj == Py_None) {
        lp_ctx = loadparm_init(NULL);
	/* We're not checking that loading the file succeeded *on purpose */
        lp_load_default(lp_ctx);
        return lp_ctx;
    }

    return PyLoadparmContext_AsLoadparmContext(py_obj);
}

struct loadparm_context *py_default_loadparm_context(TALLOC_CTX *mem_ctx)
{
    struct loadparm_context *ret;
    ret = loadparm_init(mem_ctx);
    if (!lp_load_default(ret))
        return NULL;
    return ret;
}

static PyObject *py_default_path(PyObject *self)
{
    return PyString_FromString(lp_default_path());
}

static PyMethodDef pyparam_methods[] = {
    { "default_path", (PyCFunction)py_default_path, METH_NOARGS, 
        "Returns the default smb.conf path." },
    { NULL }
};

void initparam(void)
{
	PyObject *m;

	if (PyType_Ready(&PyLoadparmContext) < 0)
		return;

	m = Py_InitModule3("param", pyparam_methods, "Parsing and writing Samba configuration files.");
	if (m == NULL)
		return;

	Py_INCREF(&PyLoadparmContext);
	PyModule_AddObject(m, "LoadParm", (PyObject *)&PyLoadparmContext);
}
