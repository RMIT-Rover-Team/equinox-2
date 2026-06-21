#include <Python.h>
#include "RoverCanMaster.h"
#include "GenericCan.h"
#include "SocketCanWrapper.h"

// Python object to hold RoverHandle
typedef struct {
    PyObject_HEAD
    GenericCan* mycan;
    RoverCanMaster* mymaster;
} PyRoverObject;

// -----------------------------
// Helpers: Convert C++ structs
// -----------------------------
static PyObject* ReceivedState_to_Py(const ReceivedState& st) {
    return Py_BuildValue(
        "{s:i,s:i,s:i}",
        "motor_id", st.motor_id,
        "error_flag", st.error_flag,
        "uncalibrated_flag", st.uncallibrated_flag
    );
}

static PyObject* Datapoint_to_Py(const Datapoint& dp) {
    return Py_BuildValue(
        "{s:i,s:i,s:i}",
        "from", dp.from,
        "stream_id", dp.stream_id,
        "channel_id", dp.channel_id
    );
}

// -----------------------------
// __new__
// -----------------------------
static PyObject* PyRover_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    PyRoverObject* self = (PyRoverObject*)type->tp_alloc(type, 0);
    if (self) {
        self->mycan = nullptr;
        self->mymaster = nullptr;
    }
    return (PyObject*)self;
}

// -----------------------------
// __init__
// -----------------------------
static int PyRover_init(PyRoverObject* self, PyObject* args, PyObject* kwds) {
    const char* can_interface = nullptr;
    int node_id = 0;

    static const char* kwlist[] = {"can_interface", "node_id", nullptr};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "si", (char**)kwlist,
                                     &can_interface, &node_id)) {
        return -1;
    }

    try {
        self->mycan = new WrappedCANBus(can_interface);
        self->mymaster = new RoverCanMaster(*self->mycan, (uint8_t)node_id);
    } catch (...) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to create RoverCanMaster");
        return -1;
    }

    return 0;
}

// -----------------------------
// Destructor
// -----------------------------
static void PyRover_dealloc(PyRoverObject* self) {
    delete self->mymaster;
    delete self->mycan;
    Py_TYPE(self)->tp_free((PyObject*)self);
}

// -----------------------------
// Method wrappers
// -----------------------------
static PyObject* PyRover_EStop(PyRoverObject* self, PyObject* args) {
    int destID;
    if (!PyArg_ParseTuple(args, "i", &destID))
        return nullptr;

    bool ok = self->mymaster->EStop(destID);
    return PyBool_FromLong(ok);
}

static PyObject* PyRover_Calibrate(PyRoverObject* self, PyObject* args) {
    int destID, motor_id;
    if (!PyArg_ParseTuple(args, "ii", &destID, &motor_id))
        return nullptr;

    bool ok = self->mymaster->Calibrate(destID, motor_id);
    return PyBool_FromLong(ok);
}

static PyObject* PyRover_SetMotorPosition(PyRoverObject* self, PyObject* args) {
    int destID, motor_id;
    float position;
    if (!PyArg_ParseTuple(args, "iif", &destID, &motor_id, &position))
        return nullptr;

    ReceivedState st = self->mymaster->SetMotorPosition(destID, motor_id, position);
    return ReceivedState_to_Py(st);
}

static PyObject* PyRover_SetMotorSpeed(PyRoverObject* self, PyObject* args) {
    int destID, motor_id;
    float speed;
    if (!PyArg_ParseTuple(args, "iif", &destID, &motor_id, &speed))
        return nullptr;

    ReceivedState st = self->mymaster->SetMotorSpeed(destID, motor_id, speed);
    return ReceivedState_to_Py(st);
}

static PyObject* PyRover_ToggleState(PyRoverObject* self, PyObject* args) {
    int destID, motor_id, toggle;
    if (!PyArg_ParseTuple(args, "iii", &destID, &motor_id, &toggle))
        return nullptr;

    ReceivedState st = self->mymaster->ToggleState(destID, motor_id, toggle != 0);
    return ReceivedState_to_Py(st);
}

static PyObject* PyRover_GetMotorPosition(PyRoverObject* self, PyObject* args) {
    int destID, motor_id;
    if (!PyArg_ParseTuple(args, "ii", &destID, &motor_id))
        return nullptr;

    auto result = self->mymaster->GetMotorPosition(destID, motor_id);
    return PyTuple_Pack(2,
                        ReceivedState_to_Py(result.first),
                        PyFloat_FromDouble(result.second));
}

