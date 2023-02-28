#!/bin/bash

# test normal cases max-age=360000
cat test_reqs/normal.txt | netcat $1 12345 
sleep 1
# test cache normal case
# should say in cache, valid
cat test_reqs/normal.txt | netcat $1 12345 

# test low max-age=2
cat test_reqs/low-age.txt | netcat $1 12345 
sleep 2
#test cache low max-age=2
# should say in cache but expired
cat test_reqs/low-age.txt | netcat $1 12345 

sleep 1
#test private
#should say not cacheable because private
cat test_reqs/private.txt | netcat $1 12345 

sleep 1
#test validate using etag
#should say no
cat test_reqs/etag.txt | netcat $1 12345 
sleep 1
# sho
cat test_reqs/etag.txt | netcat $1 12345 
sleep 1

#test no-store
cat test_reqs/no-store.txt | netcat $1 12345 
sleep 1

#test get expire time using expires field in response
cat test_reqs/expires.txt | netcat $1 12345 
sleep 1
cat test_reqs/expires.txt | netcat $1 12345 
sleep 1

# test must revalidate
cat test_reqs/revalidate.txt | netcat $1 12345 
sleep 1
cat test_reqs/revalidate.txt | netcat $1 12345 
sleep 1

# test validate using last-modified time
cat test_reqs/last-mofied.txt | netcat $1 12345 
sleep 2 
cat test_reqs/last-mofied.txt | netcat $1 12345 
sleep 1

#test chunked using url in Notion
cat test_reqs/chunked.txt | netcat $1 12345 
sleep 2

sleep 1
# test post using url in Notion
cat test_reqs/post.txt | netcat $1 12345 
sleep 1
# test connect to google.com
cat test_reqs/connect.txt | netcat $1 12345 