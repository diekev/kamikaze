かみかぜ
=======

Experimental node based 3D software used for R&D purposes, using a plugin architecture.

Dependencies
------------

| Name | Version |
| ---- | ------- |
| C++ 14 Compiler                                                      | GCC 5.3 |
| [Ego](https://github.com/diekev/utils/tree/master/ego)               | 1.1.0 |
| [Filesystem](https://github.com/diekev/utils/tree/master/filesystem) |  |
| [GLM](http://glm.g-truc.net/0.9.7/index.html)                        | 0.9.5.1-1 |
| [Qt5](https://doc.qt.io/qt-5/)                                       | 5.5 |

Getting and Building the Code
-----------------------------

### 1. Clone the repo:

```bash 
git clone https://github.com/diekev/kamikaze
```

### 2. Create a build location for the development kit:
```bash
cd kamikaze
cd sdk
mkdir build
cd build
```

### 3. Compile and install the development kit:

```bash
cmake                                 \
-DCMAKE_PREFIX_PATH=/where/to/install \
-DEGO_ROOT_DIR=/path/to/ego           \
..

make -j <NUM_CORES> install
```

### 4. Create a build location for the software:
```bash
cd kamikaze
mkdir build
cd build
```

### 5. Run a Basic build for the software:

```bash
cmake                                         \
-DEGO_ROOT_DIR=/path/to/ego                   \
-DFILESYSTEM_ROOT_DIR=/path/to/filesystem     \
-DKAMIKAZE_ROOT_DIR=/path/to/kamikaze_toolkit \
..

make -j <NUM_CORES> install
```
