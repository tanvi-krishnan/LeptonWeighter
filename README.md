# LeptonWeighter
Weights injected neutrino final states to neutrino fluxes.

Author: C.A. Arg\"uelles

Reviewer: A. Schneider

If there are any problems do not hesitate to email: caad@mit.edu

# Compilation instructions

Use the provided configure script. Try:

./configure --help

# Prerequesites

This software need the following nontrivial libraries:

* C++11 capable compiler. If working on the cobalt system do:
source scl_source enable devtoolset-2
* Boost 1.55
* Photospline-v2: https://github.com/cnweaver/photospline
* nuSQuIDS: https://github.com/arguelles/nuSQuIDS
* NewNuFlux-v2: http://code.icecube.wisc.edu/svn/sandbox/cweaver/NewNuFlux-new-photospline/

# Examples

The examples can be found in resources/example.

# Detailed Installation Instructions

These instructions are written under the assumption that you are installing LeptonWeighter on the Cobalts. 

1. Create a directory for your libraries and headers. Call it `$INSTALL`, or whatever you prefer. 

2. Set up a function in your `.bashrc` to set up needed environmental variables. If you were using python3, you would add
```
myfunction() {
    export INSTALL=/path/to/your/install/directory
    eval `/cvmfs/icecube.opensciencegrid.org/py3-v4.0.1/setup.sh`
    export PYTHONPATH=$INSTALL/lib:$INSTALL/lib/python3.6/site-packages:$PYTHONPATH
    export LD_LIBRARY_PATH=$INSTALL/lib:$INSTALL/lib64:$SROOT/lib64:$LD_LIBRARY_PATH
    export PKG_CONFIG_LIBDIR=$INSTALL/lib/pkgconfig:$SROOT/lib/pkgconfig:$SROOT/include:$PKG_CONFIG_LIBDIR
    export PKG_CONFIG_PATH=$PKG_CONFIG_LIBDIR:$PKG_CONFIG_LIBDIR
    export LIBRARY_PATH=$LD_LIBRARY_PATH:$LIBRARY_PATH
    export INCLUDE=$INSTALL/include:$INCLUDE
    export INCLUDE_DIR=$INCLUDE
    export CPATH=$INSTALL/include:$INCLUDE
    export CPLUS_INCLUDE_PATH=$CPATH:$CPLUS_INCLUDE_PATH
    export PATH=$INSTALL/bin:$PATH
    export PYTHON_INCLUDE_DIR=$SROOT/include/python3.6m/
    export PYTHON_LIBRARIES=$SROOT/lib:$SROOT/lib64
}
```
3. Log out, back in, and call the function you just defined to enter this environment. To verify, call `echo $SROOT`, and make sure it points to somewhere in `cvmfs`

4. Install the python module pkgconfig and configure h5py for python.

    a. install pkgconfig
    ```
    pip install --install-option="--prefix=$INSTALL" --ignore-installed pkgconfig
    ```
    
    b. installing and configuring h5py    

      - **PYTHON3:** h5py is already installed in this environment. Just use the pre-packaged script to create a pc file pointing towards it. Assuming you are in the LW source dir, run:
    ```
    python ./resources/make_pc.py h5py hdf5
    ```
      - **PYTHON2:** h5py is not installed in this environment, so you need to do this yourself. Pip runs into strange problems installing h5py here, so do the folowing from a separate directory. 
        a. download source code
        ```
        wget https://github.com/h5py/h5py/archive/2.9.0.tar.gz
        ```
        b. extract source code from tarball
        ```
        tar -xzf <file>
        ```
        c. go into the extracted folder, confiruge, build, and install h5py
        ```
        python setup.py configure --hdf5=$SROOT
        python setup.py build
        python setup.py install --prefix=$INSTALL
        ```
        d. then register it with pkg-config
        ```
        python ./resources/make_pc.py h5py hdf5
        ```
            
