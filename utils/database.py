from pymongo import MongoClient
import pandas as pd

def get_database():
    client = MongoClient("mongodb+srv://alfauzipros:muntilan@cluster0.imrxr0g.mongodb.net/")
    db = client["project"]
    return db

def get_data(collection_name):
    db = get_database()
    collection = db[collection_name]
    data = list(collection.find())
    # Convert to DataFrame and ensure the correct column order
    df = pd.DataFrame(data)
    df = df[['n', 'p', 'k', 'temperature', 'humidity', 'ph', 'rainfall']]
    return df
