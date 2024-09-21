# overview
this patch for previewing the MicroMsg (or WeChat) DBs. it require you can get your db key. i have some steps to help you approach just using windbg. https://github.com/bbqz007/CrackMicroMsgDBKey . 

这个补丁能够预览微信客户端的数据库，但需要你自己想方法先得到数据库的密钥。如果你有这样的需要，我有一些步骤可以为你提供参考，仅使用windbg就能够做得到。https://github.com/bbqz007/CrackMicroMsgDBKey

# howto
* copy the `sqlite3.dll` into the dir same as the `KTL.exe` locates.
* replace the `AlgoDataTool.cpp` using the patch.
* edit the `AlgoDataTool.cpp`, assign your hex key string to the `#define hexkey = ""`.
* load the AlgoDataTool.
* drag and drop your db file into the AlgoDataTool gui.
* it shows the tables of a db, when you double click one table name, a popup window shows the rows of the table.

<img src="https://github.com/bbqz007/KTL/blob/master/resources/GIF_KTL_WeChatDb.gif" width="61.8%">

# sqlite3.dll
download from nuget, [native-sqlcipher](https://www.nuget.org/packages/SQLitePCL.native.sqlcipher.windows/0.9.0-pre7).

and i rename `sqlcipher.dll` to `sqlite3.dll`.
