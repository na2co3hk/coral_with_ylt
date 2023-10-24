# coral_with_ylt
使用alibaba开发的[`yaLanTingLibs`](https://github.com/alibaba/yalantinglibs)重新封装了coral

意义不大，因为yaLanTingLibs里本来就有高性能的http框架[`cinatra`](https://github.com/qicosmos/cinatra)，这里单纯学习用

大部分还是库里原本的代码

在原本的coral中增加了以下功能
* 异步http客户端
* 基于协程的高效异步日志系统，每0.5s进行刷盘
* 全局容器Box，用于存储类似数据库连接、配置类等作用与多个服务中的类