static PyObject* PyRover_GetMotorSpeed(PyRoverObject* self, PyObject* args) {
    int destID, motor_id;
    if (!PyArg_ParseTuple(args, "ii", &destID, &motor_id))
        return nullptr;

    auto result = self->mymaster->GetMotorSpeed(destID, motor_id);
    return PyTuple_Pack(2,
                        ReceivedState_to_Py(result.first),
                        PyFloat_FromDouble(result.second));
}

static PyObject* PyRover_BroadcastDataPoint(PyRoverObject* self, PyObject* args) {
    auto result = self->mymaster->BroadcastDataPoint();
    return PyTuple_Pack(2,
                        Datapoint_to_Py(result.first),
                        PyFloat_FromDouble(result.second));
}

static PyObject* PyRover_RequestDataPoint(PyRoverObject* self, PyObject* args) {
    int destID, stream_id, channel_id;
    if (!PyArg_ParseTuple(args, "iii", &destID, &stream_id, &channel_id))
        return nullptr;

    auto result = self->mymaster->RequestDataPoint(destID, stream_id, channel_id);
    return PyTuple_Pack(2,
                        Datapoint_to_Py(result.first),
                        PyFloat_FromDouble(result.second));
}

static PyObject* PyRover_ping(PyRoverObject* self, PyObject* args) {
    int destID;
    if (!PyArg_ParseTuple(args, "i", &destID))
        return nullptr;

    bool ok = self->mymaster->ping(destID);
    return PyBool_FromLong(ok);
}

// -----------------------------
// Method table
// -----------------------------
static PyMethodDef PyRover_methods[] = {
    {"EStop", (PyCFunction)PyRover_EStop, METH_VARARGS, "Emergency stop"},
    {"Calibrate", (PyCFunction)PyRover_Calibrate, METH_VARARGS, "Calibrate motor"},
    {"SetMotorPosition", (PyCFunction)PyRover_SetMotorPosition, METH_VARARGS, "Set motor position"},
    {"SetMotorSpeed", (PyCFunction)PyRover_SetMotorSpeed, METH_VARARGS, "Set motor speed"},
    {"ToggleState", (PyCFunction)PyRover_ToggleState, METH_VARARGS, "Toggle motor state"},
    {"GetMotorPosition", (PyCFunction)PyRover_GetMotorPosition, METH_VARARGS, "Get motor position"},
    {"GetMotorSpeed", (PyCFunction)PyRover_GetMotorSpeed, METH_VARARGS, "Get motor speed"},
    {"BroadcastDataPoint", (PyCFunction)PyRover_BroadcastDataPoint, METH_NOARGS, "Receive broadcast datapoint"},
    {"RequestDataPoint", (PyCFunction)PyRover_RequestDataPoint, METH_VARARGS, "Request datapoint"},
    {"ping", (PyCFunction)PyRover_ping, METH_VARARGS, "Ping device"},
    {nullptr, nullptr, 0, nullptr}
};

// -----------------------------
// Type object
// -----------------------------
static PyTypeObject PyRoverType = {
    PyVarObject_HEAD_INIT(nullptr, 0)
    "pyRover.PyRover",     /* tp_name */
    sizeof(PyRoverObject),          /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor)PyRover_dealloc, /* tp_dealloc */
    0,                              /* tp_vectorcall_offset */
    0,                              /* tp_getattr (unused) */
    0,                              /* tp_setattr (unused) */
    0,                              /* tp_as_async */
    0,                              /* tp_repr */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    0,                              /* tp_hash */
    0,                              /* tp_call */
    0,                              /* tp_str */
    0,                              /* tp_getattro */
    0,                              /* tp_setattro */
    0,                              /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    "Rover CAN master object",      /* tp_doc */
    0,                              /* tp_traverse */
    0,                              /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    0,                              /* tp_iter */
    0,                              /* tp_iternext */
    PyRover_methods,       /* tp_methods */
    0,                              /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc)PyRover_init,/* tp_init */
    0,                              /* tp_alloc */
    PyRover_new            /* tp_new */
};


// -----------------------------
// Module definition
// -----------------------------
static PyModuleDef pyrover_module = {
    PyModuleDef_HEAD_INIT,
    "pyRover",
    "Rover CAN master Python bindings",
    -1,
    nullptr
};

PyMODINIT_FUNC PyInit_pyRover(void) {
    if (PyType_Ready(&PyRoverType) < 0)
        return nullptr;

    PyObject* m = PyModule_Create(&pyrover_module);
    if (!m)
        return nullptr;

    Py_INCREF(&PyRoverType);
    PyModule_AddObject(m, "PyRover", (PyObject*)&PyRoverType);

    return m;
}
