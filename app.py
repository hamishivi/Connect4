import subprocess
from flask import Flask, request

app = Flask(__name__)

@app.route('/')
def main_page():
    return 'I will make a connect 4 UI eventually'

@app.route('/ai', methods=["POST"])
def run_ai():
    data = request.form
    board = data['board']
    player = data['player']
    out = subprocess.run(["./ai", board, player], capture_output=True)
    return out.stdout.strip()