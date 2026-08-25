from setuptools import setup, Extension
import sys
import sysconfig

extra_compile_args = ["-std=c++17"]


ext = Extension(
    "torque",
    sources=[
        "pyTorqueWrapper.cpp",
        "torque_handler.cpp",
        "fake_wrapper.cpp",

        "pid_calc.cpp",
        "linear_calc.cpp",
        "odrive_controllers.cpp",
        "odrive_wrapper.cpp"

        # ...
    ],
    language="c++",
    extra_compile_args=extra_compile_args,
    include_dirs=["../include/"],  # add if your headers live elsewhere
    # libraries=[...],     # if you link against external libs
    # library_dirs=[...],
)

setup(
    name="torque",
    version="0.1.0",
    ext_modules=[ext],
)
