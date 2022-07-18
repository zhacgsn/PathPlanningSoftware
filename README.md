# PathPlanningSoftware
# 开发环境

- Windows 64bit操作系统

- Desktop Qt 5.9.8 MinGW 32bit编译运行

- 达梦数据库DM8 32bit


# 数据库表设计

只用到一个表PATHTASK，所属表空间数据文件为MAIN.DBF，表属性包括TASK_UUID（任务编号，主键）、TASK_DATE（任务日期）、TASK_TIME（任务时间）、TASK_OPERATOR（操作人员）、TASK_POINT（取货点），TASK_ASSEMBLE（装配点）


# 界面效果
<img width="649" alt="日志路径" src="https://user-images.githubusercontent.com/49667117/179452337-58502440-917e-465d-b9e7-d95b4419a8f3.png">
<img width="650" alt="地图管理界面" src="https://user-images.githubusercontent.com/49667117/179452381-d7f4e84b-c19b-40c4-a0a8-bc98ffcb5cec.png">
<img width="649" alt="发送路径指令（网络）" src="https://user-images.githubusercontent.com/49667117/179452493-4decf775-0ee9-4f1d-9de0-c8acbce0157c.png">
<img width="649" alt="发送路径指令（串口）" src="https://user-images.githubusercontent.com/49667117/179452500-c4eadf9d-185e-495c-b49c-a3b8b7f77964.png">
<img width="649" alt="文件数据库对应记录" src="https://user-images.githubusercontent.com/49667117/179452535-70924ae2-d59f-42ca-9f9d-f61ca38dd895.png">


# 一些说明

- 采用A* 算法

- 界面实现参考 https://github.com/feiyangqingyun/QWidgetDemo
