from flask import Flask, request, jsonify
from pymongo import MongoClient
from datetime import datetime

app = Flask(__name__)

# Koneksi ke MongoDB
client = MongoClient("mongodb+srv://alfauzipros:muntilan@cluster0.imrxr0g.mongodb.net/")
db = client['project']
collection = db['DataSensor']

# Endpoint untuk menerima data sensor
@app.route('/sensor-data', methods=['POST'])
def sensor_data():
    data = request.json
    # Validasi data
    required_keys = ['n', 'p', 'k', 'temperature', 'humidity', 'ph', 'rainfall']
    if not all(key in data for key in required_keys):
        return jsonify({"status": "Error", "message": "Invalid data format"}), 400

    data['timestamp'] = datetime.now()
    collection.insert_one(data)
    return jsonify({"status": "Data saved"}), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
