#!/bin/sh

# 初始化redis5编译环境的工具脚本

## 拉取代码
if [[ -f "5.0.zip" ]]; then
    echo "删除已有的zip文件"
    rm 5.0.zip
fi

wget https://github.com/redis/redis/archive/5.0.zip

if [[  $? -gt "0" ]]; then
    echo $?
    echo "redis 5.0代码拉取异常"
    exit
fi

unzip 5.0.zip && rm 5.0.zip

## 复制CMakeList
cp CMakeFiles/CMakeLists.txt redis-5.0
cp CMakeFiles/src/modules/CMakeLists.txt redis-5.0/src/modules
cp CMakeFiles/deps/CMakeLists.txt redis-5.0/deps
cp CMakeFiles/deps/lua/CMakeLists.txt redis-5.0/deps/lua/
cp CMakeFiles/deps/linenoise/CMakeLists.txt redis-5.0/deps/linenoise/
cp CMakeFiles/deps/hiredis/CMakeLists.txt redis-5.0/deps/hiredis/


## 在string.h后面引入必要头文件
sed -i 's/<string.h>/<string.h>\n#include "ae.h"\n#include "zmalloc.h"/' redis-5.0/src/ae_select.c

## 执行mkreleasehdr.sh
(cd redis-5.0/src && bash mkreleasehdr.sh)

