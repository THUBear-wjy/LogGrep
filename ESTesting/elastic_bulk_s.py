import sys
import json
import time
from elasticsearch import Elasticsearch
from elasticsearch import helpers
import os
from subprocess import call

es = Elasticsearch("http://localhost:9200", verify_certs=False)

files = os.listdir(sys.argv[1])
for f in files:
    fo = open(os.path.join(sys.argv[1], f),'r',encoding="ISO-8859-1")
    line = fo.readline()
    start_time = time.time()
    actions = []
    count = 0
    tot = 0
    while line:
        data = {}
        line = line.strip().replace("\'", "")
        data['log_content'] = line
            
        action = {}
        action['_index'] = sys.argv[2]
        action['_type'] = "test"
        action['_source'] = data
        if(count == 1000):
            helpers.bulk(es, actions)
            actions = []
            count = 0
        actions.append(action)
        count += 1
        tot += 1
        line = fo.readline()
        #print(tot)
    helpers.bulk(es, actions)

    fo.close()
end_time = time.time()
time_cost = end_time - start_time
print("time cost: " + str(time_cost))