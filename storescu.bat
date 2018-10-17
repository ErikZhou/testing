

@ECHO OFF
setLOCAL ENABLEDELAYEDEXPANSION
title  批量归档到PACS服务器
SET CallingAE=qqzhou
SET CalledAE=PACSSERVER
SET PACSHost=10.9.19.96
SET PACSPort=3333
:for1
SET /P ACVPath=请输入需要归档的文件夹，可以拖动到CMD窗口中
.\storescu.exe -xs +sd +r -aet %CallingAE% -aec %CalledAE% %PACSHost% %PACSPort% %ACVPath% -v
choice /c qn /n /t 20 /d n /m "请输入q键退出命令,不输入按键则继续循环:"
if %errorlevel%==1 exit
goto for1
PAUSE
