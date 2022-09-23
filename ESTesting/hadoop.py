from elasticsearch import Elasticsearch
import time

es = Elasticsearch("http://localhost:9200")

dsl1 = {
    "track_total_hits": "true",
    'query':{
        "match_phrase": {
            "log_content":"ERROR"
        }
        
    }
}

dsl2 = {
    "track_total_hits":"true",
    'query':{ 
        "bool":{
            "must":[
            {"match_phrase": {
                    "log_content":"ERROR"
                }},
            {
            "match_phrase": {
                    "log_content":"RECEIVED SIGNAL 15: SIGTERM"
            }}
            ],
        }

    }
}

dsl3 = {
    "track_total_hits":"true",
    'query':{ 
        "bool":{
            "must":[
            {"match_phrase": {
                    "log_content":"ERROR"
                }},
            {
            "match_phrase": {
                    "log_content":"RECEIVED SIGNAL 15: SIGTERM"
            }},
            {
            "match_phrase": {
                    "log_content":"2015-09-23"
            }}
            ],
        }

    }
}

time1 = time.time()
print(es.search(index = "hadoop", body=dsl3, request_timeout=100))
time2 = time.time()
print("time cost: " + str(time2 - time1))