from subprocess import call
import time

print("---------Compression time-------------")
time1 = time.time()
command = "tar -czf ./Android.tar.gz ../LogHub_Seg/Android/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Android: " + str(time2 - time1))

time1 = time.time()
command = "tar -czf ./Apache.tar.gz ../LogHub_Seg/Apache/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Apache: " + str(time2 - time1))

time1 = time.time()
command = "tar -czf ./Bgl.tar.gz ../LogHub_Seg/Bgl/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Bgl: " + str(time2 - time1))

time1 = time.time()
command = "tar -czf ./Hadoop.tar.gz ../LogHub_Seg/Hadoop/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Hadoop: " + str(time2 - time1))

time1 = time.time()
command = "tar -czf ./Hdfs.tar.gz ../LogHub_Seg/Hdfs/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Hdfs: " + str(time2 - time1))

time1 = time.time()
command = "tar -czf ./Healthapp.tar.gz ../LogHub_Seg/Healthapp/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Healthapp: " + str(time2 - time1))

time1 = time.time()
command = "tar -czf ./Hpc.tar.gz ../LogHub_Seg/Hpc/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Hpc: " + str(time2 - time1))

time1 = time.time()
command = "tar -czf ./Linux.tar.gz ../LogHub_Seg/Linux/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Linux: " + str(time2 - time1))

time1 = time.time()
command = "tar -czf ./Mac.tar.gz ../LogHub_Seg/Mac/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Mac: " + str(time2 - time1))

time1 = time.time()
command = "tar -czf ./Openstack.tar.gz ../LogHub_Seg/Openstack/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Openstack: " + str(time2 - time1))

time1 = time.time()
command = "tar -czf ./Proxifier.tar.gz ../LogHub_Seg/Proxifier/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Proxifier: " + str(time2 - time1))

time1 = time.time()
command = "tar -czf ./Spark.tar.gz ../LogHub_Seg/Spark/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Spark: " + str(time2 - time1))

time1 = time.time()
command = "tar -czf ./Ssh.tar.gz ../LogHub_Seg/Ssh/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Ssh: " + str(time2 - time1))

time1 = time.time()
command = "tar -czf ./Thunderbird.tar.gz ../LogHub_Seg/Thunderbird/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Thunderbird: " + str(time2 - time1))

time1 = time.time()
command = "tar -czf ./Windows.tar.gz ../LogHub_Seg/Windows/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Windows: " + str(time2 - time1))

time1 = time.time()
command = "tar -czf ./Zookeeper.tar.gz ../LogHub_Seg/Zookeeper/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Zookeeper: " + str(time2 - time1))

print("---------Decompression time-------------")
command = "mkdir ./temp/"
call(command, shell=True)
time1 = time.time()
command = "tar -xzf ./Android.tar.gz -C ./temp/"
print(command)
call(command, shell=True)
time2 = time.time()
print("Android: " + str(time2 - time1))
command = "rm -rf ./temp/"
call(command, shell=True)

command = "mkdir ./temp/"
call(command, shell=True)
time1 = time.time()
command = "tar -xzf ./Apache.tar.gz -C ./temp/"
print(command)
call(command, shell=True)
time2 = time.time()
print("Apache: " + str(time2 - time1))
command = "rm -rf ./temp/"
call(command, shell=True)

command = "mkdir ./temp/"
call(command, shell=True)
time1 = time.time()
command = "tar -xzf ./Bgl.tar.gz -C ./temp/"
print(command)
call(command, shell=True)
time2 = time.time()
print("Bgl: " + str(time2 - time1))
command = "rm -rf ./temp/"
call(command, shell=True)

call(command, shell=True)
time1 = time.time()
command = "tar -xzf ./Hadoop.tar.gz -C ./temp/"
print(command)
call(command, shell=True)
time2 = time.time()
print("Hadoop: " + str(time2 - time1))
command = "rm -rf ./temp/"
call(command, shell=True)

call(command, shell=True)
time1 = time.time()
command = "tar -xzf ./Hdfs.tar.gz -C ./temp/"
print(command)
call(command, shell=True)
time2 = time.time()
print("Hdfs: " + str(time2 - time1))
command = "rm -rf ./temp/"
call(command, shell=True)

call(command, shell=True)
time1 = time.time()
command = "tar -xzf ./Healthapp.tar.gz -C ./temp/"
print(command)
call(command, shell=True)
time2 = time.time()
print("Healthapp: " + str(time2 - time1))
command = "rm -rf ./temp/"
call(command, shell=True)

call(command, shell=True)
time1 = time.time()
command = "tar -xzf ./Hpc.tar.gz -C ./temp/"
print(command)
call(command, shell=True)
time2 = time.time()
print("Hpc: " + str(time2 - time1))
command = "rm -rf ./temp/"
call(command, shell=True)

call(command, shell=True)
time1 = time.time()
command = "tar -xzf ./Linux.tar.gz -C ./temp/"
print(command)
call(command, shell=True)
time2 = time.time()
print("Linux: " + str(time2 - time1))
command = "rm -rf ./temp/"
call(command, shell=True)

