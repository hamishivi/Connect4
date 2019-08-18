import subprocess
from flask import Flask, request, render_template
from subprocess import PIPE
import os
import stat

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
    from os import listdir
    from os.path import isfile, join
    print([f for f in listdir('.') if isfile(join('.', f))])
    data = request.form
    board = data['board']
    player = data['player']
    out = subprocess.run(["./ai", board, player], stdout=PIPE, stderr=PIPE)
    return out.stdout.strip()