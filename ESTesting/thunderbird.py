from elasticsearch import Elasticsearch
import time

es = Elasticsearch("http://localhost:9200")


dsl1 = {
    "track_total_hits": "true",
    'query':{
        "match_phrase": {
            "log_content":"Doorbell ACK timeout"
        }
        
    }
}




#print(es.search(index="smalltest"))
time1 = time.time()
print(es.search(index = "thunderbird", body=dsl1, request_timeout=100))
time2 = time.time()
print(time2 - time1)
#print(len(es.search(index = "logs", q = _q)['hits']['hits']))
