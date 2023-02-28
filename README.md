# Proxy Server
## Usage
entry docker-deploy folder. You can either ran 
```
sudo docker-compose up
```
or directly run the main file in ```/src``` directory


## Automatic Testing
### Pre-requisites: Python3, Flask
### step 1
Run the proxy server on your machine
### step 2 
open a new bash terminal and run ```test.py``` in ```/testcases``` directory, this will run a simple web-application writen by flask.
### step 3
- open a new bash terminal and entry ```/testcases``` directory.
- open ```generate.sh``` file, change ```NEW_STRING``` variable to your own host, the one that runs ```test.py```. 
```
NEW_STRING="vcm-xxxxx.vm.duke.edu"
```

- Run
```
bash generate.sh
```
this command will generate test requests using templete text files in ```/test_temp``` to ```/test_reqs``` folder.
- Raplace <HOST_NAME> with your own host (the one runs proxy server) and run
```
bash test.sh <HOST_NAME>
```

## Testing cases
You can read test logic in ```.sh``` files' comments

### ```test_LRU.sh``` 
Test the LRU logic.

### ```test.sh``` 
- Test malformed requests
- Test basic `GET` `POST` `CONNECT`
- Test responses with certain information (ETag, Expires, max-age, private, no-store, must-revalidate...)
- Because I didn't implement the `304 Not Modified` logic in my web-application, so the validation request will always get `200 ok`. I furthre tested my cache logic manually

### `test_req.sh`
- I test requests with certion information (max-stale, max-age, min-fresh, no-store, no-cache...)

### The `.log` files
- I stored my proxy.log outputs of these tests in `.log` files.

# Cache Policy

## cache  size: 100 items
- can change it in cache.hpp: #define defalutMax; 

## replacement  policy 
- last recently used(LRU) whenever store or visit the item, will  prioiritize it, and will drop the least prioritized item when cache size  exceeds the max size; 

## clean expired
- after each revalidation, if the response  is 200 OK, will just replace the response in cache, if is 304 not Modified,  will just change the "Date" field of the response in cache. 

## In request headers

- considered: Cache-Control: max-age, max-stale, min-fresh, no-cache, no-store

- if "Cache-Control:  no-cache": will revalidate every time no matter expires or not

- if "Cache-Control:  no-store": will not store the response of this request

## In response headers

- considered:  Cache-Control: max-age, s-maxage, no-cache, private, must-revalidate, no-store

## revalidate

- if is stale, will need ETag/Last-modified to send request to server, if the response is 200 OK, will update cache. 

