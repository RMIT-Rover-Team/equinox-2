from setuptools import setup, Extension
import sys
import sysconfig

extra_compile_args = ["-std=c++17"]


ext = Extension(
    "pyRover",
    sources=[
        "CommandUtils.cpp",
        "SocketCanWrapper.cpp",
        "RoverCanMaster.cpp",
        "pyRover.cpp"
    ],
    language="c++",
    extra_compile_args=extra_compile_args,
    include_dirs=[],  # add if your headers live elsewhere
    # libraries=[...],     # if you link against external libs
    # library_dirs=[...],
)

setup(
    name="pyRover",
    version="0.1.0",
    ext_modules=[ext],
)