5. Installing the right version of Boost. Note: Boost already exists on the icecube cvmfs environments, but a specific version is required for LW. Boost 1.55 also has outstanding issues with python 3, so extra care is required! 

   a. Building and installing Boost 1.55 for **PYTHON 2**
     - download and unpack Boost 1.55 source
     ```
     wget -O boost_1_55_0.tar.gz http://sourceforge.net/projects/boost/files/boost/1.55.0/boost_1_55_0.tar.gz/download 
     tar xzf boost_1_55_0.tar.gz
     ```
     - Call the boostrap bash script
        ```
        ./bootstrap.sh --prefix=$INSTALL --with-libraries=python
        ```
     - Call b2 to build and install the boost libraries
       ```
       ./b2 install --prefix=$INSTALL
       ```
     - Make symbolic links to different names for the boost libraries
        ```
        ln -s $INSTALL/lib/libboost_python2.a $INSTALL/lib/libboost_python27.a
        ```
        
   b. Building and installing Boost 1.55 for **PYTHON 3**
     - download and unpack the Boost 1.55 source
     - Call the boostrap bash script
        ```
        ./bootstrap.sh --prefix=$INSTALL --with-libraries=python --with-python-version=3.6
        ```
     - Modify the generated project-config.jam file to accurately search for the python3 libraries.
        change line 18 from
        ```
        using python : 3.6 :  ;
        ```
        to read
        ```
        using python : 3.6 : /cvmfs/icecube.opensciencegrid.org/py3-v4.0.1/RHEL_7_x86_64/ : /cvmfs/icecube.opensciencegrid.org/py3-v4.0.1/RHEL_7_x86_64/include/python3.6m ;
        ```
     - Call b2 to build and install the boost libraries
        ```
        ./b2 install --prefix=$INSTALL
        ```
     - Make simbolic links to different names for the newely installed libraries
        ```
        ln -s $INSTALL/lib/libboost_python3.a $INSTALL/lib/libboost_python36.a
        ```
        
6. Build and install squids

   a. download the squids source
   ```
   https://github.com/jsalvado/SQuIDS/
   ```
   b. configure the source code
   ```
   ./configure --with-gsl=$SROOT --prefix=$INSTALL
   ```
   c. Make the source into objects 
   ```
   make - j16
   ```
   d. install the objects
   ```
   make install 
   ```
        
7. Build and install NuSquids

   a. download nusquids source
   ```
   https://github.com/arguelles/nuSQuIDS
   ```
   b. configure the source code
   ```
   ./configure --with-gsl=$SROOT --with-hdf5=$SROOT --with-squids=$INSTALL --with-python-bindings --with-boost=$INSTALL --prefix=$INSTALL
   ```
   c. Make the source code into objects
   ```
   make -j16
   ```
   d. install the objects
   ```
   make install
   ```
8. Build and install NewNuFlux

    a. Download and unpack newNuFlux source. At the time of writing, there was an outstanding bug in the NewNuFlux source code, and you may need to do some tweaking of `private/pybindings/module.cxx`.
    
    b. configure the Makefile
    ```
    ./configure --prefix=$INSTALL --with-boost=$INSTALL --with-hdf5=$SROOT --with-photospline-config=$SROOT/bin/photospline-config --with-python-bindings
    ```
    c. make the source code into objects
    ```
    make -j8
    ```
        
9. Lepton Weighter
    a. download or clone the git repository at
    ```
    https://github.com/IceCubeOpenSource/LeptonWeighter
    ```
    b. Configure the Makefile
    ```
    ./configure --prefix=$INSTALL --with-squids=$INSTALL --with-nusquids=$INSTALL --with-newnuflux=$INSTALL --with-photospline-config=$SROOT/bin/photospline-config --with-boost=$INSTALL
    ```
    c. make it
    ```
    make -j8
    ```
    d. make the shared objects, install them
    ```
    make install
    ```
    e. Build the pybindings. Go to private/pybindings and call
    ```
    python setup.py build
    ```
    f. install the pybindings
    ```
    python setup.py install prefix=$INSTALL
    ```
# Detailed author contributions and citation

The LeptonInjector and LeptonWeighter modules were motivated by the high-energy light sterile neutrino search performed by B. Jones and C. Argüelles. C. Weaver wrote the first implementation of LeptonInjector using the IceCube internal software framework, icetray, and wrote the specifications for LeptonWeighter. In doing so, he also significantly enhanced the functionality of IceCube's Earth-model service. These weighting specifications were turned into code by C. Argüelles in LeptonWeighter. B. Jones performed the first detailed Monte Carlo comparisons that showed that this code had similar performance to the standard IceCube neutrino generator at the time for throughgoing muon neutrinos.

It was realized that these codes could have use beyond IceCube and could benefit the broader neutrino community. The codes were copied from IceCube internal subversion repositories to this GitHub repository; unfortunately, the code commit history was not preserved in this process. Thus the current commits do not represent the contributions from the original authors, particularly from the initial work by C. Weaver and C. Argüelles. 

The transition to this public version of the code has been spearheaded by A. Schneider and B. Smithers, with significant input and contributions from C. Weaver and C. Argüelles. B. Smithers isolated the components of the code needed to make the code public, edited the examples, and improved the interface of the code. A. Schneider contributed to improving the weighting algorithm, particularly to making it work for volume mode cascades, as well as in writing the general weighting formalism that enables joint weighting of volume and range mode.

This project also received contributions and suggestions from internal IceCube reviewers and the collaboration as a whole. Please cite this work as:

LeptonInjector and LeptonWeighter: A neutrino event generator and weighter for neutrino observatories
IceCube Collaboration
https://arxiv.org/abs/2012.10449
