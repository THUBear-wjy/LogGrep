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

## Other requirements
Python version >= 3.6.8, use ``pip3 install --upgrade requests`` to install non-listed requirement

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

``./thulr_cmdline [Compressed Folder] [QUERY]``

[QUERY] is the query statement (which can be found in ./query.txt)

[Compressed Folder] is one of the folder under ./example_zip/

For example, to run query on Apache logs, you can use command as follow:

``./thulr_cmdline ../example_zip/Apache "error and Invalid URI in request"``

# Usage instructions
To use LogGrep to compress and query their logs, users needs
## Step 1: preprocess logs
Process original big log file as a folder such as ./DIR (like one of the foler under ./example/), insider which original log file is cut as several log blocks (each is no larger than 64MB).
## Step 2: compress logs
``cd ./compression``

``python3 LogGrep-compression.py -I [Original Folder] -O [Compressed Folder]``
## step 3: query logs
``cd ./cmdline_loggrep``

``./thulr_cmdline [Compressed Folder] [YOUR QUERY STATEMENT]``

# Reproduce Results
## Testing dataset
16 types of open access logs can be downloaded at https://zenodo.org/record/7056802#.Yxm1RexBwq1
## Excution commands
The query commands for each type of logs can be found in ./query.txt (assume all logs are stored under ./example_zip)
## Claimed results
|  LogType   | Original Size(KB) |Compressed Size(KB)  | Total Compression Latency(s) | Accumulated Compression Latency(s)| Query Latency(s) |
| ----- |----- | ------ | ------ | ------ | ------ |
| Android | 186984 | 7556 | 14.62 | 40.84 | 0.80 |
|  Apache| 5020 | 120  | 0.79 | 0.79 | 0.01 | 
|  Bgl | 725796 | 20996 | 34.27 | 119.77 | 0.46 |
|  Hadoop| 16814928 | 331516 | 469.54 | 1843.33 | 4.74 |
| Hdfs| 1530136 | 81852 | 60.59 | 237.84 | 3.02 |
| Healthapp| 22736 | 948 | 5.40 | 5.40 | 0.11 |
| Hpc| 32348 | 1060 | 6.77 | 6.77 | 0.01 | 
| Linux| 2300 | 116 | 0.56 | 0.56 | 0.02 |
| Mac| 16392 | 592 | 2.76 | 2.76 | 0.07 |
| Openstack| 60008 | 3172 | 7.56 | 7.56 | 0.03 |
| Proxifier| 2468 | 128 | 0.49 | 0.49 | 0.02 |
| Spark| 2839924 | 56484 | 136.38 | 524.17 | 1.11 |
| Ssh| 71068 | 1544 | 10.50 | 11.43 | 0.03 |
| Thunderbird| 31044220 | 622200 | 1149.92 | 4494.55  | 17.00 |
| Windows| 27245112 | 61428 | 348.37 | 1376.77 | 4.80 |
| Zookeeper| 10116 | 196 | 0.99 | 0.99 | 0.02 |
* Use Linux ``du -k [DIR]`` to see the Original/Compressed size.
* We run compression in parallel with 4 threads, here we list both total and accumlated results. A time statistic file can be found under ./compression when fininshing compression.
* Query latency includes "LogMetaTime" + "SearchTotalTime".

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
