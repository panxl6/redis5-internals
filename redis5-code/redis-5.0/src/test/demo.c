//
// Created by panxl on 2020/8/8.
//

#include <stdio.h>
#include "../sds.h"


int main(void)
{
    sds str = sdsnew("hello world");
    printf("字符串长度: %zu\n", sdslen(str));

    return 0;
}
