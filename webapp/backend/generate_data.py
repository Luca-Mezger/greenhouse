import sqlite3
import numpy as np
from datetime import datetime, timedelta

# Connect to SQLite database (or create it)
conn = sqlite3.connect('data/greenhouse.db')
cursor = conn.cursor()

# Create table
cursor.execute('''CREATE TABLE IF NOT EXISTS SensorReadings (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp DATETIME NOT NULL,
                    sensor_type VARCHAR(50) NOT NULL,
                    value FLOAT NOT NULL)''')

# Function to generate data for a specific sensor type
def generate_data(sensor_type, start_date, end_date, func):
    current_date = start_date
    while current_date <= end_date:
        value = func(current_date)
        cursor.execute('INSERT INTO SensorReadings (timestamp, sensor_type, value) VALUES (?, ?, ?)',
                       (current_date, sensor_type, value))
        current_date += timedelta(minutes=30)

# Generate 2 years of data
start_date = datetime(2022, 1, 1, 0, 0, 0)
end_date = datetime(2024, 1, 1, 0, 0, 0)

# Temperature: daily sinusoidal variation + yearly trend + random noise
def temperature_func(dt):
    daily_variation = 10 * np.sin(2 * np.pi * (dt.hour * 60 + dt.minute) / 1440)  # Daily cycle
    seasonal_variation = 5 * np.sin(2 * np.pi * dt.timetuple().tm_yday / 365.25)  # Yearly cycle
    long_term_trend = 0.01 * (dt - start_date).days  # Gradual warming over time
    noise = np.random.normal(0, 0.5)  # Random noise
    return 20 + daily_variation + seasonal_variation + long_term_trend + noise

# Humidity Air: inverse daily pattern with temperature + random noise
def humidity_air_func(dt):
    daily_variation = 20 * np.cos(2 * np.pi * (dt.hour * 60 + dt.minute) / 1440)  # Inverse of daily cycle
    seasonal_variation = 10 * np.cos(2 * np.pi * dt.timetuple().tm_yday / 365.25)  # Inverse of yearly cycle
    noise = np.random.normal(0, 2)  # Random noise
    return 60 + daily_variation + seasonal_variation + noise

# Humidity Soil: slow seasonal changes + daily variation + random noise
def humidity_soil_func(dt):
    seasonal_variation = 10 * np.sin(2 * np.pi * dt.timetuple().tm_yday / 365.25)  # Yearly cycle
    daily_variation = 2 * np.sin(2 * np.pi * (dt.hour * 60 + dt.minute) / 1440)  # Daily cycle
    noise = np.random.normal(0, 1)  # Random noise
    return 40 + seasonal_variation + daily_variation + noise

# Light: strong daily pattern with a peak during noon, no seasonal effect
def light_func(dt):
    daily_variation = max(0, 1000 * np.sin(2 * np.pi * (dt.hour * 60 + dt.minute) / 1440))  # Daily cycle
    noise = np.random.normal(0, 50)  # Random noise
    return daily_variation + noise

# CO2 Concentration: daily pattern with a dip during the day (photosynthesis) + noise
def co2_concentration_func(dt):
    daily_variation = -50 * np.sin(2 * np.pi * (dt.hour * 60 + dt.minute) / 1440)  # Daily cycle (inverse)
    seasonal_variation = 20 * np.sin(2 * np.pi * dt.timetuple().tm_yday / 365.25)  # Yearly cycle
    noise = np.random.normal(0, 5)  # Random noise
    return 400 + daily_variation + seasonal_variation + noise

# Generate data for each sensor type
generate_data('temperature', start_date, end_date, temperature_func)
generate_data('humidity_air', start_date, end_date, humidity_air_func)
generate_data('humidity_soil', start_date, end_date, humidity_soil_func)
generate_data('light', start_date, end_date, light_func)
generate_data('co2_concentration', start_date, end_date, co2_concentration_func)

# Commit and close
conn.commit()
conn.close()

print("Data generation complete!")
