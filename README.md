This is the open-source code of paper "LogGrep: Fast and Cheap Cloud Log Storage by Exploiting both Static and Runtime Patterns" (Eurosys'2023)
# Paper Reference
Junyu Wei, Guangyan Zhang, Junchao Chen, Yang Wang, Weimin Zheng, Tingtao Sun, Jiesheng Wu, Jiangwei Jiang. LogGrep: Fast and Cheap Cloud Log Storage by Exploiting both Static and Runtime Patterns. in Proceedings of the 18th European Conference on Computer Systems (EuroSys’23), Roma, Italy, May 2023.

# Folder Description
## ./cmdline_loggrep
Query code to search on compressed logs. The entry is in CmdLineTool. Main logics are in LogStore_API.cpp and SearchAlgorithm.cpp
## ./compression
Compression code to compress original log files into zip files. Core logic are in main.cpp.
## ./example
A minimal working example for quick test. Each folder corresponds to a kind of log, inside which log files are cut into block (64MB for each).
## ./example_zip
Empty by default. Compressed results during quick test can be found here.
## ./output
Empty by default.
## ./readline
Third party code to parse cmd command.
## ./termcapT
Third party code.

# Supported environments
## Hardware
CPU: 2× Intel Xeon E5-2682 2.50GHz CPUs (with 16 cores)

RAM: 188GB

## OS Version
Red Hat 4.8.5 with Linux kernel 3.10.0

## Compiler Version
gcc version 4.8.5 20150623

# Compilation and quick test
## Compilation
``cd ./compression``

``make``

``cd ../``

``cd ./cmdline_loggrep``

``make``
## Quick test
``cd ./compression``

``python3 quickTest.py``

Then you can find compressed files in ./example_zip/

``cd ./cmdline_loggrep``

``./thulr_cmdline [QUERY] [Compressed Folder]``

[QUERY] is the query statement (which can be found in ./query.txt)

[Compressed Folder] is one of the folder under ./example_zip/

For example, to run query on Apache logs, you can use command as follow:

``./thulr_cmdline "error and Invalid URI in request" ./example_zip/Apache``

# Usage instructions
To use LogGrep to compress and query their logs, users needs
## Step 1: preprocess logs
Process original big log file as a folder such as ./DIR (like one of the foler under ./example/), insider which original log file is cut as several log blocks (each is no larger than 64MB).
## Step 2: compress logs
``cd ./compression``

``python3 LogGrep-compression.py -I ./DIR -O ./DIR_zip``
## step 3: query logs
``cd ./cmdline_loggrep``

``./thulr_cmdline [YOUR QUERY STATEMENT] ./DIR_zip``

# Reproduce Results
## Testing dataset
16 types of open access logs can be downloaded at https://zenodo.org/record/1596245#.Yv3zYOxBxqs
## Excution commands
See Usage instructions and the query commands for each type of logs can be found in ./query.txt
## Claimed results
|  LogType   | Original Size(KB) |Compressed Size(KB)  | Compressed Speed(s) | Query speed(s) |
| ----- |----- | ------ | ------ | ------ |
| Android | 187768 | 7556 | 41.93 | 0.80 |
|  Apache| 5016 | 120  | 0.80 | 0.01 | 
|  Bgl | 725772 | 20996 | 156.37 | 0.46 |
|  Hadoop| 16814436 | 331516 | 2183.61 | 4.30 |
| Hdfs| 1541004 | 81852 | 375.33 | 3.00 |
| Healthapp| 22980 | 948 | 5.49 | 0.11 |
| Hpc| 32768 | 1060 | 6.88 | 0.15 | 
| Linux| 2296 | 116 | 0.58 | 0.02 |
| Mac| 16484 | 592 | 2.83 | 0.07 |
| Openstack| 60004 | 3172 | 7.97 | 0.03 |
| Proxifier| 2484 | 128 | 0.50 | 0.02 |
| Spark| 2839836 | 56484 | 600.18 | 1.00 |
| Ssh| 71700 | 1544 | 11.21 | 0.03 |
| Thunderbird| 31043268 | 622200 | 4973.00  | 17.00 |
| Windows| 27356156 | 61428 | 3185.00 | 4.80 |
| Zookeeper| 10184 | 196 | 1.01 | 0.02 |
* we run compression in parallel, here we list the accumlate results

## Compared system (Baseline)
### CLP
Source code can be found at https://github.com/y-scope/clp-core

Since CLP can not process logic operators like "and" and "not", we use CLP to execute the first part connected by logic operators and use grep to execute the following part.

For example, to execute ``ERROR and socket read length failure -104`` on Apache we run

``./clg archives-dir "ERROR" | grep "socket read length failure -104"``

### ElasticSearch
We use Elasticsearch 7.8.0 at https://www.elastic.co/downloads/past-releases/elasticsearch-7-8-0
### Linux gzip and Linux grep 
Linux grep by default settings. gzip of version 1.5.

## Claims when compared with baseline
### Claim 1: 
The query latency of LogGrep is 2.27x to 51.25x (14.56x on avearge) lower than that of gzip+grep. Overall cost is 34% as much as that of gzip+grep on average.

### Claim 2: 
The query latency of LogGrep is comparable compared with ES (On Android, Hdfs, Hadoop, Thunderbird, Winodws, LogGrep has a higher latency by up to 12.23x. On other types of logs, LogGrep has a lower latency by up to 12x.) Overall cost is 5% as much as that of ES on average.

### Claim 3: 
The query latency of LogGrep is 1.94x to 42.00x (13.74x on average) lower than that of CLP. Overall cost is 41% as much as that of CLP.