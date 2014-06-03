/*
** Copyright (C) 2011 EADS France, Fabrice Desclaux <fabrice.desclaux@eads.net>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License along
** with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include <Python.h>

#include <inttypes.h>
#include <libtcc.h>

#include <stdint.h>



/* tcc global state */
TCCState *tcc_state = NULL;


int include_array_count = 0;
char **include_array = NULL;


int lib_array_count = 0;
char **lib_array = NULL;

//char *libcodenat_path = NULL;


void tcc_init_state(void)
{
	int i;

	tcc_state = tcc_new();
	if (!tcc_state) {
		fprintf(stderr, "Impossible de creer un contexte TCC\n");
		exit(1);
	}
	tcc_set_output_type(tcc_state, TCC_OUTPUT_MEMORY);

	//tcc_add_file(tcc_state, libcodenat_path);
	for (i=0;i<lib_array_count; i++){
		tcc_add_file(tcc_state, lib_array[i]);
	}

	for (i=0;i<include_array_count; i++){
		tcc_add_include_path(tcc_state, include_array[i]);
	}
}




PyObject* tcc_set_emul_lib_path(PyObject* self, PyObject* args)
{
	char* include_arg;
	char* lib_arg;

	char* str1, * str2;

	if (!PyArg_ParseTuple(args, "ss",
			      &include_arg,
			      &lib_arg))
		return NULL;
	/*
	if (include_array)
		free(include_array);
	*/

	str2 = strdup(include_arg);
	while (str2){
		str1 = strsep(&str2, ";");
		if (str1){
			include_array_count ++;
			include_array = realloc(include_array,
						     include_array_count * sizeof(char*));
			include_array[include_array_count-1] = strdup(str1);
			fprintf(stderr, "adding include file: %s\n", str1);
		}
	}


	str2 = strdup(lib_arg);
	while (str2){
		str1 = strsep(&str2, ";");
		if (str1){
			lib_array_count ++;
			lib_array = realloc(lib_array,
						 lib_array_count * sizeof(char*));
			lib_array[lib_array_count-1] = strdup(str1);
			fprintf(stderr, "adding lib file: %s\n", str1);
		}
	}

	/*
	libcodenat_path = (char*)malloc(strlen(libcodenat_path_arg)+1);
	strcpy(libcodenat_path, libcodenat_path_arg);
	*/
	Py_INCREF(Py_None);


	return Py_None;
}


PyObject* tcc_exec_bloc(PyObject* self, PyObject* args)
{
	PyObject* (*func)(void*, void*);
	uint64_t vm;
	uint64_t cpu;
	PyObject* ret;

	if (!PyArg_ParseTuple(args, "KKK", &func, &cpu, &vm))
		return NULL;
	ret = func((void*)cpu, (void*)vm);
	return ret;
}

PyObject* tcc_compil(PyObject* self, PyObject* args)
{
	char* func_name;
	char* func_code;
	int (*entry)(void);

	tcc_init_state();

	if (!PyArg_ParseTuple(args, "ss", &func_name, &func_code))
		return NULL;

	if (tcc_compile_string(tcc_state, func_code) != 0) {
		fprintf(stderr, "Erreur de compilation !\n");
		fprintf(stderr, "%s\n", func_code);
		exit(0);
	}
	/* XXX use tinycc devel with -fPIC patch in makefile */
	if (tcc_relocate(tcc_state, TCC_RELOCATE_AUTO) < 0) {
		fprintf(stderr, "tcc relocate error\n");
		exit(0);
	}
	entry = tcc_get_symbol(tcc_state, func_name);
	if (!entry){
		fprintf(stderr, "Erreur de symbole %s!\n", func_name);
		fprintf(stderr, "%s\n", func_name);
		exit(0);
	}
	return PyLong_FromUnsignedLongLong((uint64_t)entry);

}



PyObject* tcc_loop_exec(PyObject* self, PyObject* args)
{
	//PyObject* (*func)(void*, void*);
	uint64_t* vm;
	uint64_t* cpu;
	PyObject* ret;
	PyObject* func;
	PyObject* pArgs;


	if (!PyArg_ParseTuple(args, "OKK", &func, &cpu, &vm))
		return NULL;

	while (1) {
		if (!PyCallable_Check (func)) {
			fprintf(stderr, "function not callable!\n");
			exit(0);
		}

		pArgs = PyTuple_New(2);
		PyTuple_SetItem(pArgs, 0, PyLong_FromUnsignedLongLong((uint64_t)cpu));
		PyTuple_SetItem(pArgs, 1, PyLong_FromUnsignedLongLong((uint64_t)vm));
		ret = PyObject_CallObject(func, pArgs);
		Py_DECREF(2);

		if (ret == Py_None) {
			Py_INCREF(Py_None);
			return Py_None;
		}
		func = ret;
	}

	return ret;
}



static PyObject *TccError;


static PyMethodDef TccMethods[] = {
    {"tcc_set_emul_lib_path",  tcc_set_emul_lib_path, METH_VARARGS,
     "init tcc path"},
    {"tcc_exec_bloc",  tcc_exec_bloc, METH_VARARGS,
     "tcc exec bloc"},
    {"tcc_compil",  tcc_compil, METH_VARARGS,
     "tcc compil"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC
initJittcc(void)
{
    PyObject *m;

    m = Py_InitModule("Jittcc", TccMethods);
    if (m == NULL)
	    return;

    TccError = PyErr_NewException("tcc.error", NULL, NULL);
    Py_INCREF(TccError);
    PyModule_AddObject(m, "error", TccError);
}

