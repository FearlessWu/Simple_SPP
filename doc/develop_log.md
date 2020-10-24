# **开发者日志**

## **1、数据结构**
### *1)观测值*
## **2、枚举类型**
&emsp; &ensp; 在头文件中各种枚举类型是为了方便了解各种返回值、错误码等含义，根本目的是为更加方便的阅读。这里对头文件中的枚举类型进行解释，方便开发者使用和维护。
### *1）返回值*
```
typedef enum
{
    RET_FAIL    = 0, 
    RET_SUCCESS = 1
} ret_status_t;
```
含义：函数返回值：失败 = 0， 成功 = 1；<br/>维护：若有其他返回状态，可在其中进行添加，如：`RET_NOT_BAD = 2`。
### *2）观测值状态*
### *3）错误级别*
### *4）错误码*
## **3、宏定义**

## **4、通用函数**

## **5、代码提交规范**
1）代码本地commit后，不直接push，先fetch，然后rebase，若有冲突，先处理完冲突再push。这一点非常重要！<br/>2）每次提交必须添加massage，并按照一定的格式进行填写，方便版本回溯：
```
[ADD/FIX/ENC] [some decription for this commit balabala....] [author name]
ADD：增加某些功能/文件
FIX：修复某些bug
ENC：提升某些性能
```
for example: &emsp; 本次commit增加了读取o文件功能，则massage可如下添加：
```
[ADD] add a function of reading observation file. wyatt wu
```
