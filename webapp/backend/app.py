import os
from flask import Flask, jsonify, request
from flask_cors import CORS
from sqlalchemy import create_engine, text
from dotenv import load_dotenv

# Load environment variables from .env file
load_dotenv()

app = Flask(__name__)
CORS(app)

engine = create_engine('sqlite:///data/greenhouse.db')
API_KEY = os.getenv('API_KEY')

# Create table if it doesn't exist
with engine.connect() as connection:
    connection.execute(text("""
        CREATE TABLE IF NOT EXISTS SensorReadings (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            sensor_type TEXT NOT NULL,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            value REAL
        )
    """))



@app.route('/api_data', methods=['GET'])
def sensor_data():
    """
    Retrieve sensor data based on the specified sensor type and date-time range.

    Inputs (query parameters):
    - sensor_type (str): The type of sensor data to retrieve (e.g., 'temperature', 'humidity_air', 'humidity_soil', 'light', 'co2_concentration').
    - start_date (str): The start datetime of the range in 'YYYY-MM-DD HH:MM:SS' format.
    - end_date (str): The end datetime of the range in 'YYYY-MM-DD HH:MM:SS' format.

    Returns:
    - JSON array of objects, each containing:
        - timestamp (str): The date and time of the reading in ISO 8601 format.
        - value (float): The sensor reading value.
    """
    sensor_type = request.args.get('sensor_type')
    start_date = request.args.get('start_date')
    end_date = request.args.get('end_date')

    # Validate that all necessary parameters are provided
    if not sensor_type or not start_date or not end_date:
        return jsonify({'error': 'Missing required query parameters'}), 400

    # Query the database
    with engine.connect() as connection:
        query = text("""
            SELECT timestamp, value
            FROM SensorReadings
            WHERE sensor_type = :sensor_type
            AND timestamp BETWEEN :start_date AND :end_date
            ORDER BY timestamp ASC
        """)
        result = connection.execute(query, {
            'sensor_type': sensor_type,
            'start_date': start_date,
            'end_date': end_date
        })

        # Fetch all results and map them correctly
        data = [{'timestamp': row[0], 'value': row[1]} for row in result]

    return jsonify(data)

@app.route('/api/sensor_data', methods=['POST'])
def add_sensor_data():
    """
    Add sensor data to the database. Requires an API key for authorization.

    Input (Form URL-encoded body):
    - light_status (str): The light status (e.g., 'lit', 'dark times').
    - hours_of_light (int): The hours of light.
    - pump_status (str): The status of the water pump.
    - humidity (float): The humidity value.
    - temperature (float): The temperature value.
    - API key in the request headers.
    """
    # Check for API key
    api_key = request.headers.get('API-Key')
    if api_key != API_KEY:
        return jsonify({'error': 'Unauthorized'}), 401

    # Get form data from the request
    light_status = request.form.get('light_status')
    hours_of_light = request.form.get('hours_of_light')
    air_humidity = request.form.get('air_humidity')
    humidity = request.form.get('humidity')
    temperature = request.form.get('temperature')

    # Validate the input data
    if not light_status or not hours_of_light or not air_humidity or not humidity or not temperature:
        return jsonify({'error': 'Missing required fields'}), 400

    try:
        humidity = float(humidity)
        temperature = float(temperature)
        hours_of_light = int(hours_of_light)
    except ValueError:
        return jsonify({'error': 'Invalid data types'}), 400

    # Insert the data into the database with explicit transaction management
    with engine.begin() as connection:  # Automatically commits at the end of the block
        try:
            connection.execute(text("""
                INSERT INTO SensorReadings (sensor_type, value)
                VALUES ('light_status', :light_status)
            """), {'light_status': light_status})

            connection.execute(text("""
                INSERT INTO SensorReadings (sensor_type, value)
                VALUES ('hours_of_light', :hours_of_light)
            """), {'hours_of_light': hours_of_light})

            connection.execute(text("""
                INSERT INTO SensorReadings (sensor_type, value)
                VALUES ('air_humidity', :air_humidity)
            """), {'air_humidity': air_humidity})

            connection.execute(text("""
                INSERT INTO SensorReadings (sensor_type, value)
                VALUES ('soil_humidity', :humidity)
            """), {'humidity': humidity})

            connection.execute(text("""
                INSERT INTO SensorReadings (sensor_type, value)
                VALUES ('temperature', :temperature)
            """), {'temperature': temperature})

        except Exception as e:
            print(f"Error while inserting data: {str(e)}")  # Log the error to the console
            return jsonify({'error': str(e)}), 500

    return jsonify({'message': 'Data added successfully'}), 201


@app.route('/', methods=['Get'])
def home():
    return "index python backend"

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')