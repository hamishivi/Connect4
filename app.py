import subprocess
from flask import Flask, request, render_template
from subprocess import PIPE

app = Flask(__name__)

@app.route('/')
def main_page():
    return render_template("index.html", width=7, height=6)

@app.route('/ai', methods=["POST"])
def run_ai():
    data = request.form
    board = data['board']
    player = data['player']
    out = subprocess.run(["./ai", board, player], stdout=PIPE, stderr=PIPE)
    return out.stdout.strip()