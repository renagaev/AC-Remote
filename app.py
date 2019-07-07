from flask import Flask, render_template, jsonify
import requests
from datetime import datetime, timedelta, timezone
from random import randint
import json
import os
from threading import Thread
from time import sleep


def now():
    return datetime.now().replace(tzinfo=timezone(name="Europe/Moscow", offset=timedelta(hours=3)))


uno_url = "http://arduino.churchoffice.keenetic.link"
app = Flask(__name__)
# app.debug = True
temperature = []
humidity = []
state = True


@app.route('/onoff')
def onoff():
    global state

    # включаем кондей
    try:
        requests.get(uno_url + "/onoff")
        state = not state
        return jsonify(success=True)
    except:
        return jsonify(success=False)


def data_loop():
    # добавляем данные
    while True:
        x = 0
        text = requests.get(uno_url + "/data").text
        try:
            h, t = text.split("|")
            t, h = int(t), int(h)

            global temperature, humidity
            now = datetime.now()
            lastday = datetime.now() - timedelta(days=1)
            temperature.append({"date": now, "value": t})
            humidity.append({"date": now, "value": h})
        except:
            pass
        finally:
            temperature = [i for i in temperature if i["date"] > lastday]
            humidity = [i for i in humidity if i["date"] > lastday]
            sleep(10)


@app.route("/stat")
def get_stats():
    temp = [{
        "date": i["date"].isoformat(), "value": i["value"]
    } for i in temperature]
    hum = [{
        "date": i["date"].isoformat(), "value": i["value"]
    } for i in humidity]

    return json.dumps({"temp": temperature[-1]["value"], "hum": humidity[-1]["value"]})


@app.route("/state")
def _state():
    return str(int(state))

@app.route("/changeState")
def sync():
    global state
    state = not state
    return "ok"

@app.route("/")
def main():
    return render_template("main.html")


if __name__ == '__main__':
    Thread(target=data_loop).start()
    app.run()
