from elasticsearch import Elasticsearch
import time

es = Elasticsearch("http://localhost:9200")

dsl1 = {
    "track_total_hits": "true",
    'query':{
        "match_phrase": {
            "log_content":"HTTPS"
        }
        
    }
}

dsl2 = {
    "track_total_hits":"true",
    'query':{ 
        "bool":{
            "must":[
            {"match_phrase": {
                    "log_content":"HTTPS"
                }},
            {
            "match_phrase": {
                    "log_content":"play.google.com:443"
            }}
            ],
        }

    }
}



time1 = time.time()
print(es.search(index = "proxifier", body=dsl2, request_timeout=100))
time2 = time.time()
print("time cost: " + str(time2 - time1))
