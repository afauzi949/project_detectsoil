import sys
import os

sys.path.append(os.path.join(os.path.dirname(__file__), 'utils'))

import streamlit as st
import numpy as np
import pandas as pd
import pickle
import matplotlib.pyplot as plt
import seaborn as sns
from utils.database import get_data
from utils.scraper import get_food_prices

# Load model dan label encoder
with open('models/RandomForest.pkl', 'rb') as f:
    model_data = pickle.load(f)
    if isinstance(model_data, tuple):
        model, label_encoder = model_data
    else:
        model = model_data
        label_encoder = None

# Fungsi untuk memprediksi tanaman yang cocok
def predict_crop(data):
    prediction = model.predict(data)
    if label_encoder:
        crop = label_encoder.inverse_transform(prediction)
        return crop[0]
    return prediction[0]    

# Streamlit UI
st.set_page_config(page_title="Prediksi Tanaman & Harga Pangan", layout="wide")

st.title("Prediksi Tanaman yang Cocok")

# Sidebar untuk input data pengguna
st.sidebar.header("Input Data Sensor")
humidity = st.sidebar.number_input("Humidity", min_value=0.0, max_value=200.0, step=0.1)
temperature = st.sidebar.number_input("Temperature", min_value=0.0, max_value=50.0, step=0.1)
ph = st.sidebar.number_input("pH", min_value=0.0, max_value=14.0, step=0.1)
rainfall = st.sidebar.number_input("Rainfall", min_value=0.0, max_value=500.0, step=0.1)
n = st.sidebar.number_input("Nitrogen (N)", min_value=0.0, max_value=200.0, step=0.1)
p = st.sidebar.number_input("Phosphorus (P)", min_value=0.0, max_value=200.0, step=0.1)
k = st.sidebar.number_input("Potassium (K)", min_value=0.0, max_value=200.0, step=0.1)

if st.sidebar.button("Prediksi"):
    data_input = np.array([[n, p, k, temperature, humidity, ph, rainfall]])  # Masukkan fitur yang diperlukan
    prediction = predict_crop(data_input)
    st.sidebar.write(f"Tanaman yang cocok adalah: {prediction}")

# Ambil data dari MongoDB
data = get_data('DataSensor')

# Pastikan kolom yang diharapkan ada dalam DataFrame
expected_columns = ['n', 'p', 'k', 'temperature', 'humidity', 'ph', 'rainfall']
missing_columns = [col for col in expected_columns if col not in data.columns]

if missing_columns:
    st.error(f"Kolom berikut tidak ditemukan dalam data: {missing_columns}")
else:
    # Ubah indeks DataFrame agar dimulai dari 1
    data.index = data.index + 1

    # Tampilkan data terbaru dari MongoDB
    st.subheader("Data Terbaru dari Sensor")
    st.dataframe(data)

# URL target
url = "https://dpkp.jogjaprov.go.id/harga-pangan/list?page=4"

# Ambil data
headers, data = get_food_prices(url)

# Hapus kolom "#" dari headers dan data
if "#" in headers:
    index = headers.index("#")
    headers.pop(index)
    for row in data:
        row.pop(index)

# Konversi data ke DataFrame
df = pd.DataFrame(data, columns=headers)

# Ubah indeks DataFrame agar dimulai dari 1
df.index = df.index + 1

# Display in Streamlit
st.title("Harga Pangan Eceran")
if not df.empty:
    st.dataframe(df)  # Display the DataFrame in Streamlit
else:
    st.write("Tidak ada data untuk ditampilkan")