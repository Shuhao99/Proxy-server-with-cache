#!/bin/bash

# test normal response
# request with no-store and no-cache in header
cat test_reqs/req_nostore.txt | netcat $1 12345 
sleep 1
cat test_reqs/req_nocache.txt | netcat $1 12345

# test request with max-age=2
sleep 1
cat test_reqs/req_maxage2.txt | netcat $1 12345 
sleep 2
# Should say expire
cat test_reqs/req_maxage2.txt | netcat $1 12345 

sleep 1

# Test request with max-stale=2 in header and 
# and reponse give max=age=5
cat test_reqs/req_maxstale.txt | netcat $1 12345
sleep 5
# Should be in cache valid
cat test_reqs/req_maxstale.txt | netcat $1 12345

sleep 3
# Should be expired
cat test_reqs/req_maxstale.txt | netcat $1 12345

sleep 1
# Test request with min-fresh=2 in its header
# Response return max-age=5 in header
cat test_reqs/req_minfresh.txt | netcat $1 12345
sleep 2 
# Should be in cache valid
cat test_reqs/req_minfresh.txt | netcat $1 12345

sleep 2
# Should be expired
cat test_reqs/req_minfresh.txt | netcat $1 12345