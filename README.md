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
| 2022-06-23 | 完成log.h文件，log.cpp开头。对比sylar源码，可能有些地方会有偏差，但是会以实现相同的功能为目标。结构如下：<br /> 顶层 ： LogManager日志器管理者    以map的形式管理数个Logger，可以删除、添加、获取某个日志器，默认生成主日志器<br/>              Logger日志器             以vector管理数个appender，可以添加删除appender，有写日志的方法<br/>              Appender输出器           输出器控制输出的地方，控制某个日志级别是否可以输出，控制输出的格式(控制LogFormatter)<br/>              LogFormatter格式器       以某种格式输出日志 可以修改格式<br/> 底层：  LogEvent日志事件          事件中有日志的所有信息，但是经过LogFormatter之后信息并不会全部输出，会按照指定的格式进行输出 |
| 2022-6-24  | log.cpp从底层写起，写完了LogLevel、LogEvent、LogFormat。<br /> 其中LogFormat的init方法算是日志模块中的重难点，这里用了一种另一套算法去解析格式。 <br /> 注意使用std::ostream时的引用 |
| 2022-06-25 | 写完了LogAppender以及其拓展类的实现，以及一部分Logger的实现  |
| 2022-06-26 | 基本完成日志模块，剩下一些细节需要打磨。                     |
| 2022-06-27 | 完善日志模块宏定义，可以利用宏写流式日志或格式化日志<br />日志模块代码在分支：XXXX |
| 2022-06-30 | 观看配置系统视频p9-p11                                       |
| 2022-07-20 | 到目前进度：<br />视频观看至p15。<br />代码完成至可以利用代码约定配置，实现从stl常见容器与自定义类读取到配置，实现了结合yaml从文件读取简单类型与上述类型。<br /><br />**总结**：<br />采用约定大于配置的思想，利用yaml-cpp实现yaml文件的数据读取，利用yaml-cpp可以将yaml文件中的map与c++的map(或者其他stl中的容器)进行转化。<br />**重难点**：<br />掌握yaml-cpp库用法<br />boost::lexical_cast是实现配置系统的主力函数     这个boost中的函数功能主要将  不同类型转换为对应的string类型，或者将string转化为T类型<br />偏特化LexicalCast，使其支持不同的stl容器从容器转换为string  或 从string转换成为对应的stl容器    转换过程利用的yaml作为中转<br />dynamic_pointer_cast   这个函数可以将基类转换为派生类   只适用于智能指针    <br />**设计**：<br />基类是正常的类，而派生类是函数模板类，灵活利用了派生类的偏特化，将string与不同的形式数据结构进行相互转换。<br />这种设计模式的优点，如果新增一个类型的转化，只需要支持这种类型的偏特化即可。 |

