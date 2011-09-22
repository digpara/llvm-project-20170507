// RUN: mkdir -p %T/test1 %T/test2

// RUN: CPATH=%T/test1: %clang -x c -E -v %s 2>&1 | FileCheck %s -check-prefix=CPATH
// CPATH: -I {{.*}}/test1 -I .
// CPATH: search starts here
// CPATH: test1

// RUN: OBJC_INCLUDE_PATH=%T/test1  OBJCPLUS_INCLUDE_PATH=%T/test1  CPLUS_INCLUDE_PATH=%T/test1 C_INCLUDE_PATH=%T/test2 %clang -x c -E -v %s 2>&1 | FileCheck %s -check-prefix=C_INCLUDE_PATH
// C_INCLUDE_PATH: -c-isystem {{.*}}/test2 -cxx-isystem {{.*}}/test1 -objc-isystem {{.*}}/test1 -objcxx-isystem {{.*}}/test1
// C_INCLUDE_PATH: search starts here
// C_INCLUDE_PATH-NOT: test1
// C_INCLUDE_PATH: test2
// C_INCLUDE_PATH-NOT: test1

// RUN: OBJC_INCLUDE_PATH=%T/test1  OBJCPLUS_INCLUDE_PATH=%T/test2  CPLUS_INCLUDE_PATH=%T/test1 C_INCLUDE_PATH=%T/test1 %clang -x objective-c++ -E -v %s 2>&1 | FileCheck %s -check-prefix=OBJCPLUS_INCLUDE_PATH
// OBJCPLUS_INCLUDE_PATH: -c-isystem {{.*}}/test1 -cxx-isystem {{.*}}/test1 -objc-isystem {{.*}}/test1 -objcxx-isystem {{.*}}/test2
// OBJCPLUS_INCLUDE_PATH: search starts here
// OBJCPLUS_INCLUDE_PATH-NOT: test1
// OBJCPLUS_INCLUDE_PATH: test2
// OBJCPLUS_INCLUDE_PATH-NOT: test1
