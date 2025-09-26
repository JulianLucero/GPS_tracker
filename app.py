# app.py
from flask import Flask, request, jsonify, render_template
import time

app = Flask(__name__)

# almacenamiento simple en memoria
# Añadimos 'time' para almacenar la hora del GPS/Testeo
data = {"lat": None, "lon": None, "time_gps": None, "timestamp_server": None}

@app.route("/")
def index():
    # Asegúrate de tener un archivo 'index.html' en la carpeta 'templates'
    return render_template("index.html")

# Endpoint para recibir coordenadas desde Arduino
# CAMBIO CLAVE: Usamos methods=["GET"] para coincidir con AT+HTTPACTION=0
@app.route("/coordenadas/guardar", methods=["GET"]) 
def guardar_coordenadas():
    try:
        # El Arduino/SIM800L envía datos como parámetros de URL (Query String)
        # Usamos request.args para leerlos
        lat = request.args.get("lat")
        lon = request.args.get("lon")
        time_gps = request.args.get("time") # <-- ¡NUEVA LECTURA DEL PARÁMETRO TIME!

        if lat and lon and time_gps:
            # Almacenar los datos
            data["lat"] = float(lat)
            data["lon"] = float(lon)
            data["time_gps"] = time_gps
            data["timestamp_server"] = time.time() # Hora de recepción del servidor
            
            # Imprimir para depuración
            print(f"Nueva coordenada recibida:")
            print(f" -> Lat: {lat}, Lon: {lon}")
            print(f" -> Hora GPS: {time_gps}")
            
            # CRÍTICO: Devolver 200 OK para que el SIM800L lo reciba
            return "Datos recibidos OK", 200 
        else:
            print(f"Datos incompletos o incorrectos: lat={lat}, lon={lon}, time={time_gps}")
            return "Datos incompletos", 400
            
    except Exception as e:
        print(f"Error al guardar datos: {e}")
        return "Error interno del servidor", 500

# Endpoint para obtener las últimas coordenadas (GET)
@app.route("/coordenadas/obtener", methods=["GET"])
def obtener_coordenadas():
    return jsonify(data)

if __name__ == "__main__":
    # Usa debug=False y el host 0.0.0.0 si vas a desplegar en Render
    app.run(host="0.0.0.0", port=5000)