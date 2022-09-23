This is the open-source code of paper "LogGrep: Fast and Cheap Cloud Log Storage by Exploiting both Static and Runtime Patterns" (Eurosys'2023)
# Paper Reference
Junyu Wei, Guangyan Zhang, Junchao Chen, Yang Wang, Weimin Zheng, Tingtao Sun, Jiesheng Wu, Jiangwei Jiang. LogGrep: Fast and Cheap Cloud Log Storage by Exploiting both Static and Runtime Patterns. in Proceedings of the 18th European Conference on Computer Systems (EuroSys’23), Roma, Italy, May 2023.

# Folder Description
## cmdline_loggrep
Query code to search on compressed logs. The entry is in CmdLineTool. Main logics are in LogStore_API.cpp and SearchAlgorithm.cpp
## compression
Compression code to compress original log files into zip files. Core logic are in main.cpp.
## example
A minimal working example for quick test. Each folder corresponds to a kind of log, inside which log files are cut into block (64MB for each).
## example_zip
Empty by default. Compressed results during quick test can be found here.
## LogHub_Seg_zip
Empty by default. Compressed results during large test can be found here.
## output
Empty by default.
## ESTesting
Testing script for Elasticsearch to compare with LogGrep.

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

use ``yum groupinstall 'Development Tools`` to install other requirements.

# Compilation and quick test
## Compilation
``mkdir ./output``

``mkdir ./example_zip``

``cd ./compression``

``make``

``cd ../``

``cd ./cmdline_loggrep``

``make``
## Quick test
``cd ./compression``

``python3 quickTest.py``

Then you can find compressed files in ./example_zip/.

``cd ./cmdline_loggrep``

``./thulr_cmdline [Compressed Folder] [QUERY]``

[QUERY] is the query statement

[Compressed Folder] is one of the folder under ./example_zip/

All testing query for quick test can be found at ./query4quicktest.txt. For example, to run query on Apache logs, you can use command as follow:

``./thulr_cmdline ../example_zip/Apache "error and Invalid URI in request"``
## Large test
Download large dataset from https://zenodo.org/record/7056802#.Yxm1RexBwq1 at [DATASET PATH] (such as /usr/LogHub_Seg/)

``cd ./compression``

``python3 largeTest.py [DATASET PATH]``

Then you can find compressed files in ./LogHub_Seg_zip/. 

You can also find a compression summary under ./compression such as ./compression/Log_2022-09-21, which records whether there is an error and the compression speed.

``cd ./cmdline_loggrep``

``./thulr_cmdline [Compressed Folder] [QUERY]``

[QUERY] is the query statement

[Compressed Folder] is one of the folder under ./LogHub_Seg_zip/

All testing query for large test can be found at ./query4largetest.txt. For example, to run query on Hadoop logs, you can use command as follow:

``./thulr_cmdline ../LogHub_Seg_zip/Hadoop "ERROR and RECEIVED SIGNAL 15: SIGTERM and 2015-09-23"``

# Usage instructions
To use LogGrep to compress and query their logs, users need
## Step 1: preprocess logs
Process original big log file as a folder such as ./DIR (like one of the foler under ./example/), insider which original log file is cut as several log blocks (each is no larger than 64MB).
## Step 2: compress logs
``cd ./compression``

``python3 LogGrep-compression.py -I [Original Folder] -O [Compressed Folder]``
## step 3: query logs
``cd ./cmdline_loggrep``

``./thulr_cmdline [Compressed Folder] [QUERY]``

# Reproduce Results
## Testing dataset
16 types of open access logs can be downloaded at https://zenodo.org/record/7056802#.Yxm1RexBwq1
## Excution commands
See large test for reproduce above
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
* Query latency includes "LoadMetaTime" + "SearchTotalTime".

## Compared system (Baseline)
### Linux gzip and Linux grep 
Linux grep by default settings. gzip of version 1.5. Since grep and gzip are Linux native tool, it does not install. You can use gzip to compress logs, use unzip to decompress logs and use grep to query on logs. 

We use pipline stream to execute "and" logic with grep and grep -E to execute "or". 

For example, to execute ``ERROR and socket read length failure -104`` on Apache we run
``grep "ERROR" [all decompressed logs] | grep "socket read length failure -104"``

For example, to execute ``"ERROR or WARNING and Unexpected error while running command"`` on Openstack we run
``grep -E "ERROR|WARNING" [all decompressed logs] | grep "Unexpected error while running command"``

### ElasticSearch
#### Download
We download elasticsearch-7.14.0-linux-x86_64.tar at https://www.elastic.co/downloads/past-releases/elasticsearch-7-14-0.
#### Install and running
Decompress the package and run elasticsearch in background

``tar -xf elasticsearch-7.14.0-linux-x86_64.tar``

``cd ./elasticsearch-7.14.0/bin/``

``./elasticsearch -d``

``pip3 install elasticsearch``
#### Log Inserting
Using elastic_bulk_s.py under ./ESTesting to insert logs

``cd ./ESTesing``

``python3 elastic_bulk_s.py [LogFile] [indexName]``

For example, to test Apache logs, we can concate all log files under ./example/Apache as a large log file Apache.log and run

``python3 elastic_bulk_s.py ./Apache.log Apache``

The compression time can be found when running this inserting process and the compression size can be found with

``curl -X GET localhost:9200/_cat/indices?v``

The size of index Apache will be shown.
#### Log Query
We write testing scripts for all logs for large test under ./ESTesting. After inserting logs into Apache index, we can use the following command to query on logs.

``python3 apache.py``

### CLP
Source code can be found at https://github.com/y-scope/clp/tree/main/components/core
#### Download and Install
A detailed downloading and installing process can be found at README.md file under https://github.com/y-scope/clp/tree/main/components/core
#### Log Compression
We compress logs using clp:

``./clp c [compressed-dir] [log files]``

We concate all log files under the same directory in LogHub_Seg (such as concate all logs under LogHub_Seg/Hadoop as Hadoop.log) and compress this log using clp, record compression time.

The compressed size can be found by:

``du -k [compressed-dir]``
#### Log Query
We query logs using clg:

``./clg [compressed-dir] [Query]``

Since CLP can not process logic operators like "and" and "not", we use CLP to execute the first part connected by logic operators and use grep to execute the following part.

For example, to execute ``ERROR and socket read length failure -104`` on Apache we run
``./clg archives-dir "ERROR" | grep "socket read length failure -104"``

CLP can not run queries on Openstack (since it includes "or" logic), this has be reported in the paper. 

## Claims when compared with baseline
### Claim 1: 
The query latency of LogGrep is 2.27x to 51.25x (14.56x on avearge) lower than that of gzip+grep. Overall cost is 34% as much as that of gzip+grep on average.

### Claim 2: 
The query latency of LogGrep is comparable compared with ES (On Android, Hdfs, Hadoop, Thunderbird, Winodws, LogGrep has a higher latency by up to 12.23x. On other types of logs, LogGrep has a lower latency by up to 12x.) Overall cost is 5% as much as that of ES on average.

### Claim 3: 
The query latency of LogGrep is 1.94x to 42.00x (13.74x on average) lower than that of CLP. Overall cost is 41% as much as that of CLP.
