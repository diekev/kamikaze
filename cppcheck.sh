/opt/bin/cppcheck \
    -j8 \
    --enable=all \
    -ibuild_release \
    -ibuild_debug \
    -isdk/build \
    -isdk/build_debug \
    -I/opt/lib/ego/include \
    -I/opt/lib/utils/include \
    .

#    -I/home/kevin/src/Qt/5.5/gcc_64/include/ \
#    -I/home/kevin/src/Qt/5.5/gcc_64/include/QtCore/ \
#    -I/home/kevin/src/Qt/5.5/gcc_64/include/QtOpenGL/ \
#    -I/home/kevin/src/Qt/5.5/gcc_64/include/QtGui/ \
#    -I/home/kevin/src/Qt/5.5/gcc_64/include/QtWidgets/ \
