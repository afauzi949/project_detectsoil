# utils/scraper.py

import requests
from bs4 import BeautifulSoup

def get_food_prices(page_url):
    response = requests.get(page_url)
    soup = BeautifulSoup(response.content, 'html.parser')
    
    # Lokasi tabel pada HTML
    table = soup.find('table')
    
    # Ekstraksi data tabel
    data = []
    headers = [header.text for header in table.find_all('th')]
    for row in table.find_all('tr')[1:]:
        columns = row.find_all('td')
        if columns:
            data.append([column.text.strip() for column in columns])
    
    return headers, data
