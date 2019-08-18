import subprocess
from flask import Flask, request, render_template
from subprocess import PIPE
import os
import stat
import re

app = Flask(__name__)

@app.before_first_request
def startup():
    st = os.stat('./ai')
    os.chmod('./ai', st.st_mode | stat.S_IEXEC)

@app.route('/')
def main_page():
    return render_template("index.html", width=7, height=6)

@app.route('/ai', methods=["POST"])
def run_ai():
    data = request.form
    board = data['board']
    player = data['player']
    # since we are running a subprocess, we need to be careful about user input
    # so we validate strictly below.
    if not (player == "yellow" or player == "red"):
        return 'invalid POST', 422 # code for 'unprocessable entity', ie bad input
    # ensure board only has '.', ',', 'r' or 'y' in it and is correct length
    pattern = re.compile("[^\,\.ry]")
    if re.search(pattern, board) is not None or len(board) != 47:
        return 'invalid POST', 422
    out = subprocess.run(["./ai", board, player], stdout=PIPE, stderr=PIPE)
    return out.stdout.strip(), 200