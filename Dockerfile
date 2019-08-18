FROM tiangolo/meinheld-gunicorn:python3.6-alpine3.8

COPY static /app/static
COPY templates /app/templates
COPY app.py /app/main.py
COPY ConnectFourTournament.cpp ConnectFourTournament.cpp 
COPY requirements.txt /app/requirements.txt

RUN apk add --no-cache g++
RUN apk add --no-cache gcc
RUN pip install -r /app/requirements.txt

RUN g++ ConnectFourTournament.cpp  -o /app/ai
RUN chmod +x /app/ai