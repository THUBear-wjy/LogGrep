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
                    "log_content":"Error sending result"
            }}
            ],
        }

    }
}

time1 = time.time()
print(es.search(index = "spark", body=dsl2, request_timeout=100))
time2 = time.time()
print(time2 - time1)
