from subprocess import call
import os
import sys
name = ["Apsara", "Fastcgi", "Metering","Monitor","Ols","Op","Oss", "Pangu", "PAudit", "Presto", "PSummary", "Request", "Rpc", "Shennong", "Sinput", "Sla", "Sys", "Tubo"]
#name = ["K8S", "Tail"]
#name = ["K8S", "Tail", "Nginx", "Slsf", "Slss"]
def makeDir(path):
    for n in name:
        out = os.path.join(path, n) + "/"
        command = "mkdir " + out
        print(command)
        call(command, shell=True)

def test(i_path, o_path):
    failed = []
    for n in name:
        inp = os.path.join(i_path, n + "/0.log")
        out = os.path.join(o_path, n) + ".zip"
        command = "./THULR -I " + inp + " -O " + out 
        print(command)
        res = call(command, shell=True)
        if(res != 0):
            failed.append(n)
    return failed
if __name__ == "__main__":
    input_path = sys.argv[1]
    output_path = sys.argv[2]
    makeDir(output_path)
    failed = test(input_path, output_path)
    print("Failed: " + str(failed))
