# Background

此仓库中的代码为学校petri网课程代码，其中包含petri网相关的简单功能，以及类的实现和调用。

# Introduce

命名为test+数字的函数是程序开发时所用测试/调用示例，每个示例开头用注释标注测试内容，整个项目会随着课程的节奏逐渐增加功能，README.md也会随之更新

* homework1：建立一个petri网，并将其可能的状态（marking）表示为可达图。

  > 在homework1中编写了petri net 类和 reachability graph 类
  
* homework2：实现附时petri网，及其可达图

  > 编写Timed petri net 类，继承于petri net 类，并编写了Timed reachability graph用于生成附时Petri网的可达图。并且为函数和类增加了注释表
  >
# How to use it
* 需要下载本仓库的代码
* 需要下载[Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page)的代码（请参考attention第2条）
* 将Eigen代码全部解压到本仓库代码文件夹中
* 参考test文件中的代码进行测试

# Attention

1. 本仓库目的为保存本地代码，以及与同学分享和讨论petri net相关问题。因暂时未能找到让代码在其他环境都能运行的办法，若下载代码后不能运行属正常现象。
2. 因需用到矩阵运算，本项目使用了Eigen的C++线性代数模板库（v3.4.0），不清楚是否会违反开源软件许可证license（Eigen使用的是MPL2.0），并没有将Eigen的代码上传至此仓库，请用户自行前往官网下载：[Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page)（免费、且可直接下载到源代码）
3. 欢迎与我讨论和交流。