# my_sylar
this is a learning program

### 参考资料

sylar源码git地址：`https://github.com/sylar-yin/sylar`

sylar配套视频地址：`https://www.bilibili.com/video/BV184411s7qF?p=6&spm_id_from=pageDriver&vd_source=e5ea4d6dfdbf10d1586394cd882ac61e`

路人大佬注写的sylar源码git地址：`https://github.com/zhongluqiang/sylar-from-scratch`

### 项目进度

| 日期       | 事件                                                         |
| ---------- | ------------------------------------------------------------ |
| 2022-06-20 | 再战sylar项目，最开始因为字体和声音都很小劝退了。从日志模块开始(视频学习日志模块) |
| 2022-06-21 | 看完了日志模块大部分代码，视频差2个半                        |
| 2022-06-22 | 日志模块源代码和视频看完，开始自己着手开发                   |
| 2022-06-23 | 写完了log.h文件，log.cpp开了个头。代码不是原封不动按照sylar源码写的，可能有些地方会有偏差，但是会以实现相同的功能为目标。结构如下：<br /> 顶层 ： LogManager日志器管理者    以map的形式管理数个Logger，可以删除、添加、获取某个日志器，默认生成主日志器<br/>              Logger日志器             以vector管理数个appender，可以添加删除appender，有写日志的方法<br/>              Appender输出器           输出器控制输出的地方，控制某个日志级别是否可以输出，控制输出的格式(控制LogFormatter)<br/>              LogFormatter格式器       以某种格式输出日志 可以修改格式<br/> 底层：  LogEvent日志事件          事件中有日志的所有信息，但是经过LogFormatter之后信息并不会全部输出，会按照指定的格式进行输出 |
| 2022-6-24  | log.cpp从底层写起，写完了LogLevel、LogEvent、LogFormat。<br /> 其中LogFormat的init方法算是日志模块中的难点，这里用了一种另一套算法去解析格式。 <br /> 注意使用std::ostream时的引用 |
| 2022-06-25 | 写完了LogAppender以及其拓展类的实现，以及一部分Logger的实现  |
| 2022-06-26 | 基本完成日志模块，剩下一些细节需要打磨。                     |
|            |                                                              |
|            |                                                              |
|            |                                                              |

