#-------------------------------------------------
#
# Project created by QtCreator 2012-09-15T18:48:36
#
#-------------------------------------------------

QT       += core gui

TARGET = swarm-image-segmenter-qt
TEMPLATE = app


SOURCES += main.cpp \
    pso_cluster.cpp \
    main_ui.cpp \
    segmenter_engine.cpp

HEADERS  += pso_cluster.h \
    main_ui.h \
    segmenter_engine.h

FORMS    += main_ui.ui

OTHER_FILES += kernel.cu

QMAKE_CXXFLAGS_DEBUG += -pg
QMAKE_LFLAGS_DEBUG += -pg

# Path to cuda SDK install
CUDA_SOURCES = $$OTHER_FILES
CUDA_SDK = /home/adi/NVIDIA_GPU_Computing_SDK/C # path to your gpu sdk
CUDA_DIR = /usr/local/cuda
CUDA_ARCH = sm_11
# nvcc flags (ptxas option verbose is always useful)
debug:NVCCFLAGS = -g -G #-pg #-keep --keep-dir=obj
release:NVCCFLAGS = -O
NVCCFLAGS += --ptxas-options=-v

# include paths
INCLUDEPATH += $$CUDA_DIR/include
INCLUDEPATH += $$CUDA_SDK/common/inc/
INCLUDEPATH += $$CUDA_SDK/../shared/inc/
# lib dirs
QMAKE_LIBDIR += $$CUDA_DIR/lib64
QMAKE_LIBDIR += $$CUDA_SDK/lib
QMAKE_LIBDIR += $$CUDA_SDK/common/lib

LIBS += -lcudart -lcutil_x86_64 -lopencv_core -lopencv_highgui
# join the includes in a line
CUDA_INC = $$join(INCLUDEPATH,' -I','-I',' ')

cuda.input = CUDA_SOURCES
cuda.output = ${OBJECTS_DIR}${QMAKE_FILE_BASE}_cuda.o
cuda.commands = $$CUDA_DIR/bin/nvcc -arch=$$CUDA_ARCH $$NVCCFLAGS $$CUDA_INC $$LIBS -c ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_OUT}
cuda.dependency_type = TYPE_C
cuda.depend_command = $$CUDA_DIR/bin/nvcc -M $$CUDA_INC $$NVCCFLAGS   ${QMAKE_FILE_NAME}
# Tell Qt that we want add more stuff to the Makefile
QMAKE_EXTRA_COMPILERS += cuda

