clang++                                 \
    -std=c++20                          \
    -O0                                 \
    "-I${PWD}/api"                      \
    "-I${PWD}/meta/argparse/include/"   \
    meta/main.cpp                       \
    meta/impl/*                         \
    meta/dl/dl.cpp                      \
    meta/utils/*.cpp                    \
    -o                                  \
    metabuild                           \
    -ldl                                \
    -lm                                 \
    -lfmt                               \
    -D_IS_IMPL_SIDE -fvisibility=hidden \
    -rdynamic

./metabuild -V build