call(command, shell=True)
time1 = time.time()
command = "tar -xzf ./Mac.tar.gz -C ./temp/"
print(command)
call(command, shell=True)
time2 = time.time()
print("Mac: " + str(time2 - time1))
command = "rm -rf ./temp/"
call(command, shell=True)

call(command, shell=True)
time1 = time.time()
command = "tar -xzf ./Openstack.tar.gz -C ./temp/"
print(command)
call(command, shell=True)
time2 = time.time()
print("Openstack: " + str(time2 - time1))
command = "rm -rf ./temp/"
call(command, shell=True)

call(command, shell=True)
time1 = time.time()
command = "tar -xzf ./Proxifier.tar.gz -C ./temp/"
print(command)
call(command, shell=True)
time2 = time.time()
print("Proxifier: " + str(time2 - time1))
command = "rm -rf ./temp/"
call(command, shell=True)

call(command, shell=True)
time1 = time.time()
command = "tar -xzf ./Spark.tar.gz -C ./temp/"
print(command)
call(command, shell=True)
time2 = time.time()
print("Spark: " + str(time2 - time1))
command = "rm -rf ./temp/"
call(command, shell=True)

call(command, shell=True)
time1 = time.time()
command = "tar -xzf ./Ssh.tar.gz -C ./temp/"
print(command)
call(command, shell=True)
time2 = time.time()
print("Ssh: " + str(time2 - time1))
command = "rm -rf ./temp/"
call(command, shell=True)

call(command, shell=True)
time1 = time.time()
command = "tar -xzf ./Thunderbird.tar.gz -C ./temp/"
print(command)
call(command, shell=True)
time2 = time.time()
print("Thunderbird: " + str(time2 - time1))
command = "rm -rf ./temp/"
call(command, shell=True)

call(command, shell=True)
time1 = time.time()
command = "tar -xzf ./Windows.tar.gz -C ./temp/"
print(command)
call(command, shell=True)
time2 = time.time()
print("Windows: " + str(time2 - time1))
command = "rm -rf ./temp/"
call(command, shell=True)

call(command, shell=True)
time1 = time.time()
command = "tar -xzf ./Zookeeper.tar.gz -C ./temp/"
print(command)
call(command, shell=True)
time2 = time.time()
print("Zookeeper: " + str(time2 - time1))
command = "rm -rf ./temp/"
call(command, shell=True)

print("---------Query time-------------")
time1 = time.time()
command = "grep ERROR ../LogHub_Seg/Android/* | grep \"socket read length failure -104\" |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Android: " + str(time2 - time1))

time1 = time.time()
command = "grep error ../LogHub_Seg/Apache/* | grep \"Invalid URI in request\" |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Apache: " + str(time2 - time1))

time1 = time.time()
command = "grep ERROR ../LogHub_Seg/Bgl/* | grep R00-M1-ND |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Bgl: " + str(time2 - time1))

time1 = time.time()
command = "grep ERROR ../LogHub_Seg/Hadoop/* | grep \"RECEIVED SIGNAL 15: SIGTERM\" |grep 2015-09-23 |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Hadoop: " + str(time2 - time1))

time1 = time.time()
command = "grep error ../LogHub_Seg/Hdfs/* | grep blk_8846 |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Hdfs: " + str(time2 - time1))

time1 = time.time()
command = "grep Step_ExtSDM ../LogHub_Seg/Healthapp/* | grep totalAltitude=0 |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Healthapp: " + str(time2 - time1))

time1 = time.time()
command = "grep \"unavailable state\" ../LogHub_Seg/Hpc/* | grep HWID=3378 |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Hpc: " + str(time2 - time1))

time1 = time.time()
command = "grep \"authentication failure\" ../LogHub_Seg/Linux/* | grep rhost=221.230.128.214 |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Linux: " + str(time2 - time1))

time1 = time.time()
command = "grep failed ../LogHub_Seg/Mac/* | grep \"Err:-1 Errno:1\" |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Mac: " + str(time2 - time1))

time1 = time.time()
command = "grep -E \"ERROR|WARNING\" ../LogHub_Seg/Openstack/* | grep \"Unexpected error while running command\" |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Openstack: " + str(time2 - time1))

time1 = time.time()
command = "grep HTTPS ../LogHub_Seg/Proxifier/* | grep play.google.com:443 |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Proxifier: " + str(time2 - time1))

time1 = time.time()
command = "grep ERROR ../LogHub_Seg/Spark/* | grep \"Error sending result\" |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Spark: " + str(time2 - time1))

time1 = time.time()
command = "grep \"Received disconnect from\" ../LogHub_Seg/Ssh/* | grep 202.100.179.208 |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Ssh: " + str(time2 - time1))

time1 = time.time()
command = "grep \"Doorbell ACK timeout\" ../LogHub_Seg/Thunderbird/* |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Thunderbird: " + str(time2 - time1))

time1 = time.time()
command = "grep Error ../LogHub_Seg/Windows/* | grep \"Failed to process single phase execution\" |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Windows: " + str(time2 - time1))

time1 = time.time()
command = "grep ERROR ../LogHub_Seg/Zookeeper/* | grep CommitProcessor |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Zookeeper: " + str(time2 - time1))