# app.py
from flask import Flask, request, jsonify, render_template
import time

app = Flask(__name__)

# almacenamiento simple en memoria
data = {"lat": None, "lon": None, "timestamp": None}

@app.route("/")
def index():
    return render_template("index.html")

# Endpoint para recibir coordenadas desde Arduino (POST)
@app.route("/coordenadas/guardar", methods=["POST"])
def guardar_coordenadas():
    try:
        # Usar request.json para datos en formato JSON
        payload = request.get_json()
        lat = payload.get("lat")
        lon = payload.get("lon")

        # Usar request.form para datos de formulario
        if not lat or not lon:
            lat = request.form.get("lat")
            lon = request.form.get("lon")

        if lat and lon:
            data["lat"] = float(lat)
            data["lon"] = float(lon)
            data["timestamp"] = time.time()
            print(f"Nueva coordenada: {lat}, {lon}")
            return "OK", 200
        else:
            return "Datos incompletos", 400
    except Exception as e:
        print(f"Error al guardar datos: {e}")
        return "Bad Request", 400

# Endpoint para obtener las últimas coordenadas (GET)
@app.route("/coordenadas/obtener", methods=["GET"])
def obtener_coordenadas():
    return jsonify(data)

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)