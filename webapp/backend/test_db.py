import numpy as np
from datetime import datetime, timedelta
from sqlalchemy import create_engine, text

# Database connection setup
DATABASE_PATH = 'sqlite:///data/greenhouse.db'
engine = create_engine(DATABASE_PATH, echo=True)

# Sensor data generation
def generate_sensor_data(start_time, num_entries, interval_minutes, sensor_type='air_humidity', base_value=45.24, fluctuation=5.0):
    """Generates sensor data for a given period with random fluctuations."""
    data = []
    for i in range(num_entries):
        timestamp = start_time + timedelta(minutes=i * interval_minutes)
        value = base_value + np.random.normal(0, fluctuation)
        data.append((sensor_type, timestamp, value))
    return data

# Function to insert data into the database
def insert_sensor_data(data):
    """Inserts sensor data into the database."""
    with engine.begin() as connection:
        try:
            for sensor_type, timestamp, value in data:
                connection.execute(text("""
                    INSERT INTO SensorReadings (sensor_type, timestamp, value)
                    VALUES (:sensor_type, :timestamp, :value)
                """), {'sensor_type': sensor_type, 'timestamp': timestamp, 'value': value})
            print(f"Inserted {len(data)} entries successfully")
        except Exception as e:
            print(f"Error while inserting data: {str(e)}")

# Generate data for one week (every 30 minutes)
start_time = datetime(2024, 9, 12, 8, 38, 4)
num_entries = int(7 * 24 * 2)  # 1 week of data, every 30 minutes
sensor_data = generate_sensor_data(start_time, num_entries, 30)

# Insert the generated data into the database
insert_sensor_data(sensor_data)
