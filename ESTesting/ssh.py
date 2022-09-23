from elasticsearch import Elasticsearch
import time

es = Elasticsearch()

dsl1 = {
    "track_total_hits": "true",
    'query':{
        "match_phrase": {
            "log_content":"Received disconnect from"
        }
        
    }
}

dsl2 = {
    "track_total_hits":"true",
    'query':{ 
        "bool":{
            "must":[
            {"match_phrase": {
                    "log_content":"Received disconnect from"
                }},
            {
            "match_phrase": {
                    "log_content":"202.100.179.208"
            }}
            ],
        }

    }
}



#print(es.search(index="smalltest"))
time1 = time.time()
print(es.search(index = "ssh", body=dsl1, request_timeout=100))
print(es.search(index = "ssh", body=dsl2, request_timeout=100))
time2 = time.time()
print(time2 - time1)
#print(len(es.search(index = "logs", q = _q)['hits']['hits']))
