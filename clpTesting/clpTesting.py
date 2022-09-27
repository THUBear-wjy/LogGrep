from subprocess import call
import time

print("---------Compression time-------------")
time1 = time.time()
command = "./clp c ./Android.zip ../../logs/Android/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Android: " + str(time2 - time1))

time1 = time.time()
command = "./clp c ./Apache.zip ../../logs/Apache/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Apache: " + str(time2 - time1))

time1 = time.time()
command = "./clp c ./Bgl.zip ../../logs/Bgl/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Bgl: " + str(time2 - time1))

time1 = time.time()
command = "./clp c ./Hadoop.zip ../../logs/Hadoop/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Hadoop: " + str(time2 - time1))

time1 = time.time()
command = "./clp c ./Hdfs.zip ../../logs/Hdfs/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Hdfs: " + str(time2 - time1))

time1 = time.time()
command = "./clp c ./Healthapp.zip ../../logs/Healthapp/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Healthapp: " + str(time2 - time1))

time1 = time.time()
command = "./clp c ./Hpc.zip ../../logs/Hpc/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Hpc: " + str(time2 - time1))

time1 = time.time()
command = "./clp c ./Linux.zip ../../logs/Linux/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Linux: " + str(time2 - time1))

time1 = time.time()
command = "./clp c ./Mac.zip ../../logs/Mac/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Mac: " + str(time2 - time1))

time1 = time.time()
command = "./clp c ./Openstack.zip ../../logs/Openstack/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Openstack: " + str(time2 - time1))

time1 = time.time()
command = "./clp c ./Proxifier.zip ../../logs/Proxifier/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Proxifier: " + str(time2 - time1))

time1 = time.time()
command = "./clp c ./Spark.zip ../../logs/Spark/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Spark: " + str(time2 - time1))

time1 = time.time()
command = "./clp c ./Ssh.zip ../../logs/Ssh/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Ssh: " + str(time2 - time1))

time1 = time.time()
command = "./clp c ./Thunderbird.zip ../../logs/Thunderbird/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Thunderbird: " + str(time2 - time1))

time1 = time.time()
command = "./clp c ./Windows.zip ../../logs/Windows/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Windows: " + str(time2 - time1))

time1 = time.time()
command = "./clp c ./Zookeeper.zip ../../logs/Zookeeper/" 
print(command)
call(command, shell=True)
time2 = time.time()
print("Zookeeper: " + str(time2 - time1))

print("---------Query time-------------")
time1 = time.time()
command = "./clg ./Android.zip/ \"ERROR\" | grep \"socket read length failure -104\" |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Android: " + str(time2 - time1))

time1 = time.time()
command = "./clg ./Apache.zip/ \"error\" | grep \"Invalid URI in request\" |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Apache: " + str(time2 - time1))

time1 = time.time()
command = "./clg ./Bgl.zip/ \"ERROR\" | grepR00-M1-ND |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Bgl: " + str(time2 - time1))

time1 = time.time()
command = "./clg ./Hadoop.zip/ \"ERROR\" | grep \"RECEIVED SIGNAL 15: SIGTERM\" |grep 2015-09-23 |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Hadoop: " + str(time2 - time1))

time1 = time.time()
command = "./clg ./Hdfs.zip \"error\"| grep blk_8846 |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Hdfs: " + str(time2 - time1))

time1 = time.time()
command = "./clg ./Healthapp.zip \"Step_ExtSDM\" | grep totalAltitude=0 |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Healthapp: " + str(time2 - time1))

time1 = time.time()
command = "./clg ./Hpc.zip \"unavailable state\" | grep HWID=3378 |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Hpc: " + str(time2 - time1))

time1 = time.time()
command = "./clg ./Linux.zip \"authentication failure\" | grep rhost=221.230.128.214 |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Linux: " + str(time2 - time1))

time1 = time.time()
command = "./clg ./Mac.zip \"failed\" | grep \"Err:-1 Errno:1\" |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Mac: " + str(time2 - time1))

time1 = time.time()
command = "./clg ./Proxifier.zip \"HTTPS\" | grep play.google.com:443 |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Proxifier: " + str(time2 - time1))

time1 = time.time()
command = "./clg ./Spark.zip \"ERROR\" | grep \"Error sending result\" |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Spark: " + str(time2 - time1))

time1 = time.time()
command = "./clg ./Ssh.zip \"Received disconnect from\" | grep 202.100.179.208 |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Ssh: " + str(time2 - time1))

time1 = time.time()
command = "./clg ./Thunderbird.zip \"Doorbell ACK timeout\" |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Thunderbird: " + str(time2 - time1))

time1 = time.time()
command = "./clg ./Windows.zip \"Error\"| grep \"Failed to process single phase execution\" |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Windows: " + str(time2 - time1))

time1 = time.time()
command = "./clg ./Zookeeper.zip \"ERROR\" | grep CommitProcessor |wc -l"
print(command)
call(command, shell=True)
time2 = time.time()
print("Zookeeper: " + str(time2 - time1))