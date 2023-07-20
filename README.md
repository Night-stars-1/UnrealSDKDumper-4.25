# UnrealSDKDumper

### 支持的游戏
 - Fortnite
 - Rogue Company
 - Dead By Daylight
 - Witch It
 - Brickadia
 - POLYGON
 - SCUM
 - Scavengers
 - The Isle
 - Splitgate
 - Dauntless
 - Boundary
 - The Cycle: Frontier
 - Calabiyau(卡拉彼丘)
 - Seria(晶核：魔导觉醒)
 - Game(尘白禁区)
### 支持引擎版本: 
- UE 4.23-4.27

### 我们做了什么

- 根据不同package包含的类的引用关系，进行拓扑排序，确定头文件正确引用顺序
- 测试大量游戏，找出一些造成sdk编译出错的变量名（例如：+.()<> 中文等），并进行处理
- 生成一个导入即可编译成功的sdk.h文件以及导入即可用的SDK文件夹

### Todo:

- (紧急) 修复错误的结构大小：
	- 主要是对于有一些结构体，成员的偏移甚至小于父类的大小，这说明父类的大小被高估
	- 修复方法是，对于成员偏移小于父类大小的父类，将父类大小更改那个成员偏移，取最小值
- 自动生成VS脚手架项目，尝试做到像CheatGear一样能够打开即用
- 生成cpp代理文件，实现能够通过ProcessEvent调用引擎的函数

### Contributor

Modified by BobH