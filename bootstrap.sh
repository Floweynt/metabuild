clang++                                 \
    -std=c++20                          \
    -O3                                 \
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
    -D_IS_IMPL_SIDE                     \
    -DFMT_HEADER_ONLY                   \
    -fvisibility=hidden                 \
    -rdynamic

./metabuild -V build
