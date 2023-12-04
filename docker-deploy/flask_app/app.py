from flask import Flask, render_template, jsonify, request
import os
import json

app = Flask(__name__)

# Route to serve the webpage
@app.route('/')
def index():
    return render_template('index.html')

# Route to send log updates to the frontend
@app.route('/log')
def log():
    with open('file/proxy.log', 'r') as file:
        content = file.read()
    return content

@app.route('/blacklist', methods=['GET'])
def get_blacklist():
    try:
        with open('file/blacklist.txt', 'r') as file:
            blacklist_items = file.read().splitlines()
    except FileNotFoundError:
        blacklist_items = []
    return jsonify(blacklist_items)

@app.route('/add_to_blacklist', methods=['POST'])
def add_to_blacklist():
    item = request.form['item']
    with open('file/blacklist.txt', 'a') as file:
        file.write(item + '\n')
    return jsonify(success=True)

@app.route('/remove_from_blacklist', methods=['POST'])
def remove_from_blacklist():
    item = request.form['item']
    with open('file/blacklist.txt', 'r') as file:
        lines = file.readlines()
    with open('file/blacklist.txt', 'w') as file:
        for line in lines:
            if line.strip() != item:
                file.write(line)
    return jsonify(success=True)


if __name__ == '__main__':
    app.run(debug=True)
