import os
import stat
import shutil
import sys
import time

if os.path.exists('Z:/'):
    os.system('subst z: /d')
os.system('subst z: C:\edk2_zhiqiang')

def readonly_handler(func, path, excinfo): 
    os.chmod(path, stat.S_IWRITE)
    func(path)  
# if os.path.exists('Z:\Build'):
#     shutil.rmtree('Z:\Build', onerror=readonly_handler)
os.environ["PYTHON_HOME"] = r"C:\Python38"
os.chdir('Z:')

os.system(r'edksetup.bat && build -p EmulatorPkg\EmulatorPkg.dsc -t VS2019 -a X64 -b DEBUG -j build_DEBUG.log')
os.system(r'edksetup.bat && build -p DeviceSecurityPkg\DeviceSecurityPkg.dsc -t VS2019 -a X64 -b DEBUG -j build_DEBUG.log')
os.system(r'copy /Y Build\DeviceSecurityPkg\DEBUG_VS2019\X64\*.efi Build\EmulatorX64\DEBUG_VS2019\X64')

WinHost_exe_path = r'Z:\Build\EmulatorX64\DEBUG_VS2019\X64'
test_output = os.path.join(WinHost_exe_path,'test_output')
startup_nsh = os.path.join(WinHost_exe_path,'startup.nsh')
test_output_log = os.path.join(test_output, 'log')

if os.path.exists(test_output):
    shutil.rmtree(test_output, onerror=readonly_handler)
os.mkdir(test_output)
os.chdir(test_output)
os.system('echo %date% > env.txt')
os.system('echo %time% >> env.txt')
os.system('git log --pretty="%H" -1 >> env.txt')

os.mkdir(test_output_log)
for root,dirs,files in os.walk(os.path.join(sys.path[0], 'TestConfig')):
    for filespath in files:
        print (os.path.join(root,filespath))
        print(filespath[-5])
        shutil.copy(os.path.join(root,filespath), startup_nsh)
        os.chdir(WinHost_exe_path)
        if os.path.exists(os.path.join(test_output_log,'TestConfig_' + filespath[-5] + '.log')):
            os.remove(os.path.join(test_output_log,'TestConfig_' + filespath[-5] + '.log'))
        os.system(r'start WinHost.exe')
        count = 0
        while not os.path.exists(os.path.join(test_output_log,'TestConfig_' + filespath[-5] + '.log')):
            #crash before creating log file, then create a fake log file and break
            if(count > 15):
                os.system('echo fake log file> %s'%(os.path.join(test_output_log,'TestConfig_' + filespath[-5] + '.log')))
                print('waiting too long for test case finish, so exit')
                break
            time.sleep(3)
            print('waiting for test case finish')
            count+=1
        else:
            time.sleep(10)
            print('test case finish')
        time.sleep(3)
        os.system(r'taskkill /IM WinHost.exe /F /T')
        time.sleep(3)