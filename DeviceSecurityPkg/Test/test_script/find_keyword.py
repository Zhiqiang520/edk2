import re
import sys
import os
import codecs

class logprocessor:



	def __init__(self, logPath, keywordPath):
		print("========================== new test case begin ============================\n") 
		self.logPath = logPath
		self.keywordPath = keywordPath
		self.keywordLines = self.GetKeywords(self.keywordPath)
		self.LogContent = self.GetLogContent(self.logPath)
		pass
		
	def Process(self):
		processed = False
		count = 0;
		print("\n*************try to find keyword in the log ......\n") 
		for line in self.keywordLines:	
			if(line in self.LogContent):
				processed=True
			else:
				processed = False
				print("\n***Can not find the keyword below in the log!!!") 
				print(line)
				break
	
		if (processed):
			print("\n***keywords are all found in the log successfully :-)") 
			print("*** So this test case passed :-)\n") 
		else:
			print("\n***found some errors in the log!!!") 
			print("*** So this test case failed :(\n") 
			
		print("******************************************************\n") 
		pass

	def GetKeywords(self, keywordPath):
		keywordFile = open(keywordPath, 'r')
		keywordLines = []
		print("--------------------------keywords list-----------------------------\n") 
		for line in keywordFile.readlines():
			line = line.strip()
			if (not line):
				continue
			print(line)
			keywordLines.append(line)
		print("\n------------------------keywords list end--------------------------\n") 
		keywordFile.close()
		return keywordLines
		pass
		
	def GetLogContent(self, logPath):
		LogFile = codecs.open(logPath, 'r', 'utf-16le')
		LogFileLines = []
		for line in LogFile.readlines():
			line = line.strip()
			if (not line):
				continue
			LogFileLines.append(line)		
		LogFile.close()
		return LogFileLines
		pass
		
if __name__ == '__main__':

	if len(sys.argv) != 3:
		print("Usage:\n%s" % "  python find_keyword.py <logFile> <keywordFile> " )
		sys.exit(0)
	
	logPath = sys.argv[1]
	keywordPath = sys.argv[2]
	
	my_logprocessor = logprocessor(logPath, keywordPath)
	my_logprocessor.Process()
	
	pass

