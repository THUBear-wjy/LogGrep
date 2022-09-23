from elasticsearch import Elasticsearch
import time

es = Elasticsearch("http://localhost:9200")

dsl1 = {
    "track_total_hits": "true",
    'query':{
        "match_phrase": {
            "log_content":"unavailable state"
        }
        
    }
}

dsl2 = {
    "track_total_hits":"true",
    'query':{ 
        "bool":{
            "must":[
            {"match_phrase": {
                    "log_content":"unavailable state"
                }},
            {
            "match_phrase": {
                    "log_content":"HWID=3378"
            }}
            ],
        }

    }
}

time1 = time.time()
print(es.search(index = "hpc", body=dsl2, request_timeout=100))
time2 = time.time()
print("time cost: " + str(time2 - time1))