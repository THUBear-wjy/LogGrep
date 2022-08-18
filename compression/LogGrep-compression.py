import sys
import os
import argparse
import time
import datetime
import threading
import logging
from subprocess import call
from concurrent.futures import ThreadPoolExecutor, wait, ALL_COMPLETED  
from os.path import join, getsize


lock = threading.RLock()
gl_threadTotTime = 0
gl_errorNum = 0

#func define
def add_argument(parse):
    parse.add_argument("--Input", "-I", help="The input directory include log files (such as 0.log, 1.log, etc...) like example/")
    parse.add_argument("--Output", "-O", help="The output directory inclues output zip files (such as 0.zip, 1.zip, etc...) like example_zip/") 

def path_pro(path):
    if(path[-1] != '/'):
        path += '/'
    return path

def check_args(args):
    print("Input file: {}".format(args.Input))
    if (not os.path.exists(args.Input)):
        print("No input path. Quit")
        return 0

    if (not os.path.exists(args.Output)):
        print("No output path. Will make new directory at {}".format(args.Output))
    else:
        call("rm -rf " + args.Output,shell=True)
    os.mkdir(args.Output)
    return 1
    
def atomic_addTime(step):
    lock.acquire()
    global gl_threadTotTime
    gl_threadTotTime += step
    lock.release()

def atomic_addErrnum(step):
    lock.acquire()
    global gl_errorNum
    gl_errorNum += step
    lock.release()

def writeLog(fname, message, levelStr):
    logging.basicConfig(filename=fname,
                           filemode='a',
                           format = '%(asctime)s - %(message)s')
    logger = logging.getLogger(__name__)
    if (levelStr =='WARNING'):
        logger.warning(message)
    elif (levelStr =='INFO'):
        logger.info(message)   

#return exec time (t2-t1)
def procFiles(file_list, fileBeginNo, fileEndNo, now_input, now_output, isPadding):
    t1 = time.time()
    #parser
    input_path = path_pro(now_input)
    output_path = path_pro(now_output)
    for i in range(fileBeginNo, fileEndNo + 1):
        target_file = file_list[i]
        now_input = input_path + str(target_file)
        now_output = output_path + str(target_file) + ".zip"
        if(not os.path.exists(now_input)):
            print(now_input + "does not exists")
            continue
        order = "./THULR -I " + now_input + " -O " + now_output + " -P " + isPadding + " -Z zip"
        print(order + " " + threading.current_thread().name)
        res = call(order,shell=True)
        if (res != 0):
            tempStr = "thread: {}, fileNo: {} ERROR!!".format(threading.current_thread().name, filename, str(i))
            print (tempStr)
            writeLog("./Log_{}".format(datetime.date.today()), tempStr,'WARNING')
            atomic_addErrnum(1)
    
    t2 = time.time()
    tempStr = "thread:{}, fileNo: {} to {} , cost time: {}".format(threading.current_thread().name, fileBeginNo, fileEndNo, t2 - t1)
    print (tempStr)
    #writeLog(str(output_path) + "Log_{}".format(datetime.date.today()), tempStr,'WARNING')

    return t2 - t1

def procFiles_result(future):
    atomic_addTime(future.result())

# calculate the reduce rate of each type file
def getdirsize(dir):
    size = 0
    for root, dirs, files in os.walk(dir):
        size += sum([getsize(join(root, name)) for name in files])
    return size


def threadsToExecTasks(file_list, now_input, now_output, isPadding):
    fileListLen = len(file_list)
    curFileNumBegin = 0
    curFileNumEnd = 0
    step = maxSingleThreadProcFilesNum
    if (step == 0):# dynamic step
        step = fileListLen // maxThreadNum
        if(step == 0):
            step = 1 # make sure the step is bigger than 0
    
    threadPool = ThreadPoolExecutor(max_workers = maxThreadNum, thread_name_prefix="LR_")
    while curFileNumBegin < fileListLen:
        if (curFileNumBegin + step > fileListLen):
            curFileNumEnd = fileListLen - 1
        else:
            curFileNumEnd = curFileNumBegin + step - 1

        future = threadPool.submit(procFiles, file_list, curFileNumBegin, curFileNumEnd, now_input, now_output, isPadding)
        future.add_done_callback(procFiles_result)
        curFileNumBegin = curFileNumEnd + 1
    #wait(future, return_when=ALL_COMPLETED)
    threadPool.shutdown(wait=True)

def recheck(fileListLen, now_input, now_output, isPadding):
    outputfile = os.listdir(now_output)
    new_set = set()
    for i in range(0, fileListLen):
        if not ((str(i) + ".zip") in now_output):
            new_set.add(i)
    return new_set

if __name__ == "__main__":
    parse = argparse.ArgumentParser()
    add_argument(parse)
    args = parse.parse_args()
    if (not check_args(args)):
       exit(1)

    #init params
    input_path = args.Input
    output_path = args.Output
    is_padding = "T"
    maxThreadNum = 4
    maxSingleThreadProcFilesNum = 0
    #threadPool = ThreadPoolExecutor(max_workers = maxThreadNum, thread_name_prefix="test_")
    
    time1 = time.time()
    now_input = input_path
    time_t1 = time.time()
    now_output = output_path
    all_files = os.listdir(now_input)

    if (not os.path.exists(now_output)):
        os.mkdir(now_output)
        
        ###ThreadPool to Proc Files
        #file_list = list(recheck(len(all_files), now_input, now_output, is_padding))
        
    threadsToExecTasks(all_files, now_input, now_output, is_padding)

    time_t2 = time.time()
    tempStr = "Finished, total time cost: {} , thread accum time: {}".format(time_t2 - time_t1, gl_threadTotTime)
    print(tempStr)
    gl_threadTotTime = 0 # reset


   


