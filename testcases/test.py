from flask import Flask, jsonify, make_response
import datetime

app = Flask(__name__)

@app.route('/low-age')
def example1():
    # here to change headers
    headers = {
        'Content-Type': 'application/json',
        'Cache-Control': 'max-age=2',
        'ETag': '1234567890abcdef',
        'Content-Length':'10',
    }
    response = make_response(jsonify(headers), 200) # here to change status code
    response.headers = headers # set headers

    return response

@app.route('/private')
def example2():
    # here to change headers
    headers = {
        'Content-Type': 'application/json',
        'Cache-Control': 'max-age=360000, private',
        'ETag': '1234567890abcdef',
        'Content-Length':'10'
    }
    response = make_response(jsonify(headers), 200) # here to change status code
    response.headers = headers # set headers

    return response

@app.route('/normal')
def example3():
    # here to change headers
    headers = {
        'Content-Type': 'application/json',
        'Cache-Control': 'max-age=360000',
        'Content-Length':'10',
        'ETag': '1234567890abcdef',
    }
    response = make_response(jsonify(headers), 200) # here to change status code
    response.headers = headers # set headers

    return response

@app.route('/normal1')
def example111():
    # here to change headers
    headers = {
        'Content-Type': 'application/json',
        'Cache-Control': 'max-age=360000',
        'Content-Length':'10',
        'ETag': '1234567890abcdef',
    }
    response = make_response(jsonify(headers), 200) # here to change status code
    response.headers = headers # set headers

    return response

@app.route('/normal2')
def example222():
    # here to change headers
    headers = {
        'Content-Type': 'application/json',
        'Cache-Control': 'max-age=360000',
        'Content-Length':'10',
        'ETag': '1234567890abcdef',
    }
    response = make_response(jsonify(headers), 200) # here to change status code
    response.headers = headers # set headers

    return response

@app.route('/etag')
def example4():
    # here to change headers
    headers = {
        'Content-Type': 'application/json',
        'ETag': '1234567890abcdef',
        'Content-Length':'10'
    }
    response = make_response(jsonify(headers), 200) # here to change status code
    response.headers = headers # set headers

    return response

@app.route('/no-store')
def example5():
    # here to change headers
    headers = {
        'Content-Type': 'application/json',
        'Cache-Control': 'no-store',
        'ETag': '1234567890abcdef',
        'Content-Length':'10'
    }
    response = make_response(jsonify(headers), 200) # here to change status code
    response.headers = headers # set headers

    return response

@app.route('/expires')
def example6():
    # here to change headers
    headers = {
        'Content-Type': 'application/json',
        'Expires': 'Sat, 26 Feb 2023 23:59:59 GMT',
        'ETag': '1234567890abcdef',
        'Content-Length':'10'
    }
    response = make_response(jsonify(headers), 200) # here to change status code
    response.headers = headers # set headers

    return response

@app.route('/revalidate')
def example7():
    # here to change headers
    headers = {
        'Content-Type': 'application/json',
        'Cache-Control': 'must-revalidate',
        'ETag': '1234567890abcdef',
        'Content-Length':'10'
    }
    response = make_response(jsonify(headers), 200) # here to change status code
    response.headers = headers # set headers
    return response

@app.route('/last-mofied')
def example8():
    # here to change headers
    headers = {
        'Content-Type': 'application/json',
        'Cache-Control': 'public max-age=2',
        'Last-Modified': 'Fri, 26 Feb 2023 10:00:00 GMT',
        'Content-Length':'10'
    }
    response = make_response(jsonify(headers), 200) # here to change status code
    response.headers = headers # set headers

    return response

@app.route('/test-req1')
def example9():
    # here to change headers
    headers = {
        'Content-Type': 'application/json',
        'Cache-Control': 'max-age=5',
        'Content-Length':'10',
        'ETag': '1234567890abcdef',
    }
    response = make_response(jsonify(headers), 200) # here to change status code
    response.headers = headers # set headers

    return response

@app.route('/test-req2')
def example10():
    # here to change headers
    headers = {
        'Content-Type': 'application/json',
        'Cache-Control': 'max-age=5',
        'Content-Length':'10',
        'ETag': '1234567890abcdef',
    }
    response = make_response(jsonify(headers), 200) # here to change status code
    response.headers = headers # set headers

    return response

@app.route('/test-req3')
def example11():
    # here to change headers
    headers = {
        'Content-Type': 'application/json',
        'Cache-Control': 'max-age=5',
        'Content-Length':'10',
        'ETag': '1234567890abcdef',
    }
    response = make_response(jsonify(headers), 200) # here to change status code
    response.headers = headers # set headers

    return response

if __name__ == '__main__':
    app.run(host='0.0.0.0', port='8080')
