from flask import Flask, jsonify, request
from flask_cors import CORS
from sqlalchemy import create_engine, text

app = Flask(__name__)
CORS(app)

engine = create_engine('sqlite:///data/greenhouse.db')


@app.route('/api/sensor_data', methods=['GET'])
def get_sensor_data():
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

if __name__ == '__main__':
    app.run(debug=True)