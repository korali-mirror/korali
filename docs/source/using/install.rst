.. _install:

*********************
Installation
*********************

Docker
==========================

The easiest way to use Korali is to launch it's pre-built Docker container which provides Korali with all its dependencies already installed and configured. To launch the docker container, run:

    .. code-block:: bash

       docker run -it cselab/korali:latest

Manual Installation
==========================

Korali has been thoroughly tested on Linux (Ubuntu, Fedora) systems. Although it is possible to compile and run Korali on MacOS, we strongly recommend users to use the Docker image instead. Korali is not yet supported on Windows systems.

Below, we list the system requirements and steps to install Korali:

System Requirements
--------------------------

  - **C++ Compiler**
      Korali requires a C++ compiler that supports the C++17 standard (`-std=c++17`) to build.
      **Hint:** Check the following `link <https://en.cppreference.com/w/cpp/compiler_support#C.2B.2B14_core_language_features>`_ to verify whether your compiler supports C++17.
      Korali's installer will check the **$CXX** environment variable to determine the default C++ compiler. You can change the value of this variable to define a custom C++ compiler.

  - **Git Client**
      You need Git to clone (download) our code before installation.

  - **meson**
      To generate the installation configuration.

  - **ninja**
      To build Korali.

  - **Python >=3.8**
      Korali requires a version of Python higher than 3.8 to be installed in the system. Korali's installer will check the *python3* command. The path to this command should be present in the $PATH environment variable. *Hint:* Make sure Python3 is correctly installed or its module loaded before configuring Korali.

Installation Steps
--------------------------

1. Download Korali

  Download Korali with the following command:

  .. code-block:: bash

     git clone https://github.com/cselab/korali.git

2. Setup Installation

  To set up the compilation and installation, run:

  .. code-block:: bash

   cd korali
   meson setup build --buildtype=release --prefix=PREFIX

where ``PREFIX`` is the absolute path where Korali will be installed.
For example, use ``$HOME/.local/`` to install it in the same folder where ``pip`` installs packages (this can be verified with ``python3 -m sysconfig | grep userbase``).
Optionally you can install Korali with support for MPI, OneDNN, and CUDNN, using the optional parameters:

  .. code-block:: bash

   cd korali
   meson setup build --buildtype=release --prefix=PREFIX -Dmpi=true -Donednn=true -Dcudnn

A full list of installation options can be found in `meson_options.txt <https://github.com/cselab/korali/blob/master/meson_options.txt>`_. For more information, see *Optional Requirements* below.

3. Build

  To build Korali, run:

  .. code-block:: bash

   meson compile -C build

4. Install

  To install Korali, run:

 .. code-block:: bash

    meson install -C build


To uninstall Korali, run ``cd build && ninja uninstall`` or manually delete the folder containing the ``korali`` package.

5. Setup environment

  The ``LD_LIBRARY_PATH``, ``PATH``, ``PYTHONPATH`` environment variables need to be correctly setup for the linker to find the correct libraries at the moment of runtime. We provide a tool that facilitates this task:
  
  .. code-block:: bash
  
     source tools/env/set_env.sh PREFIX
     
Troubleshooting
--------------------------

If you are experiencing problems installing or running Korali, please check the following hints:

  - Check Korali's `system requirements <#system-requirements>`_ to verify that your system has all the required software components.

  - Check the `build status <https://app.circleci.com/pipelines/github/cselab/korali>`_  to see if Korali is currently building correctly.

  - If the problem persists, please submit a new `issue report <https://github.com/cselab/korali/issues>`_ on our Github repository detailing the problem, your system information, and the steps to replicate it and we will promptly address it.

  - For further questions, feel free to `contact us </korali/#contact>`_.


Cray systems (Piz Daint)
--------------------------

Cray systems use a propietary build system that may cause conflicts with the default meson configuration when using MPI. To fix this, the following steps are recommended:

1) Specify the `cc` and `CC` commands as default C and C++ compilers, respectively:

.. code-block:: bash

   CC=cc CXX=CC meson setup build --buildtype=release --prefix=PREFIX
   
2) It is possible tat the the default installation of `mpi4py` possibly uses a different MPI implementation than Korali, preventing multi-rank runs. To fix it, configure MPI compilers and reinstall `mpi4py` and Korali.

.. code-block:: bash

    # Create wrappers `mpicc` and `mpic++` around Cray compilers `cc` and `CC`, respectively.
    # Warning: this will overwrite any `mpicc` and `mpic++` in your `~/bin` folder!
    mkdir -p $HOME/bin
    echo -e '#!/bin/bash'"\n"'cc "$@"' > $HOME/bin/mpicc
    echo -e '#!/bin/bash'"\n"'CC "$@"' > $HOME/bin/mpic++
    chmod +x $HOME/bin/mpicc $HOME/bin/mpic++

    # Load Python module (you can add this to your `~/.bashrc`).
    module load cray-python

    # Reinstall mpi4py locally and reinstall korali.
    python3 -m pip install --user mpi4py --ignore-installed -v
    
Optional Requirements
--------------------------

 - **oneDNN**
      Korali uses the `OneAPI Deep Neural Network Library <https://oneapi-src.github.io/oneDNN/>`_ for deep learning modules, and is disabled by default. You can enable it by adding the ``-Donednn=true`` option on the meson configuration line. To recommended configuration for oneDNN is:

.. code-block:: bash

    wget https://github.com/oneapi-src/oneDNN/archive/refs/tags/v2.2.2.tar.gz -O oneDNN-v2.2.2.tar.gz; \
    tar -xzvf oneDNN-v2.2.2.tar.gz; \
    mkdir -p "oneDNN-2.2.2/build"; \
    cd "oneDNN-2.2.2/build"; \
    CXXFLAGS=-O3 cmake .. \
     -DCMAKE_INSTALL_PREFIX=$HOME/.local \
     -DDNNL_BUILD_EXAMPLES=OFF \
     -DDNNL_BUILD_TESTS=OFF \
     -DDNNL_ENABLE_CONCURRENT_EXEC=ON \
     -DDNNL_ARCH_OPT_FLAGS='-march=native -mtune=native' \
     -DBUILD_SHARED_LIBS=true; make -j4; make install

  - **CMake**
      Korali requires that you have `CMake <https://cmake.org/>`_ version 3.0 or higher installed in your system if you need it to install certain external libraries automatically.

  - **MPI**
      One way to enable support distributed conduits and computational models is to configure Korali to compile with an MPI compiler. The installer will check the *$MPICXX* environment variable to determine a valid MPI C++ compiler.

  - **MPI4Py**
      If you need to run Python-based MPI application as computational models in Korali, you will need to install the MPI4py python module, and install Korali via meson using the `-Dmpi4py=true` option.

