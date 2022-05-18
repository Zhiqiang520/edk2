@echo off
py -3 launch_test.py

SET test_output=Z:\Build\EmulatorX64\DEBUG_VS2019\X64\test_output
if not exist %test_output%\result mkdir %test_output%\result

echo "check the keyword in log file"
py -3 find_keyword.py %test_output%\log\TestConfig_0.log keyword\keywordTestConfig_0.txt > %test_output%\result\resultTestConfig_0.txt
py -3 find_keyword.py %test_output%\log\TestConfig_1.log keyword\keywordTestConfig_1.txt > %test_output%\result\resultTestConfig_1.txt
py -3 find_keyword.py %test_output%\log\TestConfig_2.log keyword\keywordTestConfig_2.txt > %test_output%\result\resultTestConfig_2.txt
py -3 find_keyword.py %test_output%\log\TestConfig_3.log keyword\keywordTestConfig_3.txt > %test_output%\result\resultTestConfig_3.txt
py -3 find_keyword.py %test_output%\log\TestConfig_4.log keyword\keywordTestConfig_4.txt > %test_output%\result\resultTestConfig_4.txt
py -3 find_keyword.py %test_output%\log\TestConfig_5.log keyword\keywordTestConfig_5.txt > %test_output%\result\resultTestConfig_5.txt
py -3 find_keyword.py %test_output%\log\TestConfig_6.log keyword\keywordTestConfig_6.txt > %test_output%\result\resultTestConfig_6.txt
py -3 find_keyword.py %test_output%\log\TestConfig_7.log keyword\keywordTestConfig_7.txt > %test_output%\result\resultTestConfig_7.txt

echo "Generate the Test Report"
py -3 Generate_Report.py
