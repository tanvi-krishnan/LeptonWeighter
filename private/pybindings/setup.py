from distutils.core import setup
from distutils.extension import Extension
import numpy
import os.path
import os
import pkgconfig
import sys

try:
    cvmfs_env_root=os.environ['SROOT']
except KeyError:
    cvmfs_env_root="/usr/local/"

try:
    env_prefix=os.environ['PREFIX']
except KeyError:
    env_prefix="/usr/local/"

if sys.platform == 'win32' or sys.platform == 'win64':
    print('Windows is not a supported platform.')
    quit()
else:
    include_dirs = [
            '../../public/',
            '/usr/local/Cellar/python/2.7.9/Frameworks/Python.framework/Versions/2.7/lib/python2.7/../../include/python2.7',
            numpy.get_include(),
            env_prefix+'/include',
            cvmfs_env_root + "/include/",
            ]
    if sys.version[0]=='3':
        libraries = [
                'python{}m'.format(sys.version[0:3]),'boost_python3',
                'LeptonWeighter',
                ]
    elif sys.version[0]=='2':
         libraries = [
                'python{}'.format(sys.version[0:3]),'boost_python',
                'LeptonWeighter',
                ]
    else:
        raise Exception("Python version {} not supported".format(sys.version[0]))
    library_dirs = [
            '../lib/',
            '/usr/local/Cellar/python/2.7.9/Frameworks/Python.framework/Versions/2.7/lib/python2.7',
            '/usr/local/Cellar/python/2.7.9/Frameworks/Python.framework/Versions/2.7/lib/python2.7/..',
            env_prefix+'/lib/',
            env_prefix+'/lib64/',
            cvmfs_env_root + "/lib/",
            ]

    files = ['lepton_weighter_pybi.cpp']

setup(name = 'LeptonWeighter', author = "Carlos A. Arguelles",
        ext_modules = [
            Extension('LeptonWeighter',files,
                library_dirs=library_dirs,
                libraries=libraries,
                include_dirs=include_dirs,
                extra_compile_args=['-O3','-fPIC','-std=c++11',pkgconfig.cflags('squids'),pkgconfig.cflags('nusquids')],
                extra_link_args=[pkgconfig.libs('squids'),pkgconfig.libs('nusquids')],
                depends=[]),
            ]
        )

