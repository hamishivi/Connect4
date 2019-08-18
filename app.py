import subprocess
from flask import Flask, request, render_template

app = Flask(__name__)

@app.route('/')
def main_page():
    return render_template("index.html", width=7, height=6)

@app.route('/ai', methods=["POST"])
def run_ai():
    data = request.form
    board = data['board']
    player = data['player']
    out = subprocess.run(["./ai", board, player], capture_output=True)
    return out.stdout.strip()