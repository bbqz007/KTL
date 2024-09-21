# overview
this patch for previewing the MicroMsg (or WeChat) DBs. it require you can get your db key. i have some steps to help you approach just using windbg. https://github.com/bbqz007/CrackMicroMsgDBKey . 

这个补丁能够预览微信客户端的数据库，但需要你自己想方法先得到数据库的密钥。如果你有这样的需要，我有一些步骤可以为你提供参考。https://github.com/bbqz007/CrackMicroMsgDBKey

# howto
* copy the `sqlite3.dll` into the dir same as the `KTL.exe` locates.
* replace the `AlgoDataTool.cpp` using the patch.
* edit the `AlgoDataTool.cpp`
