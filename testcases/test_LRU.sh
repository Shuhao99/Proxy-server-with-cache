# test LRU cache
# Set LRU size as 2
# LRU: last recently used: whenever store or visit some item, 
# will prioritize this item, and when the cache's size 
# exceeds max cache size, 
# will drop the least prioritized item.

# request with no-store and no-cache in header
cat test_reqs/normal.txt | netcat $1 12345 #1
sleep 1
cat test_reqs/normal2.txt | netcat $1 12345 #2
sleep 1
cat test_reqs/normal.txt | netcat $1 12345 #1
sleep 1
cat test_reqs/normal3.txt | netcat $1 12345 #3
sleep 1
# This response should already be dropped
cat test_reqs/normal2.txt | netcat $1 12345 #2