3.71
--files were errorenously getting _logXX.log names instead of _logXX.txt. Fixed
3.7
what's new:
--new feature: per thread indent in the log files
--new options:  STLOG_MULTITHREADING (turns on/off multithreading features), STLOG_USE_PERFORMANCE_CONTER (you can turn off usage of GetPerformanceCounter() function)
--fixed bug with stack overflow in GetLogFileName() function (thanks to Alexander Shargin)
--fixed bug with unclosed handle when used from DLL (thanks to Rene Heijndijk).
--fixed bug with ___DoNothing() function for the cases when the STLOG_DEBUG macro is undefined
--new functions: STLOG_WRITE_IID, STLOG_WRITE_GUID, STLOG_WRITE_CLSID
