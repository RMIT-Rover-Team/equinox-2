#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "torque_handler.hpp" 

//The internals of the python class
typedef struct {
    PyObject_HEAD
    TorqueHandler* handler;
} PyTorqueHandler;

// Deallocation
static void PyTorqueHandler_dealloc(PyTorqueHandler* self)
{
    delete self->handler;
    Py_TYPE(self)->tp_free((PyObject*)self);
}

// __new__
static PyObject* PyTorqueHandler_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    PyTorqueHandler* self;
    self = (PyTorqueHandler*)type->tp_alloc(type, 0);
    if (!self)
        return nullptr;
    self->handler = nullptr;
    return (PyObject*)self;
}

// __init__(self, can_interface: str)
static int PyTorqueHandler_init(PyTorqueHandler* self, PyObject* args, PyObject* kwds)
{
    const char* can_interface = nullptr;
    static const char* kwlist[] = {"can_interface", nullptr};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", (char**)kwlist, &can_interface)) {
        return -1;
    }

    try {
        self->handler = new TorqueHandler(can_interface);
    } catch (...) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to create TorqueHandler");
        return -1;
    }

    return 0;
}

// set_speed(self, left: float, right: float)
static PyObject* PyTorqueHandler_set_speed(PyTorqueHandler* self, PyObject* args)
{
    float left, right;
    if (!PyArg_ParseTuple(args, "ff", &left, &right))
        return nullptr;

    self->handler->setSpeed(left, right);
    Py_RETURN_NONE;
}

// calibrate(self)
static PyObject* PyTorqueHandler_calibrate(PyTorqueHandler* self, PyObject* Py_UNUSED(ignored))
{
    self->handler->calibrate();
    Py_RETURN_NONE;
}

// enable(self)
static PyObject* PyTorqueHandler_enable(PyTorqueHandler* self, PyObject* Py_UNUSED(ignored))
{
    self->handler->enable();
    Py_RETURN_NONE;
}

// disable(self)
static PyObject* PyTorqueHandler_disable(PyTorqueHandler* self, PyObject* Py_UNUSED(ignored))
{
    self->handler->disable();
    Py_RETURN_NONE;
}

// estop(self)
static PyObject* PyTorqueHandler_estop(PyTorqueHandler* self, PyObject* Py_UNUSED(ignored))
{
    self->handler->estop();
    Py_RETURN_NONE;
}

// set_mode(self, mode: int)
static PyObject* PyTorqueHandler_set_mode(PyTorqueHandler* self, PyObject* args)
{
    int mode_int;
    if (!PyArg_ParseTuple(args, "i", &mode_int))
        return nullptr;

    auto mode = static_cast<TorqueDriveMode>(mode_int);
    self->handler->setMode(mode);
    Py_RETURN_NONE;
}

// get_odom(self) -> (leftPos, rightPos, leftSpeed, rightSpeed)
static PyObject* PyTorqueHandler_get_odom(PyTorqueHandler* self, PyObject* Py_UNUSED(ignored))
{
    OdomReading odom = self->handler->getOdom();
    return Py_BuildValue("ffff",
                         odom.leftPos,
                         odom.rightPos,
                         odom.leftSpeed,
                         odom.rightSpeed);
}

// Methods table
static PyMethodDef PyTorqueHandler_methods[] = {
    {"set_speed", (PyCFunction)PyTorqueHandler_set_speed, METH_VARARGS,
     "Set left and right wheel speeds"},
    {"calibrate", (PyCFunction)PyTorqueHandler_calibrate, METH_NOARGS,
     "Calibrate motors"},
    {"enable", (PyCFunction)PyTorqueHandler_enable, METH_NOARGS,
     "Enable motors"},
    {"disable", (PyCFunction)PyTorqueHandler_disable, METH_NOARGS,
     "Disable motors"},
    {"estop", (PyCFunction)PyTorqueHandler_estop, METH_NOARGS,
     "Emergency stop"},
    {"set_mode", (PyCFunction)PyTorqueHandler_set_mode, METH_VARARGS,
     "Set drive mode (int enum)"},
    {"get_odom", (PyCFunction)PyTorqueHandler_get_odom, METH_NOARGS,
     "Get odometry reading as (leftPos, rightPos, leftSpeed, rightSpeed)"},
    {nullptr, nullptr, 0, nullptr}
};

// Type object
static PyTypeObject PyTorqueHandlerType = {
    PyVarObject_HEAD_INIT(nullptr, 0)
    "torque.TorqueHandler",          /* tp_name */
    sizeof(PyTorqueHandler),         /* tp_basicsize */
    0,                               /* tp_itemsize */
    (destructor)PyTorqueHandler_dealloc, /* tp_dealloc */
    0,                               /* tp_vectorcall_offset / tp_print (old) */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_as_async / tp_reserved */
    0,                               /* tp_repr */
    0,                               /* tp_as_number */
    0,                               /* tp_as_sequence */
    0,                               /* tp_as_mapping */
    0,                               /* tp_hash  */
    0,                               /* tp_call */
    0,                               /* tp_str */
    0,                               /* tp_getattro */
    0,                               /* tp_setattro */
    0,                               /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE,             /* tp_flags */
    "TorqueHandler objects",         /* tp_doc */
    0,                               /* tp_traverse */
    0,                               /* tp_clear */
    0,                               /* tp_richcompare */
    0,                               /* tp_weaklistoffset */
    0,                               /* tp_iter */
    0,                               /* tp_iternext */
    PyTorqueHandler_methods,         /* tp_methods */
    0,                               /* tp_members */
    0,                               /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    (initproc)PyTorqueHandler_init,  /* tp_init */
    0,                               /* tp_alloc */
    PyTorqueHandler_new,             /* tp_new */
};

// --- Module definition ---

static PyModuleDef torque_module = {
    PyModuleDef_HEAD_INIT,
    "torque",
    "Python bindings for TorqueHandler",
    -1,
    nullptr, nullptr, nullptr, nullptr, nullptr
};

PyMODINIT_FUNC PyInit_torque(void)
{
    if (PyType_Ready(&PyTorqueHandlerType) < 0)
        return nullptr;

    PyObject* m = PyModule_Create(&torque_module);
    if (!m)
        return nullptr;

    Py_INCREF(&PyTorqueHandlerType);
    if (PyModule_AddObject(m, "TorqueHandler",
                           (PyObject*)&PyTorqueHandlerType) < 0) {
        Py_DECREF(&PyTorqueHandlerType);
        Py_DECREF(m);
        return nullptr;
    }

    // Expose enum values as module-level constants
    PyModule_AddIntConstant(m, "UNLOCKED_VELOCITY", (int)UNLOCKED_VELOCITY);
    PyModule_AddIntConstant(m, "UNLOCKED_TORQUE",   (int)UNLOCKED_TORQUE);
    PyModule_AddIntConstant(m, "LOCKED_VELOCITY",   (int)LOCKED_VELOCITY);

    return m;
}
